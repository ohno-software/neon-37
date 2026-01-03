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
            case 0: { // Triangle
                float tri = 4.0f * std::abs(phase - 0.5f) - 1.0f;
                return tri;
            }
            
            case 1: { // Sawtooth
                float saw = 2.0f * phase - 1.0f;
                return saw;
            }
            
            case 2: // Square - 50% duty cycle
                return phase < 0.5f ? 1.0f : -1.0f;
                
            case 3: // 25% Pulse
                return phase < 0.25f ? 1.0f : -1.0f;
                
            case 4: // 10% Pulse
                return phase < 0.10f ? 1.0f : -1.0f;
                
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

    lfo1.initialise([this](float x) {
        float phase = (x + juce::MathConstants<float>::pi) / juce::MathConstants<float>::twoPi;
        phase = phase - std::floor(phase);
        
        float out = 0.0f;
        switch (currentLfo1Wave) {
            case 0: out = std::sin(x); break; // Sine
            case 1: out = 2.0f * phase - 1.0f; break; // Saw Up
            case 2: out = 1.0f - 2.0f * phase; break; // Saw Down
            case 3: out = phase < 0.5f ? 1.0f : -1.0f; break; // Square
            case 4: // S&H
                if (phase < lfo1LastPhase) {
                    lfo1LastSH = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                }
                out = lfo1LastSH;
                break;
            default: out = std::sin(x); break;
        }
        lfo1LastPhase = phase;
        return out;
    });

    lfo2.initialise([this](float x) {
        float phase = (x + juce::MathConstants<float>::pi) / juce::MathConstants<float>::twoPi;
        phase = phase - std::floor(phase);
        
        float out = 0.0f;
        switch (currentLfo2Wave) {
            case 0: out = std::sin(x); break; // Sine
            case 1: out = 2.0f * phase - 1.0f; break; // Saw Up
            case 2: out = 1.0f - 2.0f * phase; break; // Saw Down
            case 3: out = phase < 0.5f ? 1.0f : -1.0f; break; // Square
            case 4: // S&H
                if (phase < lfo2LastPhase) {
                    lfo2LastSH = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                }
                out = lfo2LastSH;
                break;
            default: out = std::sin(x); break;
        }
        lfo2LastPhase = phase;
        return out;
    });
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
    pitchOverrideMidiNote = midiNoteNumber;
    baseOsc1Freq = baseFreq * getOctaveMult(currentOsc1Octave) * std::pow(2.0f, (currentOsc1Semitones + currentOsc1Fine) / 12.0f);
    baseOsc2Freq = baseFreq * getOctaveMult(currentOsc2Octave) * std::pow(2.0f, (currentOsc2Semitones + currentOsc2Fine) / 12.0f);
    baseSub1Freq = baseOsc1Freq * 0.5f;
    
    // Force immediate frequency changes, no smoothing
    osc1.setFrequency (baseOsc1Freq, true);
    osc2.setFrequency (baseOsc2Freq, true);
    sub1.setFrequency (baseSub1Freq, true);
    
    // Reset oscillators only for NEW voices (not currently playing)
    // For Para-L: each new note in the chord gets a fresh voice, so reset it
    // For Mono-L: legato notes reuse the same voice without calling startNote,
    // so this only resets on the very first note
    if (!isVoiceActive())
    {
        osc1.reset();
        osc2.reset();
        sub1.reset();
    }
    
    // Start envelopes
    bool isLegato = (currentVoiceMode == 0 || currentVoiceMode == 2);
    bool isParaOrMono = (currentVoiceMode >= 0 && currentVoiceMode <= 3);
    
    // Reset filter to prevent clicks from previous state
    // Only reset filter for Poly mode or first note in Para/Mono modes
    // For legato modes, check paraState activeNotes rather than voice count
    // because mono mode kills/restarts voices which resets voice count to 0
    bool shouldResetFilter = false;
    if (!isParaOrMono)
    {
        shouldResetFilter = true; // Poly mode always resets
    }
    else if (isLegato && paraState)
    {
        // For legato modes, only reset if no notes are currently held
        shouldResetFilter = (paraState->activeNotes == 0);
    }
    else if (paraState)
    {
        // For non-legato Para/Mono modes, reset on first note
        shouldResetFilter = (paraState->activeNotes == 0);
    }
    
    if (shouldResetFilter)
    {
        // Calculate initial filter cutoff to avoid "wah" at start
        // If attack is 0, we want to start at the "open" position.
        // If attack > 0, we start at the "closed" position.
        float initialEnv1 = (env1Params.attack <= 0.001f) ? 1.0f : 0.0f;
        if (useExpEnv) initialEnv1 *= initialEnv1;
        
        float initialCutoff = currentFilterCutoff * std::pow(2.0f, (initialEnv1 * currentFilterEgDepth));
        initialCutoff = juce::jlimit(20.0f, 20000.0f, initialCutoff);
        voiceFilter.setCutoffFrequencyHz(initialCutoff);
        voiceFilter.reset();
    }
    
    fadeOutCounter = -1; // Reset fade-out
    
    if (isParaOrMono)
    {
        // Envelopes are handled by the processor for Para/Mono modes
        // Ensure per-voice envelopes are completely off
        env1.reset();
        env2.reset();
        // Para-L needs a small fade to prevent pops when notes are added to the chord
        bool isParaL = (currentVoiceMode == 2);
        fadeInCounter = isParaL ? 0 : -1;
    }
    else if (isLegato && activeVoiceCount > 0)
    {
        // Legato: only start if not already active
        if (!env2.isActive())
        {
            env1.reset();
            env2.reset();
            env1.noteOn();
            env2.noteOn();
            fadeInCounter = 0;
        }
        // If already active, we don't reset fadeInCounter to 0 to avoid a dip
    }
    else
    {
        // Normal or first note: retrigger
        env1.reset();
        env2.reset();
        env1.noteOn();
        env2.noteOn();
        fadeInCounter = 0;
    }
}

void Neon37Voice::retuneToMidiNote (int midiNoteNumber)
{
    // Retune without restarting envelopes/filters/osc phase.
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

    pitchOverrideMidiNote = midiNoteNumber;
    float baseFreq = (float)juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    baseOsc1Freq = baseFreq * getOctaveMult(currentOsc1Octave) * std::pow(2.0f, (currentOsc1Semitones + currentOsc1Fine) / 12.0f);
    baseOsc2Freq = baseFreq * getOctaveMult(currentOsc2Octave) * std::pow(2.0f, (currentOsc2Semitones + currentOsc2Fine) / 12.0f);
    baseSub1Freq = baseOsc1Freq * 0.5f;

    osc1.setFrequency (baseOsc1Freq, true);
    osc2.setFrequency (baseOsc2Freq, true);
    sub1.setFrequency (baseSub1Freq, true);
}

void Neon37Voice::stopNote (float velocity, bool allowTailOff)
{
    juce::ignoreUnused (velocity);

    pitchOverrideMidiNote = -1;
    
    bool isParaOrMono = (currentVoiceMode >= 0 && currentVoiceMode <= 3);
    if (!isParaOrMono)
    {
        env1.noteOff();
        env2.noteOff();
    }
    
    if (!allowTailOff)
    {
        clearCurrentNote();
    }
    else
    {
        // Poly mode uses fade-out for voice management
        // Para modes also need fade-out to prevent pops when individual notes are released
        bool isParaL = (currentVoiceMode == 2);
        bool isPara = (currentVoiceMode == 3);
        
        if (!isParaOrMono || isParaL || isPara)
        {
            // For Para modes, only fade out if other notes are still held.
            // If it's the last note, let it follow the shared envelope release.
            if ((isParaL || isPara) && paraState && paraState->activeNotes == 0)
            {
                // Last note, don't fade out here, let shared envelope handle it
            }
            else
            {
                fadeOutCounter = 0;
            }
        }
    }
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
    lfo1.prepare (spec);
    lfo2.prepare (spec);
    
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
    
    lfo1LastPhase = 0.0f;
    lfo1LastSH = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
    lfo2LastPhase = 0.0f;
    lfo2LastSH = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;

    // Negate all default smoothing/ramping for maximum snappiness
    osc1Gain.setRampDurationSeconds (0.0);
    osc2Gain.setRampDurationSeconds (0.0);
    sub1Gain.setRampDurationSeconds (0.0);
    noiseGain.setRampDurationSeconds (0.0);

    currentSampleRate = sampleRate;
    fadeInSamples = (int)(0.004 * sampleRate);   // 4ms fade-in for Para-L
    fadeOutSamples = (int)(0.010 * sampleRate);  // 10ms fade-out for Para-L and Poly

    synthBuffer.setSize (outputChannels, samplesPerBlock);
    isPrepared = true;
}

void Neon37Voice::retriggerEnvelopes()
{
    env1.reset();
    env2.reset();
    env1.noteOn();
    env2.noteOn();
    voiceFilter.reset();
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
                                   int voiceMode, int numActiveVoices)
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
    modLfo1Pitch = 12.0f * (std::pow(2.0f, lfo1Pitch) - 1.0f); 
    modLfo1Filter = lfo1Filter; 
    modLfo1Amp = lfo1Amp;
    
    modLfo2Pitch = 12.0f * (std::pow(2.0f, lfo2Pitch) - 1.0f); 
    modLfo2Filter = lfo2Filter; 
    modLfo2Amp = lfo2Amp;
    
    // Velocity and Aftertouch need proper scaling for filter modulation
    modVelPitch = velPitch; 
    modVelFilter = velFilter * 4.0f;  // Scale to match LFO range
    modVelAmp = velAmp;
    
    modAtPitch = atPitch; 
    modAtFilter = atFilter * 4.0f;    // Scale to match LFO range
    modAtAmp = atAmp;
    
    modPbPitch = pbPitch; 
    modPbFilter = pbFilter * 5.0f;    // Scale to match LFO range
    modPbAmp = pbAmp;
    
    modLfo1Mw = lfo1Mw;
    modLfo2Mw = lfo2Mw;
    modVelMw = velMw;
    modAtMw = atMw;
    modPbMw = pbMw;

    currentVoiceMode = voiceMode;
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
        const int noteForPitch = (pitchOverrideMidiNote >= 0) ? pitchOverrideMidiNote : getCurrentlyPlayingNote();
        float baseFreq = (float)juce::MidiMessage::getMidiNoteInHertz (noteForPitch);
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
    bool isParaOrMono = (currentVoiceMode >= 0 && currentVoiceMode <= 3);
    
    // Check if voice should render
    if (!isPrepared)
        return;
    
    if (isParaOrMono)
    {
        // For Para/Mono modes, check if voice is playing a note
        if (!isVoiceActive())
            return;
    }
    else
    {
        // For Poly mode, check per-voice envelope
        if (!env2.isActive())
            return;
    }

    synthBuffer.clear();
    
    const int subBlockSize = 16;
    int samplesRemaining = numSamples;
    int currentStartSample = 0;

    while (samplesRemaining > 0)
    {
        int numThisTime = std::min (subBlockSize, samplesRemaining);
        
        // Get modulation values at the start of this sub-block for the filter
        float currentEnv1 = 0.0f;
        if (isParaOrMono && paraState)
            currentEnv1 = paraState->env1Buffer.getSample(0, currentStartSample);
        else
        {
            currentEnv1 = env1.getNextSample();
            if (useExpEnv) currentEnv1 = currentEnv1 * currentEnv1;
        }
        lastEnv1Value = currentEnv1;

        float lfo1ValStart = lfo1.processSample(0.0f);
        float lfo2ValStart = lfo2.processSample(0.0f);
        
        float mwLfo1 = modLfo1Mw ? currentModWheel : 1.0f;
        float mwLfo2 = modLfo2Mw ? currentModWheel : 1.0f;
        float mwVel = modVelMw ? currentModWheel : 1.0f;
        float mwAt = modAtMw ? currentModWheel : 1.0f;
        float mwPb = modPbMw ? currentModWheel : 1.0f;

        float modFilterStart = (lfo1ValStart * modLfo1Filter * mwLfo1 * 5.0f) + 
                               (lfo2ValStart * modLfo2Filter * mwLfo2 * 5.0f) + 
                               (currentVelocity * modVelFilter * mwVel) + 
                               (currentAftertouch * modAtFilter * mwAt) +
                               (currentPitchBend * modPbFilter * mwPb);

        float modulatedCutoff = currentFilterCutoff * std::pow(2.0f, (currentEnv1 * currentFilterEgDepth) + modFilterStart);
        modulatedCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);
        
        voiceFilter.setCutoffFrequencyHz(modulatedCutoff);
        voiceFilter.setResonance(currentFilterResonance);
        voiceFilter.setDrive(currentFilterDrive);

        // 1. Render oscillators for this sub-block
        for (int s = 0; s < numThisTime; ++s) {
            int bufferIdx = currentStartSample + s;
            
            // Advance env1 per sample to keep it in sync, even if we only used the start value for the filter
            if (s > 0 && !isParaOrMono) {
                float e1 = env1.getNextSample();
                if (useExpEnv) e1 = e1 * e1;
                lastEnv1Value = e1;
            }

            // For the first sample of the sub-block, we use the values we already calculated
            float lfo1Val = (s == 0) ? lfo1ValStart : lfo1.processSample(0.0f);
            float lfo2Val = (s == 0) ? lfo2ValStart : lfo2.processSample(0.0f);
            
            // Pitch bend quantized to semitones
            float pbPitchOffset = std::round(currentPitchBend * modPbPitch);

            float modPitch = (lfo1Val * modLfo1Pitch * mwLfo1) + (lfo2Val * modLfo2Pitch * mwLfo2) + 
                             (currentVelocity * modVelPitch * mwVel) + (currentAftertouch * modAtPitch * mwAt) +
                             (pbPitchOffset * mwPb);

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

        // 2. Process filter for this sub-block
        auto subBlock = juce::dsp::AudioBlock<float> (synthBuffer).getSubBlock((size_t)currentStartSample, (size_t)numThisTime);
        
        if (currentFilterDrive > 1.0f)
            subBlock.multiplyBy(1.0f + (currentFilterDrive - 1.0f) * 0.15f);

        juce::dsp::ProcessContextReplacing<float> context (subBlock);
        voiceFilter.process(context);
        
        // 3. Apply amplitude envelope for this sub-block
        for (int s = 0; s < numThisTime; ++s) {
            int bufferIdx = currentStartSample + s;
            
            float env2Val = 0.0f;
            if (isParaOrMono && paraState)
                env2Val = paraState->env2Buffer.getSample(0, bufferIdx);
            else
            {
                env2Val = env2.getNextSample();
                if (useExpEnv) env2Val = env2Val * env2Val;
            }
            
            // To keep it simple and fast, we'll use the start value for modAmp in this sub-block
            // since amp smoothing is less noticeable than filter smoothing.
            float modAmpStart = (lfo1ValStart * modLfo1Amp * mwLfo1) + (lfo2ValStart * modLfo2Amp * mwLfo2) + 
                                (currentVelocity * modVelAmp * mwVel) + (currentAftertouch * modAtAmp * mwAt) +
                                (currentPitchBend * modPbAmp * mwPb);
            
            float modAmpGain = juce::jlimit(0.0f, 1.0f, 1.0f + modAmpStart);

            float fadeGain = 1.0f;
            if (fadeInCounter >= 0 && fadeInCounter < fadeInSamples) {
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

    // Clear voice if fade-out complete or envelope finished
    if (isParaOrMono)
    {
        // Para modes need to wait for BOTH fade-out AND shared envelope release
        bool isParaL = (currentVoiceMode == 2);
        bool isPara = (currentVoiceMode == 3);
        
        if (isParaL || isPara)
        {
            // For Para: wait for fade to complete OR shared envelope to finish
            bool fadeComplete = (fadeOutCounter >= fadeOutSamples && fadeOutCounter != -1);
            bool envelopeFinished = (paraState && paraState->activeNotes == 0 && !paraState->env2.isActive());
            
            if (fadeComplete || envelopeFinished)
                clearCurrentNote();
        }
        else
        {
            // For Mono modes: keep rendering through the shared release tail
            if (paraState && paraState->activeNotes == 0 && !paraState->env2.isActive())
                clearCurrentNote();
        }
    }
    else
    {
        if ((fadeOutCounter >= fadeOutSamples) || !env2.isActive())
            clearCurrentNote();
    }
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
    paraState = std::make_shared<ParaphonicState>();
    for (int i = 0; i < 8; ++i) {
        auto* v = new Neon37Voice();
        v->setParaState(paraState);
        synth.addVoice (v);
    }
    
    synth.addSound (new Neon37Sound());
}

Neon37AudioProcessor::~Neon37AudioProcessor()
{
}

const juce::String Neon37AudioProcessor::getName() const
{
    return "Neon37";
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
    paraState->prepare(sampleRate, samplesPerBlock);
    
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

    int voiceMode = (int)*apvts.getRawParameterValue("voice_mode");
    bool monoMode = (voiceMode == 0 || voiceMode == 1);
    bool paraMode = (voiceMode == 2 || voiceMode == 3);
    bool holdMode = *apvts.getRawParameterValue("hold_mode") > 0.5f;
    bool expEnv = *apvts.getRawParameterValue("env_exp_curv") > 0.5f;
    
    // Panic on mode change
    if (voiceMode != lastVoiceMode)
    {
        for (int i = 0; i < synth.getNumVoices(); ++i)
            synth.getVoice(i)->stopNote(0.0f, false);
        heldNotes.clear();
        physicallyHeldNotes.clear();
        currentPlayingNote = -1;
        paraState->activeNotes = 0;
        paraState->env1.reset();
        paraState->env2.reset();
        lastVoiceMode = voiceMode;
    }

    // Update Paraphonic State Envelopes
    juce::ADSR::Parameters p1, p2;
    p1.attack = *apvts.getRawParameterValue("env1_attack");
    p1.decay = *apvts.getRawParameterValue("env1_decay");
    p1.sustain = *apvts.getRawParameterValue("env1_sustain");
    p1.release = *apvts.getRawParameterValue("env1_release");
    p2.attack = *apvts.getRawParameterValue("env2_attack");
    p2.decay = *apvts.getRawParameterValue("env2_decay");
    p2.sustain = *apvts.getRawParameterValue("env2_sustain");
    p2.release = *apvts.getRawParameterValue("env2_release");
    paraState->env1.setParameters(p1);
    paraState->env2.setParameters(p2);

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

            // Envelope/key-gate handling differs by architecture:
            // - Para/Poly: scanned keyboard; handle here.
            // - Mono: keybed-as-potentiometer; handled in the mono branch below.
            if (paraMode)
            {
                // Paraphonic retrigger logic
                if (voiceMode == 3) // Para (retrigger all)
                {
                    paraState->env1.reset();
                    paraState->env2.reset();
                    paraState->env1.noteOn();
                    paraState->env2.noteOn();
                    paraState->justNoteOn = true;
                }
                else
                {
                    // Para-L: only trigger on first key press
                    if (paraState->activeNotes == 0)
                    {
                        paraState->env1.reset();
                        paraState->env2.reset();
                        paraState->env1.noteOn();
                        paraState->env2.noteOn();
                        paraState->justNoteOn = true;
                    }
                }

                paraState->activeNotes++;
            }
        }
        else if (msg.isNoteOff())
        {
            physicallyHeldNotes.erase(std::remove(physicallyHeldNotes.begin(), physicallyHeldNotes.end(), msg.getNoteNumber()), physicallyHeldNotes.end());
            
            if (!holdMode)
            {
                if (paraMode)
                {
                    // Para modes: decrement activeNotes and stop the specific voice
                    paraState->activeNotes = std::max(0, paraState->activeNotes - 1);
                    
                    for (int i = 0; i < synth.getNumVoices(); ++i)
                    {
                        if (auto v = dynamic_cast<Neon37Voice*>(synth.getVoice(i)))
                        {
                            if (v->getCurrentlyPlayingNote() == msg.getNoteNumber())
                            {
                                v->stopNote(0.0f, true);  // Allow tail-off for fade-out
                            }
                        }
                    }
                    
                    // Turn off shared envelopes only when all notes released
                    if (paraState->activeNotes == 0)
                    {
                        paraState->env1.noteOff();
                        paraState->env2.noteOff();
                    }
                }
                else if (monoMode)
                {
                    // Mono modes: handled in mono branch below
                    processedMidi.addEvent(msg, metadata.samplePosition);
                }
                else
                {
                    // Poly mode: normal note-off handling
                    processedMidi.addEvent(msg, metadata.samplePosition);
                }
            }
        }
        else
        {
            processedMidi.addEvent(msg, metadata.samplePosition);
        }
    }

    if (monoMode)
    {
        // Mono architecture: emulate keybed-as-potentiometer.
        // - One "CV" (pitch) value at a time (Mono-L: lowest note priority, Mono: last note priority)
        // - One gate: HIGH if any key held, LOW if no keys held
        // - Mono-L pitch changes do NOT generate new note-ons (prevents multiple voices/pitches)
        const bool isMonoL = (voiceMode == 0);

        // Update held notes from the incoming MIDI (scanned input)
        for (const auto metadata : processedMidi)
        {
            const auto msg = metadata.getMessage();
            if (msg.isNoteOn())
            {
                if (std::find(heldNotes.begin(), heldNotes.end(), msg.getNoteNumber()) == heldNotes.end())
                    heldNotes.push_back(msg.getNoteNumber());
            }
            else if (msg.isNoteOff())
            {
                heldNotes.erase(std::remove(heldNotes.begin(), heldNotes.end(), msg.getNoteNumber()), heldNotes.end());
            }
        }

        const bool gateHigh = !heldNotes.empty();
        const int desiredNote = gateHigh ?
            (isMonoL ? *std::min_element(heldNotes.begin(), heldNotes.end()) : heldNotes.back()) :
            -1;

        juce::MidiBuffer monoBuffer;

        if (!gateHigh)
        {
            // Gate LOW: release shared envelopes (tail continues while voices render)
            if (paraState->activeNotes != 0)
            {
                paraState->activeNotes = 0;
                paraState->env1.noteOff();
                paraState->env2.noteOff();
            }
            currentPlayingNote = -1;
        }
        else
        {
            // Gate HIGH: ensure envelopes are running
            if (paraState->activeNotes == 0)
            {
                paraState->env1.reset();
                paraState->env2.reset();
                paraState->env1.noteOn();
                paraState->env2.noteOn();
                paraState->justNoteOn = true;
                paraState->activeNotes = 1;

                // Start exactly one voice for the gate rising edge
                for (int i = 0; i < synth.getNumVoices(); ++i)
                    synth.getVoice(i)->stopNote(0.0f, false);

                monoBuffer.addEvent(juce::MidiMessage::noteOn(1, desiredNote, 1.0f), 0);
                currentPlayingNote = desiredNote;
            }
            else
            {
                // Gate already high
                if (!isMonoL)
                {
                    // Mono (retrigger): restart envelopes and voice on every new desired pitch
                    if (desiredNote != currentPlayingNote)
                    {
                        paraState->env1.reset();
                        paraState->env2.reset();
                        paraState->env1.noteOn();
                        paraState->env2.noteOn();
                        paraState->justNoteOn = true;
                        paraState->activeNotes = 1;

                        for (int i = 0; i < synth.getNumVoices(); ++i)
                            synth.getVoice(i)->stopNote(0.0f, false);

                        monoBuffer.addEvent(juce::MidiMessage::noteOn(1, desiredNote, 1.0f), 0);
                        currentPlayingNote = desiredNote;
                    }
                }
                else
                {
                    // Mono-L: retune the single running voice without generating a new note-on
                    if (desiredNote != currentPlayingNote)
                    {
                        Neon37Voice* active = nullptr;
                        for (int i = 0; i < synth.getNumVoices(); ++i)
                        {
                            if (auto v = dynamic_cast<Neon37Voice*>(synth.getVoice(i)))
                            {
                                if (v->isVoiceActive())
                                {
                                    if (!active) active = v;
                                    else v->stopNote(0.0f, false); // enforce single voice
                                }
                            }
                        }

                        if (active)
                            active->retuneToMidiNote(desiredNote);
                        else
                            monoBuffer.addEvent(juce::MidiMessage::noteOn(1, desiredNote, 1.0f), 0);

                        currentPlayingNote = desiredNote;
                    }
                }
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
    for (const auto metadata : processedMidi)
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
        else if (msg.isPitchWheel())
        {
            for (int i = 0; i < synth.getNumVoices(); ++i)
                if (auto v = dynamic_cast<Neon37Voice*>(synth.getVoice(i)))
                    v->pitchWheelMoved(msg.getPitchWheelValue());
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
                voiceMode,
                activeVoices
            );
        }
    }

    // Calculate shared envelopes for Para/Mono modes AFTER MIDI processing
    if (paraMode || monoMode)
        paraState->calculate(buffer.getNumSamples(), expEnv);

    // Render Synth with per-voice filtering
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // Apply Master Gain
    float masterVolume = *apvts.getRawParameterValue ("master_volume");
    masterGain.setGainDecibels (masterVolume - 6.0f);
    
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
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc1_wave", "Osc 1 Wave", juce::StringArray { "Triangle", "Sawtooth", "Square", "25% Pulse", "10% Pulse" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc1_octave", "Osc 1 Octave", juce::StringArray { "16'", "8'", "4'", "2'" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc1_semitones", "Osc 1 Semitones", -12.0f, 12.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc1_fine", "Osc 1 Fine", -0.5f, 0.5f, 0.0f)); // ±50 cents
    
    // Oscillator 2
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc2_wave", "Osc 2 Wave", juce::StringArray { "Triangle", "Sawtooth", "Square", "25% Pulse", "10% Pulse" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc2_octave", "Osc 2 Octave", juce::StringArray { "16'", "8'", "4'", "2'" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2_semitones", "Osc 2 Semitones", -12.0f, 12.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2_fine", "Osc 2 Fine", -0.5f, 0.5f, 0.0f)); // ±50 cents

    // Global Osc
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc_freq", "Osc Frequency", -7.0f, 7.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc_beat", "Beat Rate", -3.5f, 3.5f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("hard_sync", "Hard Sync", false));

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
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("voice_mode", "Voice Mode", juce::StringArray { "Mono-L", "Mono", "Para-L", "Para", "Poly" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("hold_mode", "Hold Mode", false));
    
    // Glide/Gliss - defaults to OFF (time = 0)
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("gliss_time", "Gliss Time", juce::NormalisableRange<float> (0.0f, 10.0f, 0.01f, 0.3f), 0.0f)); // Default 0 = OFF
    params.push_back (std::make_unique<juce::AudioParameterBool> ("gliss_rte", "Gliss RTE", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("gliss_tme", "Gliss TME", true)); // Default mode
    params.push_back (std::make_unique<juce::AudioParameterBool> ("gliss_log", "Gliss LOG", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("gliss_on_gat_leg", "Gliss On Gat Leg", false));

    // LFO 1
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo1_rate", "LFO 1 Rate", juce::NormalisableRange<float> (0.01f, 50.0f, 0.01f, 0.3f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("lfo1_sync", "LFO 1 Sync", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("lfo1_wave", "LFO 1 Wave", juce::StringArray { "Sine", "Saw Up", "Saw Down", "Square", "S&H" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo1_pitch", "LFO 1 Pitch", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo1_filter", "LFO 1 Filter", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo1_amp", "LFO 1 Amp", 0.0f, 1.0f, 0.0f));

    // LFO 2
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo2_rate", "LFO 2 Rate", juce::NormalisableRange<float> (0.01f, 50.0f, 0.01f, 0.3f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("lfo2_sync", "LFO 2 Sync", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("lfo2_wave", "LFO 2 Wave", juce::StringArray { "Sine", "Saw Up", "Saw Down", "Square", "S&H" }, 0));
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
