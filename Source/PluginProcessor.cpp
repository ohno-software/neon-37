#include "PluginProcessor.h"
#include "PluginEditor.h"

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
}

Neon37AudioProcessor::~Neon37AudioProcessor()
{
}

const juce::String Neon37AudioProcessor::getName() const
{
    return "Neon37-r";
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
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
    // Initialize MONO filter
    monoFilter.prepare(spec);
    monoFilter.setMode(juce::dsp::LadderFilterMode::LPF24);
    monoFilter.setEnabled(true);
    
    // Set filter to match APVTS values and reset to avoid startup transients
    float cutoff = apvts.getRawParameterValue("cutoff")->load();
    float resonance = apvts.getRawParameterValue("resonance")->load();
    monoFilter.setCutoffFrequencyHz(cutoff);
    monoFilter.setResonance(resonance);
    monoFilter.reset();
    
    // Initialize MONO filter envelope
    monoFilterEnv.setSampleRate(sampleRate);
    juce::ADSR::Parameters monoFilterEnvParams;
    monoFilterEnvParams.attack = apvts.getRawParameterValue("env1_attack")->load();
    monoFilterEnvParams.decay = apvts.getRawParameterValue("env1_decay")->load();
    monoFilterEnvParams.sustain = apvts.getRawParameterValue("env1_sustain")->load();
    monoFilterEnvParams.release = apvts.getRawParameterValue("env1_release")->load();
    monoFilterEnv.setParameters(monoFilterEnvParams);
    
    // Initialize MONO amplitude envelope
    monoAmpEnv.setSampleRate(sampleRate);
    juce::ADSR::Parameters monoAmpEnvParams;
    monoAmpEnvParams.attack = apvts.getRawParameterValue("env2_attack")->load();
    monoAmpEnvParams.decay = apvts.getRawParameterValue("env2_decay")->load();
    monoAmpEnvParams.sustain = apvts.getRawParameterValue("env2_sustain")->load();
    monoAmpEnvParams.release = apvts.getRawParameterValue("env2_release")->load();
    monoAmpEnv.setParameters(monoAmpEnvParams);
    
    // Initialize paraphonic voices
    for (int i = 0; i < MAX_VOICES; ++i)
    {
        // Amp gate: gates the oscillators on/off for click-free operation (paraphonic modes only)
        // Separate from the shared monoAmpEnv envelope
        voices[i].ampGate.setSampleRate(sampleRate);
        juce::ADSR::Parameters gateParams;
        gateParams.attack = 0.003f;   // 3ms attack (prevents clicks on note-on)
        gateParams.decay = 0.0f;
        gateParams.sustain = 1.0f;
        gateParams.release = 0.010f;  // 10ms release (lets ADSR release envelope breathe)
        voices[i].ampGate.setParameters(gateParams);
        
        // Per-voice filter (poly mode)
        voices[i].filter.prepare(spec);
        voices[i].filter.setMode(juce::dsp::LadderFilterMode::LPF24);
        voices[i].filter.setEnabled(true);
        voices[i].filter.setCutoffFrequencyHz(cutoff);
        voices[i].filter.setResonance(resonance);
        voices[i].filter.reset();
        
        // Per-voice filter envelope (poly mode)
        voices[i].filterEnv.setSampleRate(sampleRate);
        juce::ADSR::Parameters polyFilterEnvParams;
        polyFilterEnvParams.attack = apvts.getRawParameterValue("env1_attack")->load();
        polyFilterEnvParams.decay = apvts.getRawParameterValue("env1_decay")->load();
        polyFilterEnvParams.sustain = apvts.getRawParameterValue("env1_sustain")->load();
        polyFilterEnvParams.release = apvts.getRawParameterValue("env1_release")->load();
        voices[i].filterEnv.setParameters(polyFilterEnvParams);
        
        // Per-voice amp envelope (poly mode)
        voices[i].ampEnv.setSampleRate(sampleRate);
        juce::ADSR::Parameters polyAmpEnvParams;
        polyAmpEnvParams.attack = apvts.getRawParameterValue("env2_attack")->load();
        polyAmpEnvParams.decay = apvts.getRawParameterValue("env2_decay")->load();
        polyAmpEnvParams.sustain = apvts.getRawParameterValue("env2_sustain")->load();
        polyAmpEnvParams.release = apvts.getRawParameterValue("env2_release")->load();
        voices[i].ampEnv.setParameters(polyAmpEnvParams);
        
        // Voice output buffer
        voices[i].voiceBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    }
    
    // Initialize output gain to unity
    outputGain.prepare(spec);
    outputGain.setGainLinear(1.0f);
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
    
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // Process MIDI messages for pitch and envelope control
    int voiceMode = (int)*apvts.getRawParameterValue("voice_mode");
    
    // For paraphonic modes: track which notes were released this block (to deallocate voices)
    juce::Array<int> releasedNotes;
    
    // Process all MIDI messages in a single loop
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        
        // Track mod wheel from CC1
        if (msg.isController() && msg.getControllerNumber() == 1)  // CC1 = Mod Wheel
        {
            modWheelValue = msg.getControllerValue() / 127.0f;  // Normalize to 0-1
            continue;  // Skip to next message
        }
        
        // Track channel aftertouch (Channel Pressure)
        if (msg.isChannelPressure())
        {
            currentAftertouch = msg.getChannelPressureValue() / 127.0f;  // Normalize to 0-1
            
            // In poly mode, apply channel aftertouch to all active voices
            if (voiceMode == 4)
            {
                for (int i = 0; i < MAX_VOICES; ++i)
                {
                    if (voices[i].active)
                    {
                        voices[i].aftertouch = currentAftertouch;
                    }
                }
            }
            continue;
        }
        
        // Also track polyphonic aftertouch (per-note pressure)
        if (msg.isAftertouch())
        {
            int aftertouchNote = msg.getNoteNumber();
            float aftertouchValue = msg.getAfterTouchValue() / 127.0f;
            currentAftertouch = aftertouchValue;  // Also update global for MONO/Paraphonic modes
            
            // In poly mode, apply aftertouch only to the specific note
            if (voiceMode == 4)
            {
                for (int i = 0; i < MAX_VOICES; ++i)
                {
                    if (voices[i].active && voices[i].midiNote == aftertouchNote)
                    {
                        voices[i].aftertouch = aftertouchValue;
                        break;
                    }
                }
            }
            continue;
        }
        
        // Track pitch bend
        if (msg.isPitchWheel())
        {
            pitchBendValue = (msg.getPitchWheelValue() - 8192.0f) / 8192.0f;  // Normalize to -1 to +1
            continue;
        }
        if (msg.isNoteOn())
        {
            int midiNote = msg.getNoteNumber();
            // Track velocity (0-127 normalized to 0-1)
            currentVelocity = msg.getVelocity() / 127.0f;
            
            if (voiceMode == 0 || voiceMode == 1)  // MONO or MONO-L
            {
                currentMidiNote = midiNote;
                heldNoteCount++;
                noteStack.push_back(midiNote);  // Add to note stack
                
                // Envelope retrigger logic:
                // Mono (mode 1): Always retrigger
                // Mono-L (mode 0): Only retrigger if no note was previously held (staccato)
                bool shouldRetrigger = false;
                if (voiceMode == 1) // Mono
                {
                    shouldRetrigger = true;
                }
                else if (voiceMode == 0 && heldNoteCount == 1) // Mono-L, first note pressed
                {
                    shouldRetrigger = true;
                }
                
                if (shouldRetrigger)
                {
                    monoFilterEnv.noteOn();
                    monoAmpEnv.noteOn();
                }
            }
            else if (voiceMode == 2 || voiceMode == 3)  // Paraphonic modes (Para-L, Para)
            {
                // Allocate voice (refactored into helper)
                int voiceToAllocate = allocateVoice();
                
                // Allocate voice
                voices[voiceToAllocate].midiNote = midiNote;
                voices[voiceToAllocate].active = true;
                voices[voiceToAllocate].allocationTimestamp = ++voiceAllocationCounter;
                voices[voiceToAllocate].ampGate.noteOn();
                
                // Envelope retrigger logic for paraphonic modes:
                // Para-L (mode 2): Only retrigger if this is the first note after all notes were released
                // Para (mode 3): Always retrigger on every new note-on
                bool shouldRetriggerEnvelopes = false;
                if (voiceMode == 2)  // Para-L
                {
                    // Only retrigger if no voices were active last block (transition from silence to notes)
                    shouldRetriggerEnvelopes = !lastBlockHadAnyActiveVoices;
                }
                else  // Para (mode 3) - always retrigger
                {
                    shouldRetriggerEnvelopes = true;
                }
                
                if (shouldRetriggerEnvelopes)
                {
                    monoFilterEnv.noteOn();
                    monoAmpEnv.noteOn();
                }
            }
            else if (voiceMode == 4)  // Poly mode
            {
                // Allocate voice (refactored into helper)
                int voiceToAllocate = allocateVoice();
                
                // Allocate voice
                voices[voiceToAllocate].midiNote = midiNote;
                voices[voiceToAllocate].active = true;
                voices[voiceToAllocate].allocationTimestamp = ++voiceAllocationCounter;
                voices[voiceToAllocate].velocity = currentVelocity;  // Store this note's velocity
                voices[voiceToAllocate].aftertouch = 0.0f;  // Initialize aftertouch to 0
                
                // Trigger per-voice envelopes (always retrigger in poly mode, like MONO)
                voices[voiceToAllocate].filterEnv.noteOn();
                voices[voiceToAllocate].ampEnv.noteOn();
                voices[voiceToAllocate].filter.reset();  // Clear filter state to avoid startup transients
            }
        }
        else if (msg.isNoteOff())
        {
            int midiNote = msg.getNoteNumber();
            
            if (voiceMode == 0 || voiceMode == 1)  // MONO or MONO-L
            {
                heldNoteCount--;
                
                // Remove from note stack
                auto it = std::find(noteStack.begin(), noteStack.end(), midiNote);
                if (it != noteStack.end())
                {
                    noteStack.erase(it);
                }
                
                // If there are still held notes, switch to the most recent one
                if (heldNoteCount > 0 && !noteStack.empty())
                {
                    currentMidiNote = noteStack.back();  // Switch to most recent held note
                    // For Mono mode, retrigger the envelope on note switch
                    if (voiceMode == 1)
                    {
                        monoFilterEnv.noteOn();
                        monoAmpEnv.noteOn();
                    }
                }
                else
                {
                    // All keys released
                    heldNoteCount = 0;
                    noteStack.clear();
                    monoFilterEnv.noteOff();
                    monoAmpEnv.noteOff();
                }
            }
            else if (voiceMode == 2 || voiceMode == 3)  // Paraphonic modes
            {
                releasedNotes.add(midiNote);
            }
            else if (voiceMode == 4)  // Poly mode
            {
                // Find voice matching this note and trigger both filter and amp envelope release
                for (int i = 0; i < MAX_VOICES; ++i)
                {
                    if (voices[i].active && voices[i].midiNote == midiNote)
                    {
                        voices[i].filterEnv.noteOff();
                        voices[i].ampEnv.noteOff();
                        break;
                    }
                }
            }
        }
    }
    
    // Update filter envelope parameters in real-time (only if changed)
    float env1Attack = apvts.getRawParameterValue("env1_attack")->load();
    float env1Decay = apvts.getRawParameterValue("env1_decay")->load();
    float env1Sustain = apvts.getRawParameterValue("env1_sustain")->load();
    float env1Release = apvts.getRawParameterValue("env1_release")->load();
    
    if (env1Attack != cachedEnv1Attack || env1Decay != cachedEnv1Decay || 
        env1Sustain != cachedEnv1Sustain || env1Release != cachedEnv1Release)
    {
        cachedEnv1Attack = env1Attack;
        cachedEnv1Decay = env1Decay;
        cachedEnv1Sustain = env1Sustain;
        cachedEnv1Release = env1Release;
        
        juce::ADSR::Parameters filterEnvParams;
        filterEnvParams.attack = env1Attack;
        filterEnvParams.decay = env1Decay;
        filterEnvParams.sustain = env1Sustain;
        filterEnvParams.release = env1Release;
        monoFilterEnv.setParameters(filterEnvParams);
        
        // Update poly mode per-voice filter envelopes too
        if (voiceMode == 4)
        {
            for (int i = 0; i < MAX_VOICES; ++i)
            {
                voices[i].filterEnv.setParameters(filterEnvParams);
            }
        }
    }
    
    // Update amplitude envelope parameters in real-time (only if changed)
    float env2Attack = apvts.getRawParameterValue("env2_attack")->load();
    float env2Decay = apvts.getRawParameterValue("env2_decay")->load();
    float env2Sustain = apvts.getRawParameterValue("env2_sustain")->load();
    float env2Release = apvts.getRawParameterValue("env2_release")->load();
    
    if (env2Attack != cachedEnv2Attack || env2Decay != cachedEnv2Decay || 
        env2Sustain != cachedEnv2Sustain || env2Release != cachedEnv2Release)
    {
        cachedEnv2Attack = env2Attack;
        cachedEnv2Decay = env2Decay;
        cachedEnv2Sustain = env2Sustain;
        cachedEnv2Release = env2Release;
        
        juce::ADSR::Parameters ampEnvParams;
        ampEnvParams.attack = env2Attack;
        ampEnvParams.decay = env2Decay;
        ampEnvParams.sustain = env2Sustain;
        ampEnvParams.release = env2Release;
        monoAmpEnv.setParameters(ampEnvParams);
        
        // Update poly mode per-voice amp envelopes too
        if (voiceMode == 4)
        {
            for (int i = 0; i < MAX_VOICES; ++i)
            {
                voices[i].ampEnv.setParameters(ampEnvParams);
            }
        }
    }
    
    // === LFO RENDERING ===
    // Update LFO1 parameters
    float lfo1Rate = apvts.getRawParameterValue("lfo1_rate")->load();
    bool lfo1SyncEnabled = (bool)*apvts.getRawParameterValue("lfo1_sync");
    if (lfo1SyncEnabled)
    {
        int lfo1SyncVal = (int)*apvts.getRawParameterValue("lfo1_sync_val");
        lfo1.rate = getSyncMultiplier(lfo1SyncVal) * 2.0f;  // Tempo-synced (2 Hz as base rate)
    }
    else
    {
        lfo1.rate = lfo1Rate;
    }
    lfo1.waveform = (int)*apvts.getRawParameterValue("lfo1_wave");
    lfo1.pitchAmount = apvts.getRawParameterValue("lfo1_pitch")->load();
    lfo1.filterAmount = apvts.getRawParameterValue("lfo1_filter")->load();
    lfo1.ampAmount = apvts.getRawParameterValue("lfo1_amp")->load();
    
    // Update LFO2 parameters
    float lfo2Rate = apvts.getRawParameterValue("lfo2_rate")->load();
    bool lfo2SyncEnabled = (bool)*apvts.getRawParameterValue("lfo2_sync");
    if (lfo2SyncEnabled)
    {
        int lfo2SyncVal = (int)*apvts.getRawParameterValue("lfo2_sync_val");
        lfo2.rate = getSyncMultiplier(lfo2SyncVal) * 2.0f;  // Tempo-synced (2 Hz as base rate)
    }
    else
    {
        lfo2.rate = lfo2Rate;
    }
    lfo2.waveform = (int)*apvts.getRawParameterValue("lfo2_wave");
    lfo2.pitchAmount = apvts.getRawParameterValue("lfo2_pitch")->load();
    lfo2.filterAmount = apvts.getRawParameterValue("lfo2_filter")->load();
    lfo2.ampAmount = apvts.getRawParameterValue("lfo2_amp")->load();
    
    // Check if mod wheel scaling is enabled
    modWheelEnabled = (bool)*apvts.getRawParameterValue("mw_enable");
    float modWheelScale = modWheelEnabled ? modWheelValue : 1.0f;
    
    // Advance LFO phases for this block and generate waveforms
    float phaseIncrement1 = (lfo1.rate / (float)currentSampleRate) * juce::MathConstants<float>::twoPi;
    float phaseIncrement2 = (lfo2.rate / (float)currentSampleRate) * juce::MathConstants<float>::twoPi;
    
    lfo1.phase += phaseIncrement1 * buffer.getNumSamples();
    lfo2.phase += phaseIncrement2 * buffer.getNumSamples();
    
    // Wrap phases to [0, 2π)
    while (lfo1.phase >= juce::MathConstants<float>::twoPi)
        lfo1.phase -= juce::MathConstants<float>::twoPi;
    while (lfo2.phase >= juce::MathConstants<float>::twoPi)
        lfo2.phase -= juce::MathConstants<float>::twoPi;
    
    // Generate LFO waveforms (output range: -1 to +1, representing -100% to +100%)
    float lfo1Output = generateLFOWaveform(lfo1.phase, lfo1.waveform);
    float lfo2Output = generateLFOWaveform(lfo2.phase, lfo2.waveform);
    
    // Calculate modulation amounts (all scaled by mod wheel if enabled)
    // LFO output is bipolar: -1 to +1 representing -100% to +100%
    float lfoPitchMod = lfo1Output * lfo1.pitchAmount + lfo2Output * lfo2.pitchAmount;
    float lfoFilterMod = lfo1Output * lfo1.filterAmount + lfo2Output * lfo2.filterAmount;
    float lfoAmpMod = lfo1Output * lfo1.ampAmount + lfo2Output * lfo2.ampAmount;
    
    // === CALCULATE ALL MODULATIONS (refactored into helper) ===
    ModulationState modState;
    calculateAllModulations(modState, lfoFilterMod, lfoPitchMod, lfoAmpMod, modWheelScale);
    
    float totalPitchModSemitones = modState.pitchModSemitones;
    float totalFilterModMultiplier = modState.totalFilterModMultiplier;
    float totalAmpModMultiplier = modState.totalAmpModMultiplier;
    
    // Get oscillator parameters
    int osc1Wave = (int)*apvts.getRawParameterValue("osc1_wave");
    int osc1Octave = (int)*apvts.getRawParameterValue("osc1_octave");
    int osc1Semitones = (int)*apvts.getRawParameterValue("osc1_semitones");
    float osc1Fine = apvts.getRawParameterValue("osc1_fine")->load();
    
    int osc2Wave = (int)*apvts.getRawParameterValue("osc2_wave");
    int osc2Octave = (int)*apvts.getRawParameterValue("osc2_octave");
    int osc2Semitones = (int)*apvts.getRawParameterValue("osc2_semitones");
    float osc2Fine = apvts.getRawParameterValue("osc2_fine")->load();
    
    // Get mixer levels from parameters (in dB, so convert to linear)
    float mixerOsc1Db = apvts.getRawParameterValue("mixer_osc1")->load();
    float mixerOsc2Db = apvts.getRawParameterValue("mixer_osc2")->load();
    float mixerSub1Db = apvts.getRawParameterValue("mixer_sub1")->load();
    
    float mixerOsc1 = juce::Decibels::decibelsToGain(mixerOsc1Db);
    float mixerOsc2 = juce::Decibels::decibelsToGain(mixerOsc2Db);
    float mixerSub1 = juce::Decibels::decibelsToGain(mixerSub1Db);
    
    // Global oscillator level scaling to prevent overdrive (-12dB to keep headroom)
    constexpr float oscLevelScale = 0.25f;
    mixerOsc1 *= oscLevelScale;
    mixerOsc2 *= oscLevelScale;
    mixerSub1 *= oscLevelScale;
    
    // Get filter parameters
    float baseCutoff = apvts.getRawParameterValue("cutoff")->load();
    float resonance = apvts.getRawParameterValue("resonance")->load();
    float egDepth = apvts.getRawParameterValue("eg_depth")->load();
    float drive = apvts.getRawParameterValue("drive")->load();
    
    // Get master volume
    float masterVolDb = apvts.getRawParameterValue("master_volume")->load();
    float masterVol = juce::Decibels::decibelsToGain(masterVolDb);
    
    // Create a working buffer for synthesis
    juce::AudioBuffer<float> synthBuffer(totalNumOutputChannels, buffer.getNumSamples());
    synthBuffer.clear();
    
    // Create buffer for amplitude envelope values
    juce::AudioBuffer<float> ampEnvBuffer(1, buffer.getNumSamples());
    ampEnvBuffer.clear();
    
    // Generate and mix oscillators
    float twoPiOverSr = juce::MathConstants<float>::twoPi / (float)currentSampleRate;
    
    if (voiceMode == 0 || voiceMode == 1)  // MONO or MONO-L rendering
    {
        // Calculate base frequency from current MIDI note
        float baseFreq = 440.0f * std::pow(2.0f, (currentMidiNote - 69) / 12.0f);
        
        // Apply all pitch modulations (LFO, velocity, aftertouch, pitch bend)
        float lfoModFreq = baseFreq * std::pow(2.0f, totalPitchModSemitones / 12.0f);
        
        // Osc1 frequency (with LFO modulation)
        float osc1Freq = lfoModFreq * std::pow(2.0f, (float)osc1Octave) * std::pow(2.0f, (float)osc1Semitones / 12.0f) * std::pow(2.0f, osc1Fine / 12.0f);
        
        // Osc2 frequency (with LFO modulation)
        float osc2Freq = lfoModFreq * std::pow(2.0f, (float)osc2Octave) * std::pow(2.0f, (float)osc2Semitones / 12.0f) * std::pow(2.0f, osc2Fine / 12.0f);
        
        // Sub oscillator is one octave below osc1
        float subFreq = osc1Freq * 0.5f;
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Get envelope values
            float filterEnvValue = monoFilterEnv.getNextSample();
            float ampEnvValue = monoAmpEnv.getNextSample();
            ampEnvBuffer.setSample(0, sample, ampEnvValue);
            
            // Calculate and apply modulated cutoff
            float modulatedCutoff = calculateModulatedCutoff(baseCutoff, filterEnvValue, egDepth, totalFilterModMultiplier, resonance);
            monoFilter.setCutoffFrequencyHz(modulatedCutoff);
            monoFilter.setResonance(resonance);
            monoFilter.setDrive(drive);
            
            // Generate oscillator samples
            float osc1Sample = generateWaveform(osc1Phase, osc1Wave) * mixerOsc1;
            float osc2Sample = generateWaveform(osc2Phase, osc2Wave) * mixerOsc2;
            float subSample = generateWaveform(subOscPhase, 2) * mixerSub1;
            
            float mixed = osc1Sample + osc2Sample + subSample;
            
            for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            {
                synthBuffer.setSample(channel, sample, mixed);
            }
            
            // Update phases
            osc1Phase += twoPiOverSr * osc1Freq;
            osc2Phase += twoPiOverSr * osc2Freq;
            subOscPhase += twoPiOverSr * subFreq;
            
            // Wrap phases
            if (osc1Phase > juce::MathConstants<float>::twoPi) osc1Phase -= juce::MathConstants<float>::twoPi;
            if (osc2Phase > juce::MathConstants<float>::twoPi) osc2Phase -= juce::MathConstants<float>::twoPi;
            if (subOscPhase > juce::MathConstants<float>::twoPi) subOscPhase -= juce::MathConstants<float>::twoPi;
        }
    }
    else if (voiceMode == 2 || voiceMode == 3)  // Paraphonic rendering (modes 2, 3)
    {
        // Handle note-offs for paraphonic mode
        for (int note : releasedNotes)
        {
            for (int i = 0; i < MAX_VOICES; ++i)
            {
                if (voices[i].active && voices[i].midiNote == note)
                {
                    voices[i].ampGate.noteOff();
                    break;
                }
            }
        }
        
        // Track if any voice is still active before processing this block
        bool anyVoiceActiveBefore = false;
        int activeVoiceCount = 0;
        for (int i = 0; i < MAX_VOICES; ++i)
        {
            if (voices[i].active)
            {
                anyVoiceActiveBefore = true;
                ++activeVoiceCount;
            }
        }
        
        // Calculate voice scaling to prevent overdrive from multiple voices mixing
        float voiceGain = activeVoiceCount > 0 ? 1.0f / (float)activeVoiceCount : 1.0f;
        
        // Render each active voice
        for (int voiceIdx = 0; voiceIdx < MAX_VOICES; ++voiceIdx)
        {
            if (!voices[voiceIdx].active)
                continue;
            
            // Calculate frequencies for this voice (with all pitch modulations)
            float voiceBaseFreq = 440.0f * std::pow(2.0f, (voices[voiceIdx].midiNote - 69) / 12.0f);
            
            // Apply all pitch modulations (LFO, velocity, aftertouch, pitch bend)
            float voiceLfoModFreq = voiceBaseFreq * std::pow(2.0f, totalPitchModSemitones / 12.0f);
            
            float voiceOsc1Freq = voiceLfoModFreq * std::pow(2.0f, (float)osc1Octave) * std::pow(2.0f, (float)osc1Semitones / 12.0f) * std::pow(2.0f, osc1Fine / 12.0f);
            float voiceOsc2Freq = voiceLfoModFreq * std::pow(2.0f, (float)osc2Octave) * std::pow(2.0f, (float)osc2Semitones / 12.0f) * std::pow(2.0f, osc2Fine / 12.0f);
            float voiceSubFreq = voiceOsc1Freq * 0.5f;
            
            // Render voice's oscillators to voiceBuffer
            voices[voiceIdx].voiceBuffer.clear();
            
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                // Generate oscillator samples for this voice
                float osc1Sample = generateWaveform(voices[voiceIdx].osc1Phase, osc1Wave) * mixerOsc1;
                float osc2Sample = generateWaveform(voices[voiceIdx].osc2Phase, osc2Wave) * mixerOsc2;
                float subSample = generateWaveform(voices[voiceIdx].subOscPhase, 2) * mixerSub1;
                
                float mixed = osc1Sample + osc2Sample + subSample;
                
                // Apply voice's amp gate (gates the oscillators on/off)
                float gateValue = voices[voiceIdx].ampGate.getNextSample();
                mixed *= gateValue;
                
                for (int channel = 0; channel < totalNumOutputChannels; ++channel)
                {
                    voices[voiceIdx].voiceBuffer.addSample(channel, sample, mixed);
                }
                
                // Update voice's oscillator phases
                voices[voiceIdx].osc1Phase += twoPiOverSr * voiceOsc1Freq;
                voices[voiceIdx].osc2Phase += twoPiOverSr * voiceOsc2Freq;
                voices[voiceIdx].subOscPhase += twoPiOverSr * voiceSubFreq;
                
                // Wrap phases
                if (voices[voiceIdx].osc1Phase > juce::MathConstants<float>::twoPi) voices[voiceIdx].osc1Phase -= juce::MathConstants<float>::twoPi;
                if (voices[voiceIdx].osc2Phase > juce::MathConstants<float>::twoPi) voices[voiceIdx].osc2Phase -= juce::MathConstants<float>::twoPi;
                if (voices[voiceIdx].subOscPhase > juce::MathConstants<float>::twoPi) voices[voiceIdx].subOscPhase -= juce::MathConstants<float>::twoPi;
            }
            
            // Mix this voice's buffer to synthesis buffer
            for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            {
                synthBuffer.addFrom(channel, 0, voices[voiceIdx].voiceBuffer, channel, 0, buffer.getNumSamples(), voiceGain);
            }
            
            // Mark voice inactive if gate is fully released
            if (!voices[voiceIdx].ampGate.isActive())
            {
                voices[voiceIdx].active = false;
            }
        }
        
        // Check if all voices just became inactive (transition from at least one active to all inactive)
        bool anyVoiceActiveAfter = false;
        for (int i = 0; i < MAX_VOICES; ++i)
        {
            if (voices[i].active)
            {
                anyVoiceActiveAfter = true;
                break;
            }
        }
        
        // When last voice becomes inactive, trigger envelope release
        if (anyVoiceActiveBefore && !anyVoiceActiveAfter)
        {
            monoFilterEnv.noteOff();
            monoAmpEnv.noteOff();
        }
        
        // Generate envelope values for paraphonic mode
        // Continue processing as long as envelopes are still active (releasing)
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float filterEnvValue = monoFilterEnv.getNextSample();
            float ampEnvValue = monoAmpEnv.getNextSample();
            ampEnvBuffer.setSample(0, sample, ampEnvValue);
            
            // Calculate and apply modulated cutoff
            float modulatedCutoff = calculateModulatedCutoff(baseCutoff, filterEnvValue, egDepth, totalFilterModMultiplier, resonance);
            monoFilter.setCutoffFrequencyHz(modulatedCutoff);
            monoFilter.setResonance(resonance);
            monoFilter.setDrive(drive);
        }
        
        // Update tracking flag for next block's retrigger logic
        lastBlockHadAnyActiveVoices = anyVoiceActiveAfter;
    }
    else if (voiceMode == 4)  // Poly mode: full per-voice signal chain
    {
        // Handle note-offs for poly mode
        for (int note : releasedNotes)
        {
            for (int i = 0; i < MAX_VOICES; ++i)
            {
                if (voices[i].active && voices[i].midiNote == note)
                {
                    voices[i].ampEnv.noteOff();
                    break;
                }
            }
        }
        
        // Render each active voice with complete per-voice signal chain
        for (int voiceIdx = 0; voiceIdx < MAX_VOICES; ++voiceIdx)
        {
            if (!voices[voiceIdx].active)
                continue;
            
            // Calculate frequencies for this voice (with LFO pitch modulation)
            float voiceBaseFreq = 440.0f * std::pow(2.0f, (voices[voiceIdx].midiNote - 69) / 12.0f);
            
            // Apply all pitch modulations (LFO, velocity, aftertouch, pitch bend)
            float voiceLfoModFreq = voiceBaseFreq * std::pow(2.0f, totalPitchModSemitones / 12.0f);
            
            float voiceOsc1Freq = voiceLfoModFreq * std::pow(2.0f, (float)osc1Octave) * std::pow(2.0f, (float)osc1Semitones / 12.0f) * std::pow(2.0f, osc1Fine / 12.0f);
            float voiceOsc2Freq = voiceLfoModFreq * std::pow(2.0f, (float)osc2Octave) * std::pow(2.0f, (float)osc2Semitones / 12.0f) * std::pow(2.0f, osc2Fine / 12.0f);
            float voiceSubFreq = voiceOsc1Freq * 0.5f;
            
            // Render voice's oscillators
            voices[voiceIdx].voiceBuffer.clear();
            
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                // Generate oscillator samples for this voice
                float osc1Sample = generateWaveform(voices[voiceIdx].osc1Phase, osc1Wave) * mixerOsc1;
                float osc2Sample = generateWaveform(voices[voiceIdx].osc2Phase, osc2Wave) * mixerOsc2;
                float subSample = generateWaveform(voices[voiceIdx].subOscPhase, 2) * mixerSub1;
                
                float mixed = (osc1Sample + osc2Sample + subSample);
                
                for (int channel = 0; channel < totalNumOutputChannels; ++channel)
                {
                    voices[voiceIdx].voiceBuffer.setSample(channel, sample, mixed);
                }
                
                // Update voice's oscillator phases
                voices[voiceIdx].osc1Phase += twoPiOverSr * voiceOsc1Freq;
                voices[voiceIdx].osc2Phase += twoPiOverSr * voiceOsc2Freq;
                voices[voiceIdx].subOscPhase += twoPiOverSr * voiceSubFreq;
                
                // Wrap phases
                if (voices[voiceIdx].osc1Phase > juce::MathConstants<float>::twoPi) voices[voiceIdx].osc1Phase -= juce::MathConstants<float>::twoPi;
                if (voices[voiceIdx].osc2Phase > juce::MathConstants<float>::twoPi) voices[voiceIdx].osc2Phase -= juce::MathConstants<float>::twoPi;
                if (voices[voiceIdx].subOscPhase > juce::MathConstants<float>::twoPi) voices[voiceIdx].subOscPhase -= juce::MathConstants<float>::twoPi;
            }
            
            // Process this voice through its own filter with all modulations
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                float filterEnvValue = voices[voiceIdx].filterEnv.getNextSample();
                
                // === CALCULATE PER-VOICE FILTER MODULATION ===
                float velFilterAmount = *apvts.getRawParameterValue("vel_filter");
                float atFilterAmount = *apvts.getRawParameterValue("at_filter");
                
                // Calculate velocity and aftertouch modulation for this specific voice
                float velFilterMod = velFilterAmount * voices[voiceIdx].velocity;
                float atFilterMod = atFilterAmount * voices[voiceIdx].aftertouch;
                
                // Combine LFO + velocity + aftertouch for this voice's filter mod
                float voiceTotalFilterMod = lfoFilterMod + velFilterMod + atFilterMod;
                float voiceFilterModMultiplier = 1.0f + juce::jlimit(-5.0f, 5.0f, voiceTotalFilterMod);
                
                // Calculate and apply modulated cutoff
                float modulatedCutoff = calculateModulatedCutoff(baseCutoff, filterEnvValue, egDepth, voiceFilterModMultiplier, resonance);
                voices[voiceIdx].filter.setCutoffFrequencyHz(modulatedCutoff);
                voices[voiceIdx].filter.setResonance(resonance);
                voices[voiceIdx].filter.setDrive(drive);
            }
            
            // Apply per-voice filter
            juce::dsp::AudioBlock<float> voiceBlock(voices[voiceIdx].voiceBuffer);
            juce::dsp::ProcessContextReplacing<float> voiceContext(voiceBlock);
            voices[voiceIdx].filter.process(voiceContext);
            
            // Apply per-voice amplitude envelope and mix to output (with all modulations)
            // Generate all amp env samples first, then apply to all channels
            juce::AudioBuffer<float> voiceAmpEnvBuffer(1, buffer.getNumSamples());
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                float ampEnvValue = voices[voiceIdx].ampEnv.getNextSample();
                
                // === CALCULATE PER-VOICE AMPLITUDE MODULATION ===
                float velAmpAmount = *apvts.getRawParameterValue("vel_amp");
                float atAmpAmount = *apvts.getRawParameterValue("at_amp");
                
                // Calculate velocity and aftertouch modulation for this specific voice
                float velAmpMod = velAmpAmount * voices[voiceIdx].velocity;
                float atAmpMod = atAmpAmount * voices[voiceIdx].aftertouch;
                
                // Combine LFO + velocity + aftertouch for this voice's amp mod
                float voiceTotalAmpMod = lfoAmpMod + velAmpMod + atAmpMod;
                float voiceAmpModMultiplier = 1.0f + juce::jlimit(-5.0f, 5.0f, voiceTotalAmpMod);
                
                // Apply all amplitude modulations (LFO, velocity, aftertouch)
                ampEnvValue *= voiceAmpModMultiplier;
                voiceAmpEnvBuffer.setSample(0, sample, ampEnvValue);
            }
            
            // Mix to output with envelope scaling
            for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            {
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                {
                    float ampEnvValue = voiceAmpEnvBuffer.getSample(0, sample);
                    float voiceSample = voices[voiceIdx].voiceBuffer.getSample(channel, sample);
                    synthBuffer.addSample(channel, sample, voiceSample * ampEnvValue);
                }
            }
            
            // Don't mark voice inactive until envelope is fully released
            // Voice will continue rendering (silently) until ampEnv.isActive() returns false
            if (!voices[voiceIdx].ampEnv.isActive())
            {
                voices[voiceIdx].active = false;
            }
        }
    }
    
    // Process through shared filter (MONO and Paraphonic modes only)
    if (voiceMode != 4)  // Not poly mode
    {
        juce::dsp::AudioBlock<float> block(synthBuffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        monoFilter.process(context);
    }
    
    // Apply master volume and amplitude envelope (for MONO/Paraphonic) or just master volume (for Poly), then output
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    {
        buffer.copyFrom(channel, 0, synthBuffer, channel, 0, buffer.getNumSamples());
        
        if (voiceMode != 4)  // Not poly mode - apply shared amp envelope with all modulations
        {
            // Apply master volume and shared amplitude envelope per-sample
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                float ampValue = ampEnvBuffer.getSample(0, sample);
                // Apply all amplitude modulations (LFO, velocity, aftertouch)
                ampValue *= totalAmpModMultiplier;
                float sample_val = buffer.getSample(channel, sample);
                buffer.setSample(channel, sample, sample_val * masterVol * ampValue);
            }
        }
        else  // Poly mode - just apply master volume (per-voice envelopes already applied)
        {
            // Apply master volume only
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                float sample_val = buffer.getSample(channel, sample);
                buffer.setSample(channel, sample, sample_val * masterVol);
            }
        }
    }
}

float Neon37AudioProcessor::generateWaveform(float phase, int waveformType)
{
    // Normalize phase to 0-1
    float normPhase = phase / juce::MathConstants<float>::twoPi;
    normPhase = normPhase - std::floor(normPhase);
    
    switch (waveformType)
    {
        case 0: // Sine
            return std::sin(phase);
            
        case 1: // Triangle
            return 4.0f * std::abs(normPhase - 0.5f) - 1.0f;
            
        case 2: // Sawtooth
            return 2.0f * normPhase - 1.0f;
            
        case 3: // Square
            return normPhase < 0.5f ? 1.0f : -1.0f;
            
        case 4: // 25% Pulse
            return normPhase < 0.25f ? 1.0f : -1.0f;
            
        case 5: // 10% Pulse
            return normPhase < 0.10f ? 1.0f : -1.0f;
            
        default:
            return std::sin(phase);
    }
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
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("master_tune", "Master Tune", juce::NormalisableRange<float> (400.0f, 480.0f, 0.1f, 1.0f), 440.0f)); // A4 center

    // Oscillator 1
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc1_wave", "Osc 1 Wave", juce::StringArray { "Sine", "Triangle", "Sawtooth", "Square", "25% Pulse", "10% Pulse" }, 2)); // Default: Sawtooth
    params.push_back (std::make_unique<juce::AudioParameterInt> ("osc1_octave", "Osc 1 Octave", -3, 3, 0));
    params.push_back (std::make_unique<juce::AudioParameterInt> ("osc1_semitones", "Osc 1 Semitones", -12, 12, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc1_fine", "Osc 1 Fine", juce::NormalisableRange<float> (-0.5f, 0.5f, 0.01f, 1.0f), 0.0f)); // ±50 cents
    
    // Oscillator 2
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("osc2_wave", "Osc 2 Wave", juce::StringArray { "Sine", "Triangle", "Sawtooth", "Square", "25% Pulse", "10% Pulse" }, 2)); // Default: Sawtooth
    params.push_back (std::make_unique<juce::AudioParameterInt> ("osc2_octave", "Osc 2 Octave", -3, 3, 0));
    params.push_back (std::make_unique<juce::AudioParameterInt> ("osc2_semitones", "Osc 2 Semitones", -12, 12, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("osc2_fine", "Osc 2 Fine", juce::NormalisableRange<float> (-0.5f, 0.5f, 0.01f, 1.0f), 0.0f)); // ±50 cents

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
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("eg_depth", "EG Depth", juce::NormalisableRange<float> (-100.0f, 100.0f, 1.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("key_track", "Key Track", 0.0f, 2.0f, 0.0f));

    // Envelope 1 (Filter/Mod) - Exponential time range 3ms to 10s
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env1_attack", "Env 1 Attack", juce::NormalisableRange<float> (0.003f, 10.0f, 0.001f, 0.2f), 0.003f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env1_decay", "Env 1 Decay", juce::NormalisableRange<float> (0.003f, 10.0f, 0.001f, 0.2f), 0.05f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env1_sustain", "Env 1 Sustain", 0.0f, 1.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env1_release", "Env 1 Release", juce::NormalisableRange<float> (0.003f, 10.0f, 0.001f, 0.2f), 0.05f));

    // Envelope 2 (Amplitude) - Exponential time range 3ms to 10s
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env2_attack", "Env 2 Attack", juce::NormalisableRange<float> (0.003f, 10.0f, 0.001f, 0.2f), 0.003f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env2_decay", "Env 2 Decay", juce::NormalisableRange<float> (0.003f, 10.0f, 0.001f, 0.2f), 0.05f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env2_sustain", "Env 2 Sustain", 0.0f, 1.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("env2_release", "Env 2 Release", juce::NormalisableRange<float> (0.003f, 10.0f, 0.001f, 0.2f), 0.05f));
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
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo1_rate", "LFO 1 Rate", juce::NormalisableRange<float> (0.01f, 100.0f, 0.01f, 0.3f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("lfo1_sync", "LFO 1 Sync", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("lfo1_sync_val", "LFO 1 Sync Val", juce::StringArray { "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1", "3/1", "4/1", "8/1" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("lfo1_wave", "LFO 1 Wave", juce::StringArray { "Triangle", "Ramp Up", "Ramp Down", "Square", "S&H" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo1_pitch", "LFO 1 Pitch", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo1_filter", "LFO 1 Filter", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo1_amp", "LFO 1 Amp", 0.0f, 1.0f, 0.0f));

    // LFO 2
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo2_rate", "LFO 2 Rate", juce::NormalisableRange<float> (0.01f, 100.0f, 0.01f, 0.3f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("lfo2_sync", "LFO 2 Sync", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("lfo2_sync_val", "LFO 2 Sync Val", juce::StringArray { "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1", "3/1", "4/1", "8/1" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("lfo2_wave", "LFO 2 Wave", juce::StringArray { "Triangle", "Ramp Up", "Ramp Down", "Square", "S&H" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo2_pitch", "LFO 2 Pitch", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo2_filter", "LFO 2 Filter", 0.0f, 1.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("lfo2_amp", "LFO 2 Amp", 0.0f, 1.0f, 0.0f));

    // Velocity - Pitch: -12 to +12 semitones (snap), Filter and Amp: -200% to +500% (continuous)
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("vel_pitch", "Vel Pitch", juce::NormalisableRange<float>(-12.0f, 12.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("vel_filter", "Vel Filter", -5.0f, 5.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("vel_amp", "Vel Amp", -2.0f, 2.0f, 0.0f));

    // Aftertouch - Pitch: -12 to +12 semitones (snap), Filter and Amp: -200% to +500% (continuous)
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("at_pitch", "AT Pitch", juce::NormalisableRange<float>(-12.0f, 12.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("at_filter", "AT Filter", -5.0f, 5.0f, 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("at_amp", "AT Amp", -2.0f, 2.0f, 0.0f));

    // Pitch Bend - Pitch: 1-12 semitones (snap), no filter/amp
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("pb_pitch", "PB Pitch", juce::NormalisableRange<float>(1.0f, 12.0f, 1.0f), 2.0f));

    // Mod Wheel Toggles - removed vel_mw, at_mw, pb_mw
    params.push_back (std::make_unique<juce::AudioParameterBool> ("mw_enable", "MW Enable", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("lfo1_mw", "LFO 1 MW", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("lfo2_mw", "LFO 2 MW", false));

    return { params.begin(), params.end() };
}

float Neon37AudioProcessor::getSyncMultiplier(int syncIndex)
{
    // Convert sync index (0-10) to time multiplier
    // 0=1/64, 1=1/32, 2=1/16, 3=1/8, 4=1/4, 5=1/2, 6=1/1, 7=2/1, 8=3/1, 9=4/1, 10=8/1
    constexpr float multipliers[] = { 1.0f/64.0f, 1.0f/32.0f, 1.0f/16.0f, 1.0f/8.0f, 1.0f/4.0f, 
                                       1.0f/2.0f, 1.0f, 2.0f, 3.0f, 4.0f, 8.0f };
    if (syncIndex < 0 || syncIndex > 10)
        return 1.0f;
    return multipliers[syncIndex];
}

float Neon37AudioProcessor::generateLFOWaveform(float phase, int waveformType)
{
    // Normalize phase to 0-1
    float normPhase = phase / juce::MathConstants<float>::twoPi;
    normPhase = normPhase - std::floor(normPhase);
    
    switch (waveformType)
    {
        case 0: // Triangle
            return 4.0f * std::abs(normPhase - 0.5f) - 1.0f;
            
        case 1: // Ramp Up (Sawtooth)
            return 2.0f * normPhase - 1.0f;
            
        case 2: // Ramp Down
            return 1.0f - 2.0f * normPhase;
            
        case 3: // Square
            return normPhase < 0.5f ? 1.0f : -1.0f;
            
        case 4: // Sample & Hold
        {
            // Simple S&H: quantize to 32 steps per cycle
            int step = (int)(normPhase * 32.0f);
            // Generate deterministic pseudo-random value per step
            uint32_t seed = step * 2654435761U;
            seed = seed ^ (seed >> 16);
            seed = seed * 73856093U;
            return ((float)(seed & 0x7FFF) / 32767.5f) - 1.0f;
        }
        
        default:
            return 0.0f;
    }
}

// === REFACTORED HELPERS FOR CLEANER CODE ===

void Neon37AudioProcessor::calculateAllModulations(ModulationState& modState, float lfoFilterMod, float lfoPitchMod, float lfoAmpMod, float modWheelScale)
{
    // Convert LFO pitch modulation to semitones
    float pitchModSemitones = lfoPitchMod * 12.0f * modWheelScale;
    
    // === VELOCITY MODULATION ===
    float velPitchAmount = *apvts.getRawParameterValue("vel_pitch");
    float velFilterAmount = *apvts.getRawParameterValue("vel_filter");
    float velAmpAmount = *apvts.getRawParameterValue("vel_amp");
    
    float velPitchMod = velPitchAmount * currentVelocity;
    float velFilterMod = velFilterAmount * currentVelocity;
    float velAmpMod = velAmpAmount * currentVelocity;
    
    // === AFTERTOUCH MODULATION ===
    float atPitchAmount = *apvts.getRawParameterValue("at_pitch");
    float atFilterAmount = *apvts.getRawParameterValue("at_filter");
    float atAmpAmount = *apvts.getRawParameterValue("at_amp");
    
    float atPitchMod = atPitchAmount * currentAftertouch;
    float atFilterMod = atFilterAmount * currentAftertouch;
    float atAmpMod = atAmpAmount * currentAftertouch;
    
    // === PITCH BEND MODULATION ===
    float pbPitchAmount = *apvts.getRawParameterValue("pb_pitch");
    float pbPitchMod = pbPitchAmount * pitchBendValue;
    
    // === COMBINE ALL MODULATIONS ===
    modState.pitchModSemitones = pitchModSemitones + velPitchMod + atPitchMod + pbPitchMod;
    
    float totalFilterMod = (lfoFilterMod * modWheelScale) + velFilterMod + atFilterMod;
    modState.totalFilterModMultiplier = 1.0f + juce::jlimit(-5.0f, 5.0f, totalFilterMod);
    
    float totalAmpMod = (lfoAmpMod * modWheelScale) + velAmpMod + atAmpMod;
    modState.totalAmpModMultiplier = 1.0f + juce::jlimit(-1.0f, 1.0f, totalAmpMod);
}

float Neon37AudioProcessor::calculateModulatedCutoff(float baseCutoff, float filterEnvValue, float egDepth, float totalFilterModMultiplier, float /*resonance*/) const
{
    // Apply envelope modulation to cutoff
    float egModulation = (egDepth / 100.0f) * filterEnvValue * 10.0f;
    float modulatedCutoff = baseCutoff * std::pow(2.0f, egModulation);
    
    // Apply all filter modulations (LFO, velocity, aftertouch)
    modulatedCutoff *= totalFilterModMultiplier;
    modulatedCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);
    
    return modulatedCutoff;
}

int Neon37AudioProcessor::allocateVoice()
{
    // Find an inactive voice or steal the oldest active one
    int voiceToAllocate = -1;
    for (int i = 0; i < MAX_VOICES; ++i)
    {
        if (!voices[i].active)
        {
            voiceToAllocate = i;
            break;
        }
    }
    
    // If no inactive voice, steal the oldest one
    if (voiceToAllocate == -1)
    {
        uint64_t oldestTimestamp = voices[0].allocationTimestamp;
        voiceToAllocate = 0;
        for (int i = 1; i < MAX_VOICES; ++i)
        {
            if (voices[i].allocationTimestamp < oldestTimestamp)
            {
                oldestTimestamp = voices[i].allocationTimestamp;
                voiceToAllocate = i;
            }
        }
    }
    
    return voiceToAllocate;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Neon37AudioProcessor();
}
