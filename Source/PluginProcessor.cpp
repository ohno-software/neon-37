#include "PluginProcessor.h"
#include "PluginEditor.h"

// --- Neon37Voice Implementation ---

Neon37Voice::Neon37Voice()
{
    // Moog-style oscillator waveforms with anti-aliasing considerations
    auto waveFunc = [this](float x, int& waveType) {
        float phase = (x + juce::MathConstants<float>::pi) / juce::MathConstants<float>::twoPi;
        phase = phase - std::floor(phase); // Normalize to 0-1
        
        switch (waveType) {
            case 0: // Sine - pure sine wave
                return std::sin (x);
                
            case 1: { // Triangle - Moog-style with slight rounding
                float tri = 4.0f * std::abs(phase - 0.5f) - 1.0f;
                return tri;
            }
            
            case 2: { // Sawtooth - Moog-style with DC offset correction
                float saw = 2.0f * phase - 1.0f;
                return saw;
            }
            
            case 3: // Square - 50% duty cycle
                return phase < 0.5f ? 1.0f : -1.0f;
                
            case 4: // Pulse - Variable duty cycle
                return phase < currentPw ? 1.0f : -1.0f;
                
            default:
                return std::sin (x);
        }
    };

    osc1.initialise ([this, waveFunc](float x) { return waveFunc(x, currentOsc1Wave); });
    osc2.initialise ([this, waveFunc](float x) { return waveFunc(x, currentOsc2Wave); });
    // Sub oscillator is always square wave, one octave below Osc1
    sub1.initialise ([](float x) { 
        float phase = (x + juce::MathConstants<float>::pi) / juce::MathConstants<float>::twoPi;
        phase = phase - std::floor(phase);
        return phase < 0.5f ? 1.0f : -1.0f;
    });
    // White noise generator
    noise.initialise ([](float x) { 
        juce::ignoreUnused(x); 
        return juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
    });

    auto lfoWaveFunc = [](float x, int waveType) {
        float phase = (x + juce::MathConstants<float>::pi) / juce::MathConstants<float>::twoPi;
        phase = phase - std::floor(phase);
        switch (waveType) {
            case 0: return std::sin(x); // Sine
            case 1: return 4.0f * std::abs(phase - 0.5f) - 1.0f; // Triangle
            case 2: return 2.0f * phase - 1.0f; // Saw
            case 3: return phase < 0.5f ? 1.0f : -1.0f; // Square
            case 4: return phase < 0.25f ? 1.0f : -1.0f; // Pulse (25%)
            default: return std::sin(x);
        }
    };

    lfo1.initialise([this, lfoWaveFunc](float x) { return lfoWaveFunc(x, currentLfo1Wave); });
    lfo2.initialise([this, lfoWaveFunc](float x) { return lfoWaveFunc(x, currentLfo2Wave); });
}

bool Neon37Voice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<Neon37Sound*> (sound) != nullptr;
}

void Neon37Voice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition)
{
    juce::ignoreUnused (velocity, sound, currentPitchWheelPosition);
    currentVelocity = velocity;
    
    // Set frequencies immediately with correct tuning to prevent glide
    auto getOctaveMult = [](float choice) -> float {
        int octave = (int)choice;
        switch (octave) {
            case 0: return 0.5f; // 16'
            case 1: return 1.0f; // 8'
            case 2: return 2.0f; // 4'
            case 3: return 4.0f; // 2'
            default: return 1.0f;
        }
    };
    
    float baseFreq = (float)juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    baseOsc1Freq = baseFreq * getOctaveMult(currentOsc1Octave) * std::pow(2.0f, (currentOsc1Semitones + currentOsc1Fine) / 12.0f);
    baseOsc2Freq = baseFreq * getOctaveMult(currentOsc2Octave) * std::pow(2.0f, (currentOsc2Semitones + currentOsc2Fine) / 12.0f);
    baseSub1Freq = baseOsc1Freq * 0.5f;
    
    osc1.setFrequency (baseOsc1Freq, true);
    osc2.setFrequency (baseOsc2Freq, true);
    sub1.setFrequency (baseSub1Freq, true);
    
    osc1.reset();
    osc2.reset();
    sub1.reset();
    
    // Reset filter to prevent clicks from previous state
    // Calculate initial filter cutoff to avoid "wah" at start
    // If attack is 0, we want to start at the "open" position.
    // If attack > 0, we start at the "closed" position.
    float initialEnv1 = (env1Params.attack <= 0.001f) ? 1.0f : 0.0f;
    if (useExpEnv) initialEnv1 *= initialEnv1;
    
    float initialCutoff = currentFilterCutoff * std::pow(2.0f, (initialEnv1 * currentFilterEgDepth));
    initialCutoff = juce::jlimit(20.0f, 20000.0f, initialCutoff);
    voiceFilter.setCutoffFrequencyHz(initialCutoff);
    voiceFilter.reset();
    
    fadeOutCounter = -1; // Reset fade-out

    // Start envelopes
    if (currentLegatoMode && activeVoiceCount > 0)
    {
        // Legato: only start if not already active
        if (!env2.isActive())
        {
            env1.noteOn();
            env2.noteOn();
            fadeInCounter = 0;
        }
        // If already active, we don't reset fadeInCounter to 0 to avoid a dip
    }
    else
    {
        // Normal or first note: retrigger
        env1.noteOn();
        env2.noteOn();
        fadeInCounter = 0;
    }
}

void Neon37Voice::stopNote (float velocity, bool allowTailOff)
{
    juce::ignoreUnused (velocity);
    env1.noteOff();
    env2.noteOff();
    
    // Start fade-out to prevent click at note-off
    fadeOutCounter = 0;
    
    if (!allowTailOff || !env2.isActive())
        clearCurrentNote();
}

void Neon37Voice::pitchWheelMoved (int newPitchWheelValue) 
{ 
    currentPitchBend = (newPitchWheelValue - 8192) / 8192.0f;
}

void Neon37Voice::controllerMoved (int controllerNumber, int newControllerValue) 
{ 
    if (controllerNumber == 1)
        currentModWheel = newControllerValue / 127.0f;
}

void Neon37Voice::prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = outputChannels;

    osc1.prepare (spec);
    osc2.prepare (spec);
    sub1.prepare (spec);
    noise.prepare (spec);
    
    osc1Gain.prepare (spec);
    osc2Gain.prepare (spec);
    sub1Gain.prepare (spec);
    noiseGain.prepare (spec);
    
    voiceFilter.prepare (spec);
    voiceFilter.setMode (juce::dsp::LadderFilterMode::LPF24);
    voiceFilter.setEnabled (true);
    voiceFilter.reset();

    env1.setSampleRate (sampleRate);
    env2.setSampleRate (sampleRate);
    
    // Negate all default smoothing/ramping for maximum snappiness
    osc1Gain.setRampDurationSeconds (0.0);
    osc2Gain.setRampDurationSeconds (0.0);
    sub1Gain.setRampDurationSeconds (0.0);
    noiseGain.setRampDurationSeconds (0.0);

    currentSampleRate = sampleRate;
    fadeInSamples = 0;   // Truly instant attack
    fadeOutSamples = (int)(0.002 * sampleRate); // Keep a tiny bit of fade-out to prevent pops on release

    synthBuffer.setSize (outputChannels, samplesPerBlock);
    isPrepared = true;
}

void Neon37Voice::retriggerEnvelopes()
{
    env1.noteOn();
    env2.noteOn();
    fadeInCounter = 0;
}

void Neon37Voice::startEnvelopesWithoutRetrigger()
{
    // If already active, don't do anything. 
    // If not active (e.g. new voice in para mode), we should ideally 
    // jump to the current sustain level, but ADSR doesn't support that easily.
    // For now, we'll just start it normally but the user's request 
    // implies that subsequent notes in Para+Legato don't retrigger.
    if (!env2.isActive())
    {
        env1.noteOn();
        env2.noteOn();
        fadeInCounter = 0;
    }
}

void Neon37Voice::updateParameters (float osc1Wave, float osc1Octave, float osc1Semitones, float osc1Fine,
                                   float osc2Wave, float osc2Octave, float osc2Semitones, float osc2Fine,
                                   float env1A, float env1D, float env1S, float env1R,
                                   float env2A, float env2D, float env2S, float env2R,
                                   float mixerOsc1, float mixerOsc2, float mixerSub1, float mixerNoise,
                                   float filterCutoff, float filterResonance, float filterDrive, float filterEgDepth,
                                   bool expEnv,
                                   float lfo1Rate, bool lfo1Sync, int lfo1Wave, float lfo1Pitch, float lfo1Filter, float lfo1Amp,
                                   float lfo2Rate, bool lfo2Sync, int lfo2Wave, float lfo2Pitch, float lfo2Filter, float lfo2Amp,
                                   float velPitch, float velFilter, float velAmp,
                                   float atPitch, float atFilter, float atAmp,
                                   float pbPitch, float pbFilter, float pbAmp,
                                   bool lfo1Mw, bool lfo2Mw, bool velMw, bool atMw, bool pbMw,
                                   bool legatoMode, bool paraMode, int numActiveVoices)
{
    juce::ignoreUnused(lfo1Sync, lfo2Sync);
    currentOsc1Wave = (int)osc1Wave;
    currentOsc2Wave = (int)osc2Wave;
    currentLfo1Wave = lfo1Wave;
    currentLfo2Wave = lfo2Wave;
    
    // Store oscillator tuning for use in startNote
    currentOsc1Octave = osc1Octave;
    currentOsc1Semitones = osc1Semitones;
    currentOsc1Fine = osc1Fine;
    currentOsc2Octave = osc2Octave;
    currentOsc2Semitones = osc2Semitones;
    currentOsc2Fine = osc2Fine;
    
    // Store filter parameters for use during rendering
    currentFilterCutoff = filterCutoff;
    currentFilterResonance = filterResonance;
    currentFilterDrive = filterDrive;
    currentFilterEgDepth = filterEgDepth;
    useExpEnv = expEnv;

    // Store mod parameters
    modLfo1Pitch = lfo1Pitch; modLfo1Filter = lfo1Filter; modLfo1Amp = lfo1Amp;
    modLfo2Pitch = lfo2Pitch; modLfo2Filter = lfo2Filter; modLfo2Amp = lfo2Amp;
    modVelPitch = velPitch; modVelFilter = velFilter; modVelAmp = velAmp;
    modAtPitch = atPitch; modAtFilter = atFilter; modAtAmp = atAmp;
    modPbPitch = pbPitch; modPbFilter = pbFilter; modPbAmp = pbAmp;
    
    modLfo1Mw = lfo1Mw;
    modLfo2Mw = lfo2Mw;
    modVelMw = velMw;
    modAtMw = atMw;
    modPbMw = pbMw;

    currentLegatoMode = legatoMode;
    currentParaMode = paraMode;
    activeVoiceCount = numActiveVoices;

    lfo1.setFrequency(lfo1Rate);
    lfo2.setFrequency(lfo2Rate);
    
    auto getOctaveMult = [](int choice) -> float {
        switch (choice) {
            case 0: return 0.5f; // 16'
            case 1: return 1.0f; // 8'
            case 2: return 2.0f; // 4'
            case 3: return 4.0f; // 2'
            default: return 1.0f;
        }
    };
    
    if (isVoiceActive())
    {
        float baseFreq = (float)juce::MidiMessage::getMidiNoteInHertz (getCurrentlyPlayingNote());
        // Apply octave, semitones, and fine tuning (fine is in semitones/cents)
        baseOsc1Freq = baseFreq * getOctaveMult((int)osc1Octave) * std::pow(2.0f, (osc1Semitones + osc1Fine) / 12.0f);
        baseOsc2Freq = baseFreq * getOctaveMult((int)osc2Octave) * std::pow(2.0f, (osc2Semitones + osc2Fine) / 12.0f);
        baseSub1Freq = baseOsc1Freq * 0.5f;
        
        osc1.setFrequency (baseOsc1Freq);
        osc2.setFrequency (baseOsc2Freq);
        sub1.setFrequency (baseSub1Freq);
    }

    env1Params.attack = env1A;
    env1Params.decay = env1D;
    env1Params.sustain = env1S;
    env1Params.release = env1R;
    env1.setParameters (env1Params);

    env2Params.attack = env2A;
    env2Params.decay = env2D;
    env2Params.sustain = env2S;
    env2Params.release = env2R;
    env2.setParameters (env2Params);

    osc1Gain.setGainDecibels (mixerOsc1);
    osc2Gain.setGainDecibels (mixerOsc2);
    sub1Gain.setGainDecibels (mixerSub1);
    noiseGain.setGainDecibels (mixerNoise);
}

void Neon37Voice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (!isPrepared || !env2.isActive())
        return;

    synthBuffer.clear();
    
    const int subBlockSize = 16;
    int samplesRemaining = numSamples;
    int currentStartSample = 0;

    while (samplesRemaining > 0)
    {
        int numThisTime = std::min (subBlockSize, samplesRemaining);
        
        float avgEnv1 = 0.0f;
        float avgModFilter = 0.0f;
        float avgModPitch = 0.0f;
        float avgModAmp = 0.0f;

        // 1. Render oscillators and calculate modulations for this sub-block
        for (int s = 0; s < numThisTime; ++s) {
            int bufferIdx = currentStartSample + s;
            float env1Val = env1.getNextSample();
            if (useExpEnv) env1Val = env1Val * env1Val;
            lastEnv1Value = env1Val;
            avgEnv1 += env1Val;

            float lfo1Val = lfo1.processSample(0.0f);
            float lfo2Val = lfo2.processSample(0.0f);

            // Mod Wheel scaling
            float mwLfo1 = modLfo1Mw ? currentModWheel : 1.0f;
            float mwLfo2 = modLfo2Mw ? currentModWheel : 1.0f;
            float mwVel = modVelMw ? currentModWheel : 1.0f;
            float mwAt = modAtMw ? currentModWheel : 1.0f;
            float mwPb = modPbMw ? currentModWheel : 1.0f;

            // Pitch bend quantized to semitones
            float pbPitchOffset = std::round(currentPitchBend * modPbPitch);

            float modPitch = (lfo1Val * modLfo1Pitch * mwLfo1) + (lfo2Val * modLfo2Pitch * mwLfo2) + 
                             (currentVelocity * modVelPitch * mwVel) + (currentAftertouch * modAtPitch * mwAt) +
                             (pbPitchOffset * mwPb);
            
            float modFilter = (lfo1Val * modLfo1Filter * mwLfo1) + (lfo2Val * modLfo2Filter * mwLfo2) + 
                              (currentVelocity * modVelFilter * mwVel) + (currentAftertouch * modAtFilter * mwAt) +
                              (currentPitchBend * modPbFilter * mwPb);

            float modAmp = (lfo1Val * modLfo1Amp * mwLfo1) + (lfo2Val * modLfo2Amp * mwLfo2) + 
                           (currentVelocity * modVelAmp * mwVel) + (currentAftertouch * modAtAmp * mwAt) +
                           (currentPitchBend * modPbAmp * mwPb);

            avgModFilter += modFilter;
            avgModPitch += modPitch;
            avgModAmp += modAmp;

            float pitchMult = std::pow(2.0f, modPitch / 12.0f);
            osc1.setFrequency(baseOsc1Freq * pitchMult);
            osc2.setFrequency(baseOsc2Freq * pitchMult);
            sub1.setFrequency(baseSub1Freq * pitchMult);

            float val = 0.0f;
            val += osc1Gain.processSample(osc1.processSample(0.0f));
            val += osc2Gain.processSample(osc2.processSample(0.0f));
            val += sub1Gain.processSample(sub1.processSample(0.0f));
            val += noiseGain.processSample(noise.processSample(0.0f));

            for (int c = 0; c < synthBuffer.getNumChannels(); ++c)
                synthBuffer.setSample(c, bufferIdx, val);
        }

        // 2. Apply filter for this sub-block
        avgEnv1 /= (float)numThisTime;
        avgModFilter /= (float)numThisTime;
        
        float modulatedCutoff = currentFilterCutoff * std::pow(2.0f, (avgEnv1 * currentFilterEgDepth) + (avgModFilter * 5.0f));
        modulatedCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);
        
        voiceFilter.setCutoffFrequencyHz(modulatedCutoff);
        voiceFilter.setResonance(currentFilterResonance);
        voiceFilter.setDrive(currentFilterDrive);
        
        auto subBlock = juce::dsp::AudioBlock<float> (synthBuffer).getSubBlock((size_t)currentStartSample, (size_t)numThisTime);
        
        // Pre-filter boost to push the ladder filter's internal saturation harder
        // This makes the 'Drive' feel much more aggressive
        if (currentFilterDrive > 1.0f)
            subBlock.multiplyBy(1.0f + (currentFilterDrive - 1.0f) * 0.15f);

        juce::dsp::ProcessContextReplacing<float> context (subBlock);
        voiceFilter.process(context);
        
        // 3. Apply amplitude envelope for this sub-block
        avgModAmp /= (float)numThisTime;
        float modAmpGain = juce::jlimit(0.0f, 1.0f, 1.0f + avgModAmp);

        for (int s = 0; s < numThisTime; ++s) {
            int bufferIdx = currentStartSample + s;
            float env2Val = env2.getNextSample();
            if (useExpEnv) env2Val = env2Val * env2Val;
            float fadeGain = 1.0f;
            
            if (fadeInCounter < fadeInSamples) {
                fadeGain *= (float)fadeInCounter / (float)fadeInSamples;
                fadeInCounter++;
            }
            
            if (fadeOutCounter >= 0) {
                float fadeOutGain = 1.0f - ((float)fadeOutCounter / (float)fadeOutSamples);
                fadeGain *= juce::jmax(0.0f, fadeOutGain);
                fadeOutCounter++;
            }
            
            for (int c = 0; c < synthBuffer.getNumChannels(); ++c) {
                float sample = synthBuffer.getSample(c, bufferIdx) * env2Val * modAmpGain * fadeGain;
                synthBuffer.setSample(c, bufferIdx, sample);
            }
        }

        currentStartSample += numThisTime;
        samplesRemaining -= numThisTime;
    }

    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
    {
        outputBuffer.addFrom (channel, startSample, synthBuffer, channel, 0, numSamples);
    }

    if ((fadeOutCounter >= fadeOutSamples) || !env2.isActive())
        clearCurrentNote();
}

// --- Neon37AudioProcessor Implementation ---

Neon37AudioProcessor::Neon37AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
     :
#endif
    apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new Neon37Voice());
    
    synth.addSound (new Neon37Sound());
}

Neon37AudioProcessor::~Neon37AudioProcessor()
{
}

const juce::String Neon37AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Neon37AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Neon37AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Neon37AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Neon37AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Neon37AudioProcessor::getNumPrograms()
{
    return 1;
}

int Neon37AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Neon37AudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String Neon37AudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void Neon37AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void Neon37AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<Neon37Voice*> (synth.getVoice (i)))
            voice->prepareToPlay (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    }

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
    masterGain.prepare (spec);
    masterGain.setRampDurationSeconds (0.0);
}

void Neon37AudioProcessor::releaseResources()
{
}

bool Neon37AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void Neon37AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    bool monoMode = *apvts.getRawParameterValue("mono_mode") > 0.5f;
    bool holdMode = *apvts.getRawParameterValue("hold_mode") > 0.5f;
    bool legatoMode = *apvts.getRawParameterValue("legato_mode") > 0.5f;
    bool paraMode = *apvts.getRawParameterValue("para_mode") > 0.5f;

    // Handle Hold Mode transition (OFF -> ON is fine, ON -> OFF needs to release notes)
    if (lastHoldState && !holdMode)
    {
        // Release all notes that are not physically held
        for (int i = 0; i < 128; ++i)
        {
            bool isPhysicallyHeld = std::find(physicallyHeldNotes.begin(), physicallyHeldNotes.end(), i) != physicallyHeldNotes.end();
            if (!isPhysicallyHeld)
            {
                synth.noteOff(1, i, 0.0f, true);
                heldNotes.erase(std::remove(heldNotes.begin(), heldNotes.end(), i), heldNotes.end());
            }
        }
    }
    lastHoldState = holdMode;

    juce::MidiBuffer processedMidi;
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            if (std::find(physicallyHeldNotes.begin(), physicallyHeldNotes.end(), msg.getNoteNumber()) == physicallyHeldNotes.end())
                physicallyHeldNotes.push_back(msg.getNoteNumber());
            
            processedMidi.addEvent(msg, metadata.samplePosition);
            
            // Paraphonic retrigger logic for Poly mode
            if (!monoMode && paraMode && !legatoMode)
            {
                for (int i = 0; i < synth.getNumVoices(); ++i)
                    if (auto v = dynamic_cast<Neon37Voice*>(synth.getVoice(i)))
                        if (v->isVoiceActive())
                            v->retriggerEnvelopes();
            }
        }
        else if (msg.isNoteOff())
        {
            physicallyHeldNotes.erase(std::remove(physicallyHeldNotes.begin(), physicallyHeldNotes.end(), msg.getNoteNumber()), physicallyHeldNotes.end());
            
            if (!holdMode)
                processedMidi.addEvent(msg, metadata.samplePosition);
        }
        else
        {
            processedMidi.addEvent(msg, metadata.samplePosition);
        }
    }

    if (monoMode)
    {
        juce::MidiBuffer monoBuffer;
        for (const auto metadata : processedMidi)
        {
            auto msg = metadata.getMessage();
            if (msg.isNoteOn())
            {
                if (std::find(heldNotes.begin(), heldNotes.end(), msg.getNoteNumber()) == heldNotes.end())
                    heldNotes.push_back(msg.getNoteNumber());
                std::sort(heldNotes.begin(), heldNotes.end());
                
                int targetNote = heldNotes[0];
                if (targetNote != currentPlayingNote)
                {
                    if (currentPlayingNote != -1)
                        monoBuffer.addEvent(juce::MidiMessage::noteOff(msg.getChannel(), currentPlayingNote), metadata.samplePosition);
                    
                    monoBuffer.addEvent(juce::MidiMessage::noteOn(msg.getChannel(), targetNote, msg.getFloatVelocity()), metadata.samplePosition);
                    currentPlayingNote = targetNote;

                    // Paraphonic retrigger logic for Mono mode
                    if (paraMode && !legatoMode)
                    {
                        for (int i = 0; i < synth.getNumVoices(); ++i)
                            if (auto v = dynamic_cast<Neon37Voice*>(synth.getVoice(i)))
                                if (v->isVoiceActive())
                                    v->retriggerEnvelopes();
                    }
                }
            }
            else if (msg.isNoteOff())
            {
                heldNotes.erase(std::remove(heldNotes.begin(), heldNotes.end(), msg.getNoteNumber()), heldNotes.end());
                
                if (heldNotes.empty())
                {
                    if (currentPlayingNote != -1)
                        monoBuffer.addEvent(juce::MidiMessage::noteOff(msg.getChannel(), currentPlayingNote), metadata.samplePosition);
                    currentPlayingNote = -1;
                }
                else
                {
                    int targetNote = heldNotes[0];
                    if (targetNote != currentPlayingNote)
                    {
                        if (currentPlayingNote != -1)
                            monoBuffer.addEvent(juce::MidiMessage::noteOff(msg.getChannel(), currentPlayingNote), metadata.samplePosition);
                        
                        monoBuffer.addEvent(juce::MidiMessage::noteOn(msg.getChannel(), targetNote, 1.0f), metadata.samplePosition);
                        currentPlayingNote = targetNote;

                        // Paraphonic retrigger logic for Mono mode
                        if (paraMode && !legatoMode)
                        {
                            for (int i = 0; i < synth.getNumVoices(); ++i)
                                if (auto v = dynamic_cast<Neon37Voice*>(synth.getVoice(i)))
                                    if (v->isVoiceActive())
                                        v->retriggerEnvelopes();
                        }
                    }
                }
            }
            else
            {
                monoBuffer.addEvent(msg, metadata.samplePosition);
            }
        }
        midiMessages.swapWith(monoBuffer);
    }
    else
    {
        heldNotes.clear();
        currentPlayingNote = -1;
        midiMessages.swapWith(processedMidi);
    }

    // Update parameters for all voices
    float cutoff = *apvts.getRawParameterValue ("cutoff");
    float resonance = *apvts.getRawParameterValue ("resonance");
    float drive = *apvts.getRawParameterValue ("drive");
    float egDepth = *apvts.getRawParameterValue ("eg_depth");
    
    // LFO 1
    float lfo1Rate = *apvts.getRawParameterValue ("lfo1_rate");
    bool lfo1Sync = *apvts.getRawParameterValue ("lfo1_sync") > 0.5f;
    int lfo1Wave = (int)*apvts.getRawParameterValue ("lfo1_wave");
    float lfo1Pitch = *apvts.getRawParameterValue ("lfo1_pitch");
    float lfo1Filter = *apvts.getRawParameterValue ("lfo1_filter");
    float lfo1Amp = *apvts.getRawParameterValue ("lfo1_amp");

    // LFO 2
    float lfo2Rate = *apvts.getRawParameterValue ("lfo2_rate");
    bool lfo2Sync = *apvts.getRawParameterValue ("lfo2_sync") > 0.5f;
    int lfo2Wave = (int)*apvts.getRawParameterValue ("lfo2_wave");
    float lfo2Pitch = *apvts.getRawParameterValue ("lfo2_pitch");
    float lfo2Filter = *apvts.getRawParameterValue ("lfo2_filter");
    float lfo2Amp = *apvts.getRawParameterValue ("lfo2_amp");

    // Velocity/AT/CC
    float velPitch = *apvts.getRawParameterValue ("vel_pitch");
    float velFilter = *apvts.getRawParameterValue ("vel_filter");
    float velAmp = *apvts.getRawParameterValue ("vel_amp");
    float atPitch = *apvts.getRawParameterValue ("at_pitch");
    float atFilter = *apvts.getRawParameterValue ("at_filter");
    float atAmp = *apvts.getRawParameterValue ("at_amp");

    float pbPitch = *apvts.getRawParameterValue ("pb_pitch");
    float pbFilter = *apvts.getRawParameterValue ("pb_filter");
    float pbAmp = *apvts.getRawParameterValue ("pb_amp");
    
    bool lfo1Mw = *apvts.getRawParameterValue ("lfo1_mw") > 0.5f;
    bool lfo2Mw = *apvts.getRawParameterValue ("lfo2_mw") > 0.5f;
    bool velMw = *apvts.getRawParameterValue ("vel_mw") > 0.5f;
    bool atMw = *apvts.getRawParameterValue ("at_mw") > 0.5f;
    bool pbMw = *apvts.getRawParameterValue ("pb_mw") > 0.5f;
    
    // Handle MIDI CC and Aftertouch
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isAftertouch())
        {
            for (int i = 0; i < synth.getNumVoices(); ++i)
                if (auto v = dynamic_cast<Neon37Voice*>(synth.getVoice(i)))
                    v->setAftertouch(msg.getAfterTouchValue() / 127.0f);
        }
        else if (msg.isChannelPressure())
        {
            for (int i = 0; i < synth.getNumVoices(); ++i)
                if (auto v = dynamic_cast<Neon37Voice*>(synth.getVoice(i)))
                    v->setAftertouch(msg.getChannelPressureValue() / 127.0f);
        }
        else if (msg.isController() && msg.getControllerNumber() == 1)
        {
            for (int i = 0; i < synth.getNumVoices(); ++i)
                if (auto v = dynamic_cast<Neon37Voice*>(synth.getVoice(i)))
                    v->setModWheel(msg.getControllerValue() / 127.0f);
        }
    }

    int activeVoices = 0;
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (synth.getVoice(i)->isVoiceActive())
            activeVoices++;

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<Neon37Voice*> (synth.getVoice (i)))
        {
            voice->updateParameters (
                *apvts.getRawParameterValue ("osc1_wave"),
                *apvts.getRawParameterValue ("osc1_octave"),
                *apvts.getRawParameterValue ("osc1_semitones"),
                *apvts.getRawParameterValue ("osc1_fine"),
                *apvts.getRawParameterValue ("osc2_wave"),
                *apvts.getRawParameterValue ("osc2_octave"),
                *apvts.getRawParameterValue ("osc2_semitones"),
                *apvts.getRawParameterValue ("osc2_fine"),
                *apvts.getRawParameterValue ("env1_attack"),
                *apvts.getRawParameterValue ("env1_decay"),
                *apvts.getRawParameterValue ("env1_sustain"),
                *apvts.getRawParameterValue ("env1_release"),
                *apvts.getRawParameterValue ("env2_attack"),
                *apvts.getRawParameterValue ("env2_decay"),
                *apvts.getRawParameterValue ("env2_sustain"),
                *apvts.getRawParameterValue ("env2_release"),
                *apvts.getRawParameterValue ("mixer_osc1"),
                *apvts.getRawParameterValue ("mixer_osc2"),
                *apvts.getRawParameterValue ("mixer_sub1"),
                *apvts.getRawParameterValue ("mixer_noise"),
                cutoff, resonance, drive, egDepth,
                *apvts.getRawParameterValue ("env_exp_curv") > 0.5f,
                lfo1Rate, lfo1Sync, lfo1Wave, lfo1Pitch, lfo1Filter, lfo1Amp,
                lfo2Rate, lfo2Sync, lfo2Wave, lfo2Pitch, lfo2Filter, lfo2Amp,
                velPitch, velFilter, velAmp,
                atPitch, atFilter, atAmp,
                pbPitch, pbFilter, pbAmp,
                lfo1Mw, lfo2Mw, velMw, atMw, pbMw,
                *apvts.getRawParameterValue("legato_mode") > 0.5f,
                *apvts.getRawParameterValue("para_mode") > 0.5f,
                activeVoices
            );
        }
    }

    // Render Synth with per-voice filtering
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // Apply Master Gain
    float masterVolume = *apvts.getRawParameterValue ("master_volume");
    masterGain.setGainDecibels (masterVolume);
    
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    
    masterGain.process (context);
}

bool Neon37AudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* Neon37AudioProcessor::createEditor()
{
    return new Neon37AudioProcessorEditor (*this);
}

void Neon37AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
}

void Neon37AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

juce::AudioProcessorValueTreeState::ParameterLayout Neon37AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Master Section
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("master_volume", "Volume", juce::NormalisableRange<float> (-60.0f, 10.0f, 0.1f, 2.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("master_freq", "Master Freq", 410.0f, 470.0f, 440.0f)); // Centered at 440Hz
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("master_tune", "Master Tune", -100.0f, 100.0f, 0.0f));

    // Oscillator 1
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc1_wave", "Osc 1 Wave", juce::StringArray { "Sine", "Triangle", "Saw", "Square", "Pulse" }, 2));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc1_octave", "Osc 1 Octave", juce::StringArray { "16'", "8'", "4'", "2'" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc1_semitones", "Osc 1 Semitones", -12.0f, 12.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc1_fine", "Osc 1 Fine", -0.5f, 0.5f, 0.0f)); // ±50 cents
    
    // Oscillator 2
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc2_wave", "Osc 2 Wave", juce::StringArray { "Sine", "Triangle", "Saw", "Square", "Pulse" }, 2));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc2_octave", "Osc 2 Octave", juce::StringArray { "16'", "8'", "4'", "2'" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2_semitones", "Osc 2 Semitones", -12.0f, 12.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2_fine", "Osc 2 Fine", -0.5f, 0.5f, 0.0f)); // ±50 cents

    // Global Osc
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc_freq", "Osc Frequency", -7.0f, 7.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc_beat", "Beat Rate", -3.5f, 3.5f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("hard_sync", "Hard Sync", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("para_mode", "Paraphonic", false));

    // Mixer
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("mixer_osc1", "Mixer Osc 1", juce::NormalisableRange<float> (-60.0f, 10.0f, 0.1f, 2.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("mixer_sub1", "Mixer Sub 1", juce::NormalisableRange<float> (-60.0f, 10.0f, 0.1f, 2.0f), -60.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("mixer_osc2", "Mixer Osc 2", juce::NormalisableRange<float> (-60.0f, 10.0f, 0.1f, 2.0f), -60.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("mixer_noise", "Mixer Noise", juce::NormalisableRange<float> (-60.0f, 10.0f, 0.1f, 2.0f), -60.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("mixer_return", "Mixer Return", juce::NormalisableRange<float> (-60.0f, 10.0f, 0.1f, 2.0f), -60.0f));

    // Filter
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("cutoff", "Cutoff", juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.3f), 20000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("resonance", "Resonance", 0.0f, 1.2f, 0.0f)); // Up to 1.2 for self-oscillation
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("drive", "Drive", 1.0f, 25.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("eg_depth", "EG Depth", -5.0f, 5.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("key_track", "Key Track", 0.0f, 2.0f, 0.0f));

    // Envelope 1 (Filter/Mod)
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env1_attack", "Env 1 Attack", juce::NormalisableRange<float> (0.0f, 10.0f, 0.001f, 0.3f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env1_decay", "Env 1 Decay", juce::NormalisableRange<float> (0.001f, 10.0f, 0.001f, 0.3f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env1_sustain", "Env 1 Sustain", 0.0f, 1.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env1_release", "Env 1 Release", juce::NormalisableRange<float> (0.001f, 10.0f, 0.001f, 0.3f), 0.1f));

    // Envelope 2 (Amplitude)
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env2_attack", "Env 2 Attack", juce::NormalisableRange<float> (0.0f, 10.0f, 0.001f, 0.3f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env2_decay", "Env 2 Decay", juce::NormalisableRange<float> (0.001f, 10.0f, 0.001f, 0.3f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env2_sustain", "Env 2 Sustain", 0.0f, 1.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env2_release", "Env 2 Release", juce::NormalisableRange<float> (0.001f, 10.0f, 0.001f, 0.3f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("env_exp_curv", "Exponential Envelopes", true));

    // Arpeggiator
    params.push_back (std::make_unique<juce::AudioParameterBool> ("arp_on", "Arp On", false)); // Default OFF
    params.push_back (std::make_unique<juce::AudioParameterBool> ("mono_mode", "Mono Mode", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("hold_mode", "Hold Mode", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("legato_mode", "Legato Mode", false));
    
    // Glide/Gliss - defaults to OFF (time = 0)
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("gliss_time", "Gliss Time", juce::NormalisableRange<float> (0.0f, 10.0f, 0.01f, 0.3f), 0.0f)); // Default 0 = OFF
    params.push_back (std::make_unique<juce::AudioParameterBool> ("gliss_rte", "Gliss RTE", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("gliss_tme", "Gliss TME", true)); // Default mode
    params.push_back (std::make_unique<juce::AudioParameterBool> ("gliss_log", "Gliss LOG", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("gliss_on_gat_leg", "Gliss On Gat Leg", false));

    // LFO 1
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo1_rate", "LFO 1 Rate", juce::NormalisableRange<float> (0.01f, 50.0f, 0.01f, 0.3f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("lfo1_sync", "LFO 1 Sync", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("lfo1_wave", "LFO 1 Wave", juce::StringArray { "Sine", "Triangle", "Saw", "Square", "Pulse" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo1_pitch", "LFO 1 Pitch", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo1_filter", "LFO 1 Filter", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo1_amp", "LFO 1 Amp", 0.0f, 1.0f, 0.0f));

    // LFO 2
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo2_rate", "LFO 2 Rate", juce::NormalisableRange<float> (0.01f, 50.0f, 0.01f, 0.3f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("lfo2_sync", "LFO 2 Sync", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("lfo2_wave", "LFO 2 Wave", juce::StringArray { "Sine", "Triangle", "Saw", "Square", "Pulse" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo2_pitch", "LFO 2 Pitch", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo2_filter", "LFO 2 Filter", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo2_amp", "LFO 2 Amp", 0.0f, 1.0f, 0.0f));

    // Velocity
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("vel_pitch", "Vel Pitch", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("vel_filter", "Vel Filter", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("vel_amp", "Vel Amp", 0.0f, 1.0f, 1.0f));

    // Aftertouch
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("at_pitch", "AT Pitch", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("at_filter", "AT Filter", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("at_amp", "AT Amp", 0.0f, 1.0f, 0.0f));

    // Pitch Bend
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("pb_pitch", "PB Pitch", juce::NormalisableRange<float>(0.0f, 12.0f, 1.0f), 2.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("pb_filter", "PB Filter", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("pb_amp", "PB Amp", 0.0f, 1.0f, 0.0f));

    // Mod Wheel Toggles
    params.push_back (std::make_unique<juce::AudioParameterBool> ("lfo1_mw", "LFO 1 MW", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("lfo2_mw", "LFO 2 MW", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("vel_mw", "Vel MW", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("at_mw", "AT MW", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("pb_mw", "PB MW", false));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Neon37AudioProcessor();
}
