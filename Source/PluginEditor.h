#pragma once

#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"

class Neon37AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    Neon37AudioProcessorEditor (Neon37AudioProcessor&);
    ~Neon37AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void showParameterTooltip(const juce::String& text, juce::Component* source);
    void hideParameterTooltip();
    void showErrorDialog(const juce::String& errorMessage);
    void showInfoDialog(const juce::String& message);
    void showSaveDialog();
    void showLoadDialog();

private:
    Neon37AudioProcessor& audioProcessor;
    CustomLookAndFeel customLookAndFeel;
    juce::TooltipWindow tooltipWindow{this, 700};
    
    // Patch Management Section
    juce::Component patchManagementSection;
    juce::TextEditor patchNameBox;
    juce::TextButton savePatchBtn{"SAVE"};
    juce::TextButton loadPatchBtn{"LOAD"};
    juce::TextButton newPatchBtn{"NEW"};
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    // Parameter value tooltip display
    juce::Label parameterValueTooltip;
    juce::Component* lastActiveKnob = nullptr;
    
    class TooltipTimer : public juce::Timer
    {
    public:
        std::function<void()> onTimeout;
        void timerCallback() override {
            if (onTimeout) onTimeout();
            stopTimer();
        }
    };
    TooltipTimer tooltipTimer;

    struct Knob : public juce::Component
    {
        juce::Slider slider;
        juce::Label label;
        bool isWaveKnob = false;
        juce::RangedAudioParameter* parameter = nullptr;
        Neon37AudioProcessorEditor* editor = nullptr;

        Knob(juce::String name, bool isWave = false) : isWaveKnob(isWave) {
            addAndMakeVisible(slider);
            slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            
            slider.onValueChange = [this] {
                if (parameter && editor) {
                    editor->showParameterTooltip(parameter->getCurrentValueAsText(), this);
                }
            };

            addAndMakeVisible(label);
            label.setText(name, juce::dontSendNotification);
            label.setFont(juce::Font(13.0f, juce::Font::bold));
            label.setJustificationType(juce::Justification::centred);
            label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.9f));
        }

        void setParameter(juce::RangedAudioParameter* p, Neon37AudioProcessorEditor* ed) {
            parameter = p;
            editor = ed;
        }

        void paint(juce::Graphics& g) override {
            if (isWaveKnob) {
                auto b = getLocalBounds().withTrimmedBottom(20).reduced(5);
                float centerX = (float)b.getCentreX();
                float centerY = (float)b.getCentreY();
                float r = juce::jmin((float)b.getWidth(), (float)b.getHeight()) / 2.0f + 12.0f;
                
                g.setColour(juce::Colours::white.withAlpha(0.7f));
                
                auto drawIcon = [&](int index, float angle) {
                    float x = centerX + r * std::sin(angle);
                    float y = centerY - r * std::cos(angle);
                    float size = 8.0f;
                    
                    if (index == 0) { // Sine
                        g.drawHorizontalLine((int)y, x - size, x + size);
                    } else if (index == 1) { // Triangle
                        juce::Path p;
                        p.startNewSubPath(x - size, y + size/2);
                        p.lineTo(x, y - size/2);
                        p.lineTo(x + size, y + size/2);
                        g.strokePath(p, juce::PathStrokeType(1.5f));
                    } else if (index == 2) { // Saw
                        juce::Path p;
                        p.startNewSubPath(x - size, y + size/2);
                        p.lineTo(x + size, y - size/2);
                        p.lineTo(x + size, y + size/2);
                        g.strokePath(p, juce::PathStrokeType(1.5f));
                    } else if (index == 3) { // Square
                        g.drawRect(x - size, y - size/2, size * 2, size, 1.5f);
                    } else if (index == 4) { // Pulse
                        g.drawRect(x - size/2, y - size/2, size, size, 1.5f);
                    }
                };

                float startAngle = -juce::MathConstants<float>::pi * 0.75f;
                float endAngle = juce::MathConstants<float>::pi * 0.75f;
                for (int i = 0; i < 5; ++i) {
                    float angle = startAngle + (float)i * (endAngle - startAngle) / 4.0f;
                    drawIcon(i, angle);
                }
            }
        }

        void resized() override {
            auto b = getLocalBounds();
            label.setBounds(b.removeFromBottom(20));
            slider.setBounds(b.reduced(2));
        }
    };

    struct Section : public juce::Component
    {
        juce::String name;
        Section(juce::String n) : name(n) {}
        void paint(juce::Graphics& g) override {
            auto b = getLocalBounds();
            // Subsequent 37 style panel - darker blue-gray with texture
            g.setColour(juce::Colour(0xFF1E2A32));
            g.fillRoundedRectangle(b.toFloat(), 4.0f);
            
            // Subtle border for panel separation
            g.setColour(juce::Colour(0xFF0A0F14));
            g.drawRoundedRectangle(b.toFloat(), 4.0f, 1.5f);
            
            // Subtle highlight on top edge (hardware look)
            g.setColour(juce::Colours::white.withAlpha(0.05f));
            g.drawLine((float)b.getX(), (float)b.getY() + 1.0f, (float)b.getRight(), (float)b.getY() + 1.0f, 1.0f);

            auto header = b.removeFromTop(30);
            // White labels instead of teal for Moog style
            g.setColour(juce::Colours::white.withAlpha(0.9f));
            g.setFont(juce::Font(14.0f, juce::Font::bold));
            g.drawText(name, header.reduced(12, 0), juce::Justification::left);
            
            g.setColour(juce::Colours::white.withAlpha(0.1f));
            g.drawLine((float)header.getX() + 12.0f, (float)header.getBottom(), (float)header.getRight() - 12.0f, (float)header.getBottom(), 1.0f);

            // Draw specific labels based on section name
            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.setFont(juce::Font(10.0f, juce::Font::bold));
            if (name == "OSCILLATOR") {
                g.drawText("16' 8' 4' 2'", 70, 140, 100, 15, juce::Justification::centred);
                g.drawText("16' 8' 4' 2'", 70, 480, 100, 15, juce::Justification::centred);
            }
            else if (name == "OSCILLATOR") {
                // Draw separator line between Osc 1 and Osc 2
                auto separator = getLocalBounds().getCentreY();
                g.setColour(juce::Colours::white.withAlpha(0.15f));
                g.drawLine((float)b.getX() + 15.0f, (float)separator, (float)b.getRight() - 15.0f, (float)separator, 1.5f);
            }
            else if (name == "ENVELOPES") {
                g.drawText("M-SEC .1", 10, 115, 60, 15, juce::Justification::left);
                g.drawText("10 SEC", 160, 115, 60, 15, juce::Justification::right);
                g.drawText("M-SEC .1", 10, 235, 60, 15, juce::Justification::left);
                g.drawText("10 SEC", 160, 235, 60, 15, juce::Justification::right);
            }
        }
    };

    struct Logo : public juce::Component
    {
        void paint(juce::Graphics& g) override {
            // Subsequent 37 style branding
            g.setColour(juce::Colours::white.withAlpha(0.95f));
            g.setFont(juce::Font(28.0f, juce::Font::bold));
            g.drawText("NEON", getLocalBounds().removeFromTop(35), juce::Justification::centred);
            g.setFont(juce::Font(24.0f, juce::Font::bold));
            g.drawText("37", getLocalBounds().withTrimmedTop(35).removeFromTop(30), juce::Justification::centred);
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.setFont(juce::Font(11.0f));
            g.drawText("PARAPHONIC ANALOG SYNTHESIZER", getLocalBounds().withTrimmedTop(68), juce::Justification::centred);
        }
    };

    struct LED : public juce::Component
    {
        bool on = false;
        juce::Colour color = juce::Colours::red;
        void paint(juce::Graphics& g) override {
            g.setColour(on ? color : color.darker().withAlpha(0.2f));
            g.fillEllipse(getLocalBounds().reduced(1).toFloat());
            if (on) {
                g.setColour(juce::Colours::white.withAlpha(0.5f));
                g.fillEllipse(getLocalBounds().getX() + 2.0f, getLocalBounds().getY() + 2.0f, 3.0f, 3.0f);
            }
        }
    };

    struct RoutingSelector : public juce::Component
    {
        LED leds[3];
        juce::Label labels[3];
        int selectedIndex = 1; // Default to 1+2
        std::function<void(int)> onClick;

        RoutingSelector() {
            juce::String names[] = {"1", "1+2", "2"};
            for (int i = 0; i < 3; ++i) {
                addAndMakeVisible(leds[i]);
                leds[i].setInterceptsMouseClicks(false, false); // Pass clicks to parent
                leds[i].color = juce::Colours::orangered; // Moog-style orange/red LEDs
                addAndMakeVisible(labels[i]);
                labels[i].setInterceptsMouseClicks(false, false); // Pass clicks to parent
                labels[i].setText(names[i], juce::dontSendNotification);
                labels[i].setFont(juce::Font(11.0f, juce::Font::bold));
                labels[i].setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
            }
            updateLeds();
        }

        void updateLeds() {
            for (int i = 0; i < 3; ++i) leds[i].on = (i == selectedIndex);
        }

        void mouseDown(const juce::MouseEvent& e) override {
            juce::ignoreUnused(e);
            selectedIndex = (selectedIndex + 1) % 3;
            updateLeds();
            repaint();
            if (onClick) onClick(selectedIndex);
        }

        void resized() override {
            auto area = getLocalBounds();
            auto h = area.getHeight() / 3;
            for (int i = 0; i < 3; ++i) {
                auto row = area.removeFromTop(h);
                leds[i].setBounds(row.removeFromLeft(12).withSizeKeepingCentre(8, 8));
                labels[i].setBounds(row);
            }
        }
    };

    struct SmallButton : public juce::ToggleButton
    {
        SmallButton(juce::String name) : juce::ToggleButton(name) {
            setColour(juce::ToggleButton::tickColourId, juce::Colours::orangered);
            setColour(juce::ToggleButton::textColourId, juce::Colours::white);
        }

        void paint(juce::Graphics& g) override {
            auto b = getLocalBounds().reduced(2).toFloat();
            bool isToggled = getToggleState();

            // Moog-style button - darker when off, orange/red glow when on
            g.setColour(isToggled ? juce::Colours::orangered.withAlpha(0.4f) : juce::Colour(0xFF0A0A0A));
            g.fillRoundedRectangle(b, 2.0f);

            g.setColour(isToggled ? juce::Colours::orangered : juce::Colours::white.withAlpha(0.3f));
            g.drawRoundedRectangle(b, 2.0f, 1.0f);

            g.setColour(isToggled ? juce::Colours::white : juce::Colours::white.withAlpha(0.7f));
            g.setFont(juce::Font(10.0f, juce::Font::bold));
            g.drawText(getButtonText(), b, juce::Justification::centred);
        }
    };

    // Sidebar Buttons
    SmallButton holdBtn{"HOLD"}, monoLBtn{"MONO-L"}, monoBtn{"MONO"}, paraLBtn{"PARA-L"}, paraBtn{"PARA"}, polyBtn{"POLY"};
    
    // Oscillator Buttons

    
    // Envelope Buttons

    // Gliss Buttons
    SmallButton glissRteBtn{"RT"}, glissTmeBtn{"TM"}, glissLogBtn{"LG"}, glissOnGatLegBtn{"ON GAT LEG"};

    struct LockIcon : public juce::Component {
        void paint(juce::Graphics& g) override {
            auto b = getLocalBounds().reduced(2).toFloat();
            g.setColour(juce::Colours::grey);
            g.drawRoundedRectangle(b.withTrimmedTop(b.getHeight()*0.4f), 1.0f, 1.0f);
            g.drawHorizontalLine((int)b.getCentreY(), b.getX() + 2, b.getRight() - 2);
        }
    } tuneLock;

    Logo logo;

    Section oscillatorAndMixerSection{"OSCILLATOR & MIXER"};
    Section oscillatorSection{"OSCILLATOR"};
    struct Slider : public juce::Component
    {
        juce::Slider slider;
        juce::Label label;

        Slider(juce::String name, bool horizontal = true) {
            addAndMakeVisible(slider);
            slider.setSliderStyle(horizontal ? juce::Slider::LinearHorizontal : juce::Slider::LinearVertical);
            slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            
            addAndMakeVisible(label);
            label.setText(name, juce::dontSendNotification);
            label.setFont(juce::Font(13.0f, juce::Font::bold));
            label.setJustificationType(juce::Justification::centred);
            label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.9f));
        }

        void resized() override {
            auto b = getLocalBounds();
            label.setBounds(b.removeFromBottom(20));
            slider.setBounds(b.reduced(10, 5));
        }
    };

    Knob osc1Wave{"WAVE", true}, osc1Octave{"OCTAVE"}, osc1Semitones{"SEMITONES"}, osc1Fine{"FINE"};
    Knob osc2Wave{"WAVE", true}, osc2Octave{"OCTAVE"}, osc2Semitones{"SEMITONES"}, osc2Fine{"FINE"};
    SmallButton oscSyncBtn{"SYNC"}; // New Sync Button

    Section mixerSection{"MIXER"};
    Knob mixOsc1{"OSC 1"}, mixSub1{"SUB 1"}, mixOsc2{"OSC 2"}, mixNoise{"NOISE"};

    Section filterSection{"FILTER"};
    Knob filterCutoff{"CUTOFF"}, filterRes{"RESONANCE"}, filterDrive{"DRIVE"}, filterEgDepth{"EG DEPTH"}, filterKeyTrk{"KEY TRK"};

    Section env1Section{"FILTER ENV"};
    Knob fltA{"A"}, fltD{"D"}, fltS{"S"}, fltR{"R"};

    Section env2Section{"AMP ENV"};
    Knob ampA{"A"}, ampD{"D"}, ampS{"S"}, ampR{"R"};

    Section pitchEnvSection{"PITCH ENV"};
    Knob pitchA{"A"}, pitchD{"D"}, pitchS{"S"}, pitchR{"R"}, pitchDepth{"DEPTH"};
    RoutingSelector pitchTargetSelector;

    Section mod1Section{"LFO"};
    Section modBottomSection{"MODULATION"};

    Knob lfo1Rate{"RATE"}, lfo1Wave{"WAVE", true};
    Knob lfo1Pitch{"PITCH"}, lfo1Filter{"FILTER"}, lfo1Amp{"AMP"};
    SmallButton lfo1SyncBtn{"SYNC"};

    Knob lfo2Rate{"RATE"}, lfo2Wave{"WAVE", true};
    Knob lfo2Pitch{"PITCH"}, lfo2Filter{"FILTER"}, lfo2Amp{"AMP"};
    SmallButton lfo2SyncBtn{"SYNC"};

    Knob velPitch{"PITCH"}, velFilter{"FILTER"}, velAmp{"AMP"};
    Knob atPitch{"PITCH"}, atFilter{"FILTER"}, atAmp{"AMP"};
    Knob mwPitch{"PITCH"}, mwFilter{"FILTER"}, mwAmp{"AMP"};
    Knob pbPitch{"PITCH"}, pbFilter{"FILTER"}, pbAmp{"AMP"};
    
    SmallButton lfo1MwBtn{"MW"}, lfo2MwBtn{"MW"}, velMwBtn{"MW"}, atMwBtn{"MW"}, mwMwBtn{"MW"}, pbMwBtn{"MW"};
    
    juce::Label lfo1Label{"", "LFO 1"}, lfo2Label{"", "LFO 2"}, velLabel{"", "VELOCITY"}, atLabel{"", "AFTERTOUCH"}, mwLabel{"", "MOD WHEEL"}, pbLabel{"", "PITCH BEND"};
    
    Section masterSection{"MASTER"};
    juce::Label modeLabel{};  // For voice mode buttons
    Knob glissTime{"GLIDE TIME"};
    Knob masterVolume{"VOLUME"}, masterFreq{"MASTER FREQ"}, masterTune{"MASTER TUNE"};

    // APVTS Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> masterVolAttach, masterFreqAttach, masterTuneAttach;
    std::unique_ptr<SliderAttachment> osc1WaveAttach, osc1OctaveAttach, osc1SemitonesAttach, osc1FineAttach;
    std::unique_ptr<SliderAttachment> osc2WaveAttach, osc2OctaveAttach, osc2SemitonesAttach, osc2FineAttach;
    std::unique_ptr<ButtonAttachment> oscSyncAttach; // Attachment for Sync
    std::unique_ptr<SliderAttachment> mixOsc1Attach, mixSub1Attach, mixOsc2Attach, mixNoiseAttach;
    std::unique_ptr<SliderAttachment> cutoffAttach, resAttach, driveAttach, egDepthAttach, keyTrkAttach;
    std::unique_ptr<SliderAttachment> fltAAttach, fltDAttach, fltSAttach, fltRAttach;
    std::unique_ptr<SliderAttachment> ampAAttach, ampDAttach, ampSAttach, ampRAttach;
    std::unique_ptr<SliderAttachment> pitchAAttach, pitchDAttach, pitchSAttach, pitchRAttach, pitchDepthAttach;
    
    // Custom attachment for RoutingSelector (Pitch Target)
    struct PitchTargetAttachment : public juce::AudioProcessorValueTreeState::Listener
    {
        Neon37AudioProcessorEditor& editor;
        PitchTargetAttachment(Neon37AudioProcessorEditor& e) : editor(e) {
            editor.audioProcessor.apvts.addParameterListener("env_pitch_target", this);
            parameterChanged("env_pitch_target", *editor.audioProcessor.apvts.getRawParameterValue("env_pitch_target"));
        }
        ~PitchTargetAttachment() {
            editor.audioProcessor.apvts.removeParameterListener("env_pitch_target", this);
        }
        void parameterChanged(const juce::String& parameterID, float newValue) override {
            if (parameterID == "env_pitch_target") {
                int target = (int)newValue;
                juce::MessageManager::callAsync([this, target] {
                    editor.pitchTargetSelector.selectedIndex = target;
                    editor.pitchTargetSelector.updateLeds();
                    editor.pitchTargetSelector.repaint();
                });
            }
        }
    };
    std::unique_ptr<PitchTargetAttachment> pitchTargetAttach;

    std::unique_ptr<SliderAttachment> glissTimeAttach;
    std::unique_ptr<ButtonAttachment> glissRteAttach, glissTmeAttach, glissLogAttach, glissOnGatLegAttach;
    std::unique_ptr<ButtonAttachment> envExpCurvAttach, holdAttach;
    
    struct VoiceModeAttachment : public juce::AudioProcessorValueTreeState::Listener
    {
        Neon37AudioProcessorEditor& editor;
        VoiceModeAttachment(Neon37AudioProcessorEditor& e) : editor(e) {
            editor.audioProcessor.apvts.addParameterListener("voice_mode", this);
            parameterChanged("voice_mode", *editor.audioProcessor.apvts.getRawParameterValue("voice_mode"));
        }
        ~VoiceModeAttachment() {
            editor.audioProcessor.apvts.removeParameterListener("voice_mode", this);
        }
        void parameterChanged(const juce::String& parameterID, float newValue) override {
            if (parameterID == "voice_mode") {
                int mode = (int)newValue;
                juce::MessageManager::callAsync([this, mode] {
                    editor.monoLBtn.setToggleState(mode == 0, juce::dontSendNotification);
                    editor.monoBtn.setToggleState(mode == 1, juce::dontSendNotification);
                    editor.paraLBtn.setToggleState(mode == 2, juce::dontSendNotification);
                    editor.paraBtn.setToggleState(mode == 3, juce::dontSendNotification);
                    editor.polyBtn.setToggleState(mode == 4, juce::dontSendNotification);
                });
            }
        }
    };
    std::unique_ptr<VoiceModeAttachment> voiceModeAttach;

    std::unique_ptr<SliderAttachment> lfo1RateAttach, lfo1WaveAttach, lfo1PitchAttach, lfo1FilterAttach, lfo1AmpAttach;
    std::unique_ptr<ButtonAttachment> lfo1SyncAttach;
    std::unique_ptr<SliderAttachment> lfo2RateAttach, lfo2WaveAttach, lfo2PitchAttach, lfo2FilterAttach, lfo2AmpAttach;
    std::unique_ptr<ButtonAttachment> lfo2SyncAttach;

    std::unique_ptr<SliderAttachment> velPitchAttach, velFilterAttach, velAmpAttach;
    std::unique_ptr<SliderAttachment> atPitchAttach, atFilterAttach, atAmpAttach;
    std::unique_ptr<SliderAttachment> mwPitchAttach, mwFilterAttach, mwAmpAttach;
    std::unique_ptr<SliderAttachment> pbPitchAttach, pbFilterAttach, pbAmpAttach;
    std::unique_ptr<ButtonAttachment> lfo1MwAttach, lfo2MwAttach, velMwAttach, atMwAttach, mwMwAttach, pbMwAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Neon37AudioProcessorEditor)
};
