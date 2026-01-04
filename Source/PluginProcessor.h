#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <map>
#include <algorithm>
#include <array>

// LFO structure for global LFO modulation
struct Neon37LFO
{
    float phase = 0.0f;           // 0-1 normalized phase
    float rate = 0.1f;            // Hz (free mode) or tempo multiplier (sync mode)
    int waveform = 0;             // 0=Triangle, 1=RampUp, 2=RampDown, 3=Square, 4=SampleHold
    bool syncEnabled = false;
    float pitchAmount = 0.0f;     // 0-12 semitones
    float filterAmount = 0.0f;    // 0-1 (0-100%)
    float ampAmount = 0.0f;       // 0-1 (0-100%)
};

// Voice structure for paraphonic and poly operation
// Paraphonic: Uses shared monoFilter/monoFilterEnv/monoAmpEnv, per-voice oscillators + ampGate
// Poly: Each voice has independent filter, filterEnv, ampEnv - complete signal chain per voice
struct Neon37Voice
{
    int midiNote = -1;
    bool active = false;
    uint64_t allocationTimestamp = 0;  // Track when this voice was allocated (for stealing oldest voice)
    
    // Oscillator phase tracking (independent per voice - free-running)
    float osc1Phase = 0.0f, osc2Phase = 0.0f, subOscPhase = 0.0f;
    
    // For poly mode: Per-voice velocity and aftertouch tracking (independent per note)
    float velocity = 0.0f;  // 0-1, from MIDI note-on velocity
    float aftertouch = 0.0f;  // 0-1, from MIDI note aftertouch (per-note polyphonic aftertouch)
    
    // For paraphonic modes: Amp gate for this voice (gates the oscillators on/off)
    // Independent of the shared monoAmpEnv envelope
    juce::ADSR ampGate;
    
    // For poly mode: Per-voice filter and envelopes (complete signal chain)
    juce::dsp::LadderFilter<float> filter;
    juce::ADSR filterEnv;
    juce::ADSR ampEnv;
    
    // Buffer for rendering this voice's signal (before shared processing in para modes)
    juce::AudioBuffer<float> voiceBuffer;
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

    // Preset management (.xml files)
    bool savePresetToFile (const juce::File& file);
    bool loadPresetFromFile (const juce::File& file);
    juce::String getPresetFileExtension() const { return ".xml"; }
    juce::File getPresetsDirectory() const;
    juce::String filenameFromPatchName (const juce::String& patchName) const;
    void resetToDefaults();

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP Components - for MONO modes
    juce::dsp::LadderFilter<float> monoFilter;
    juce::dsp::Gain<float> outputGain;
    juce::ADSR monoFilterEnv;
    juce::ADSR monoAmpEnv;
    
    // Cache envelope parameters to avoid updating every block
    float cachedEnv1Attack = 0.001f, cachedEnv1Decay = 0.05f, cachedEnv1Sustain = 1.0f, cachedEnv1Release = 0.05f;
    float cachedEnv2Attack = 0.003f, cachedEnv2Decay = 0.05f, cachedEnv2Sustain = 1.0f, cachedEnv2Release = 0.05f;
    
    double currentSampleRate = 44100.0;
    
    // Oscillator phase tracking - for MONO modes
    float osc1Phase = 0.0f, osc2Phase = 0.0f, subOscPhase = 0.0f;
    
    // MIDI note tracking - for MONO modes
    int currentMidiNote = 60; // Middle C
    int heldNoteCount = 0;    // Track how many keys are currently held
    std::vector<int> noteStack;  // Stack of held notes (for proper mono note recall)
    
    // Paraphonic voices (8 voices max)
    static constexpr int MAX_VOICES = 8;
    std::array<Neon37Voice, MAX_VOICES> voices;
    uint64_t voiceAllocationCounter = 0;  // Incremented on each voice allocation to track age
    bool lastBlockHadAnyActiveVoices = false;  // Track if previous block had active voices (for envelope retrigger logic)
    
    // Global LFO modulation
    Neon37LFO lfo1;
    Neon37LFO lfo2;
    float modWheelValue = 0.0f;  // 0-1, from MIDI CC1 (defaults to 0 when enabled, forced to 1 when disabled)
    bool modWheelEnabled = false;
    
    // Velocity, Aftertouch, and Mod Wheel tracking
    float currentVelocity = 0.0f;  // 0-1, from MIDI note-on velocity
    float currentAftertouch = 0.0f;  // 0-1, from MIDI channel aftertouch (CC176)
    float pitchBendValue = 0.0f;  // -1 to +1, from MIDI pitch bend
    float modWheelValueRaw = 0.0f;  // 0-1, from MIDI CC1 (raw mod wheel value, separate from modWheelValue which is used for LFO scaling)
    
    // Helper function to generate waveform samples
    float generateWaveform(float phase, int waveformType);
    
    // Helper function to generate LFO waveforms
    float generateLFOWaveform(float phase, int waveformType);
    
    // Helper to convert sync index (0-10) to time multiplier
    float getSyncMultiplier(int syncIndex);
    
    // Refactored helper functions for cleaner processBlock
    struct ModulationState {
        float pitchModSemitones;
        float totalFilterModMultiplier;
        float totalAmpModMultiplier;
    };
    
    void calculateAllModulations(ModulationState& modState, float lfoFilterMod, float lfoPitchMod, float lfoAmpMod, float modWheelScale);
    float calculateModulatedCutoff(float baseCutoff, float filterEnvValue, float egDepth, float totalFilterModMultiplier, float resonance) const;
    int allocateVoice();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Neon37AudioProcessor)
};
