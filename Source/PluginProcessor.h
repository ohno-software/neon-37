#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class Neon37Sound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class Neon37Voice : public juce::SynthesiserVoice
{
public:
    Neon37Voice();
    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;
    void prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels);
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    
    void retriggerEnvelopes();
    void startEnvelopesWithoutRetrigger();

    void updateParameters (float osc1Wave, float osc1Octave, float osc1Semitones, float osc1Fine,
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
                          bool legatoMode, bool paraMode, int numActiveVoices);
    
    float getFilterEnvValue() const { return lastEnv1Value; }

    void setAftertouch(float value) { currentAftertouch = value; }
    void setModWheel(float value) { currentModWheel = value; }

private:
    juce::dsp::Oscillator<float> osc1, osc2, sub1;
    juce::dsp::Oscillator<float> noise;
    juce::dsp::Oscillator<float> lfo1, lfo2;
    juce::dsp::Gain<float> osc1Gain, osc2Gain, sub1Gain, noiseGain;
    juce::dsp::LadderFilter<float> voiceFilter; // Per-voice filter
    
    int currentOsc1Wave = 2;
    int currentOsc2Wave = 2;
    int currentLfo1Wave = 0;
    int currentLfo2Wave = 0;
    float currentPw = 0.5f;
    float currentOsc1Octave = 1.0f, currentOsc1Semitones = 0.0f, currentOsc1Fine = 0.0f;
    float currentOsc2Octave = 1.0f, currentOsc2Semitones = 0.0f, currentOsc2Fine = 0.0f;
    float currentFilterCutoff = 20000.0f;
    float currentFilterResonance = 0.0f;
    float currentFilterDrive = 1.0f;
    float currentFilterEgDepth = 0.0f;
    bool useExpEnv = true;

    // Mod parameters
    float modLfo1Pitch = 0, modLfo1Filter = 0, modLfo1Amp = 0;
    float modLfo2Pitch = 0, modLfo2Filter = 0, modLfo2Amp = 0;
    float modVelPitch = 0, modVelFilter = 0, modVelAmp = 0;
    float modAtPitch = 0, modAtFilter = 0, modAtAmp = 0;
    float modPbPitch = 2, modPbFilter = 0, modPbAmp = 0;
    bool modLfo1Mw = false, modLfo2Mw = false, modVelMw = false, modAtMw = false, modPbMw = false;
    bool currentLegatoMode = false, currentParaMode = false;
    int activeVoiceCount = 0;

    float currentVelocity = 0.0f;
    float currentAftertouch = 0.0f;
    float currentPitchBend = 0.0f; // -1.0 to 1.0
    float currentModWheel = 0.0f; // 0.0 to 1.0

    float baseOsc1Freq = 440.0f;
    float baseOsc2Freq = 440.0f;
    float baseSub1Freq = 220.0f;

    juce::ADSR env1, env2; // env1 for filter/mod, env2 for amp
    juce::ADSR::Parameters env1Params, env2Params;
    float lastEnv1Value = 0.0f;
    
    // 3ms fade-in/out to prevent clicks at note-on/off
    int fadeInSamples = 0;
    int fadeInCounter = 0;
    int fadeOutSamples = 0;
    int fadeOutCounter = -1; // -1 means not fading out
    double currentSampleRate = 44100.0;

    juce::AudioBuffer<float> synthBuffer;
    bool isPrepared = false;
};

class Neon37AudioProcessor : public juce::AudioProcessor
{
public:
    Neon37AudioProcessor();
    ~Neon37AudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::Synthesiser synth;

    juce::dsp::Gain<float> masterGain;

    // Mono mode state
    std::vector<int> heldNotes;
    int currentPlayingNote = -1;
    
    // Hold mode state
    std::vector<int> physicallyHeldNotes;
    bool lastHoldState = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Neon37AudioProcessor)
};
