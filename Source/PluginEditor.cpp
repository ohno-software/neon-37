#include "PluginProcessor.h"
#include "PluginEditor.h"

Neon37AudioProcessorEditor::Neon37AudioProcessorEditor (Neon37AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&customLookAndFeel);

    addAndMakeVisible(masterSection);
    masterSection.addAndMakeVisible(logo);
    
    addAndMakeVisible(oscillatorAndMixerSection);
    oscillatorAndMixerSection.addAndMakeVisible(oscillatorSection);    
    oscillatorSection.addAndMakeVisible(osc1Wave);
    oscillatorSection.addAndMakeVisible(osc1Octave);
    oscillatorSection.addAndMakeVisible(osc1Semitones);
    oscillatorSection.addAndMakeVisible(osc1Fine);
    oscillatorSection.addAndMakeVisible(osc2Wave);
    oscillatorSection.addAndMakeVisible(oscSyncBtn); // Should be visible in oscillator section
    oscillatorSection.addAndMakeVisible(osc2Octave);
    oscillatorSection.addAndMakeVisible(osc2Semitones);
    oscillatorSection.addAndMakeVisible(osc2Fine);
    oscillatorAndMixerSection.addAndMakeVisible(mixerSection);
    mixerSection.addAndMakeVisible(mixOsc1);
    mixerSection.addAndMakeVisible(mixSub1);
    mixerSection.addAndMakeVisible(mixOsc2);
    mixerSection.addAndMakeVisible(mixNoise);

    addAndMakeVisible(filterSection);
    filterSection.addAndMakeVisible(filterCutoff);
    filterSection.addAndMakeVisible(filterRes);
    filterSection.addAndMakeVisible(filterDrive);
    // filterEgDepth moved to env1Section
    filterSection.addAndMakeVisible(filterKeyTrk);
    
    addAndMakeVisible(pitchEnvSection);
    pitchEnvSection.addAndMakeVisible(pitchTargetSelector);
    pitchEnvSection.addAndMakeVisible(pitchDepth);
    pitchEnvSection.addAndMakeVisible(pitchA);
    pitchEnvSection.addAndMakeVisible(pitchD);
    pitchEnvSection.addAndMakeVisible(pitchS);
    pitchEnvSection.addAndMakeVisible(pitchR);

    addAndMakeVisible(env1Section);
    env1Section.addAndMakeVisible(filterEgDepth); // Moved here
    env1Section.addAndMakeVisible(fltA);
    env1Section.addAndMakeVisible(fltD);
    env1Section.addAndMakeVisible(fltS);
    env1Section.addAndMakeVisible(fltR);

    addAndMakeVisible(env2Section);
    env2Section.addAndMakeVisible(ampA);
    env2Section.addAndMakeVisible(ampD);
    env2Section.addAndMakeVisible(ampS);
    env2Section.addAndMakeVisible(ampR);

    addAndMakeVisible(mod1Section);
    mod1Section.addAndMakeVisible(lfo1Rate);
    mod1Section.addAndMakeVisible(lfo1Wave);
    mod1Section.addAndMakeVisible(lfo1SyncBtn);
    mod1Section.addAndMakeVisible(lfo2Rate);
    mod1Section.addAndMakeVisible(lfo2Wave);
    mod1Section.addAndMakeVisible(lfo2SyncBtn);


    addAndMakeVisible(modBottomSection);
    modBottomSection.addAndMakeVisible(lfo1Label);
    modBottomSection.addAndMakeVisible(lfo1Pitch);
    modBottomSection.addAndMakeVisible(lfo1Filter);
    modBottomSection.addAndMakeVisible(lfo1Amp);
    modBottomSection.addAndMakeVisible(lfo1MwBtn);

    modBottomSection.addAndMakeVisible(lfo2Label);
    modBottomSection.addAndMakeVisible(lfo2Pitch);
    modBottomSection.addAndMakeVisible(lfo2Filter);
    modBottomSection.addAndMakeVisible(lfo2Amp);
    modBottomSection.addAndMakeVisible(lfo2MwBtn);

    modBottomSection.addAndMakeVisible(velLabel);
    modBottomSection.addAndMakeVisible(velPitch);
    modBottomSection.addAndMakeVisible(velFilter);
    modBottomSection.addAndMakeVisible(velAmp);
    modBottomSection.addAndMakeVisible(atLabel);
    modBottomSection.addAndMakeVisible(atPitch);
    modBottomSection.addAndMakeVisible(atFilter);
    modBottomSection.addAndMakeVisible(atAmp);

    modBottomSection.addAndMakeVisible(mwLabel);
    modBottomSection.addAndMakeVisible(mwPitch);
    modBottomSection.addAndMakeVisible(mwFilter);
    modBottomSection.addAndMakeVisible(mwAmp);

    modBottomSection.addAndMakeVisible(pbLabel);
    modBottomSection.addAndMakeVisible(pbPitch);
    modBottomSection.addAndMakeVisible(pbFilter);
    modBottomSection.addAndMakeVisible(pbAmp);
    
    masterSection.addAndMakeVisible(masterVolume);
    masterSection.addAndMakeVisible(masterFreq);
    masterSection.addAndMakeVisible(masterTune);
    masterSection.addAndMakeVisible(glissTime);
    masterSection.addAndMakeVisible(glissRteBtn);
    masterSection.addAndMakeVisible(glissTmeBtn);
    masterSection.addAndMakeVisible(glissLogBtn);
    masterSection.addAndMakeVisible(glissOnGatLegBtn);
    masterSection.addAndMakeVisible(holdBtn);
    
    masterSection.addAndMakeVisible(modeLabel);
    modeLabel.setText("MODE", juce::dontSendNotification);
    modeLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    modeLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    modeLabel.setJustificationType(juce::Justification::centred);
    
    masterSection.addAndMakeVisible(monoLBtn);
    masterSection.addAndMakeVisible(monoBtn);
    masterSection.addAndMakeVisible(paraLBtn);
    masterSection.addAndMakeVisible(paraBtn);
    masterSection.addAndMakeVisible(polyBtn);

    // Initialize APVTS Attachments
    auto& apvts = audioProcessor.apvts;
    
    auto setupKnob = [&](Knob& knob, juce::String paramID) {
        knob.setParameter(dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(paramID)), this);
    };

    masterVolAttach = std::make_unique<SliderAttachment>(apvts, "master_volume", masterVolume.slider);
    setupKnob(masterVolume, "master_volume");
    masterFreqAttach = std::make_unique<SliderAttachment>(apvts, "master_freq", masterFreq.slider);
    setupKnob(masterFreq, "master_freq");
    masterTuneAttach = std::make_unique<SliderAttachment>(apvts, "master_tune", masterTune.slider);
    setupKnob(masterTune, "master_tune");

    osc1WaveAttach = std::make_unique<SliderAttachment>(apvts, "osc1_wave", osc1Wave.slider);
    setupKnob(osc1Wave, "osc1_wave");
    osc1OctaveAttach = std::make_unique<SliderAttachment>(apvts, "osc1_octave", osc1Octave.slider);
    setupKnob(osc1Octave, "osc1_octave");
    osc1SemitonesAttach = std::make_unique<SliderAttachment>(apvts, "osc1_semitones", osc1Semitones.slider);
    setupKnob(osc1Semitones, "osc1_semitones");
    osc1FineAttach = std::make_unique<SliderAttachment>(apvts, "osc1_fine", osc1Fine.slider);
    setupKnob(osc1Fine, "osc1_fine");
    osc2WaveAttach = std::make_unique<SliderAttachment>(apvts, "osc2_wave", osc2Wave.slider);
    setupKnob(osc2Wave, "osc2_wave");
    osc2OctaveAttach = std::make_unique<SliderAttachment>(apvts, "osc2_octave", osc2Octave.slider);
    setupKnob(osc2Octave, "osc2_octave");
    osc2SemitonesAttach = std::make_unique<SliderAttachment>(apvts, "osc2_semitones", osc2Semitones.slider);
    setupKnob(osc2Semitones, "osc2_semitones");
    osc2FineAttach = std::make_unique<SliderAttachment>(apvts, "osc2_fine", osc2Fine.slider);
    setupKnob(osc2Fine, "osc2_fine");
    
    // Existing hard_sync parameter is already defined in PluginProcessor.cpp L346 under Global Osc
    // We can use it, or create a new one if it was intended differently. 
    // Looking at read_file output which I didn't see earlier, I should check parameter name.
    // L346: params.push_back (std::make_unique<juce::AudioParameterBool> ("hard_sync", "Hard Sync", false));
    oscSyncAttach = std::make_unique<ButtonAttachment>(apvts, "hard_sync", oscSyncBtn);

    mixOsc1Attach = std::make_unique<SliderAttachment>(apvts, "mixer_osc1", mixOsc1.slider);
    setupKnob(mixOsc1, "mixer_osc1");
    mixSub1Attach = std::make_unique<SliderAttachment>(apvts, "mixer_sub1", mixSub1.slider);
    setupKnob(mixSub1, "mixer_sub1");
    mixOsc2Attach = std::make_unique<SliderAttachment>(apvts, "mixer_osc2", mixOsc2.slider);
    setupKnob(mixOsc2, "mixer_osc2");
    mixNoiseAttach = std::make_unique<SliderAttachment>(apvts, "mixer_noise", mixNoise.slider);
    setupKnob(mixNoise, "mixer_noise");

    cutoffAttach = std::make_unique<SliderAttachment>(apvts, "cutoff", filterCutoff.slider);
    setupKnob(filterCutoff, "cutoff");
    resAttach = std::make_unique<SliderAttachment>(apvts, "resonance", filterRes.slider);
    setupKnob(filterRes, "resonance");
    driveAttach = std::make_unique<SliderAttachment>(apvts, "drive", filterDrive.slider);
    setupKnob(filterDrive, "drive");
    egDepthAttach = std::make_unique<SliderAttachment>(apvts, "eg_depth", filterEgDepth.slider);
    setupKnob(filterEgDepth, "eg_depth");
    keyTrkAttach = std::make_unique<SliderAttachment>(apvts, "key_track", filterKeyTrk.slider);
    setupKnob(filterKeyTrk, "key_track");

    pitchTargetSelector.onClick = [this](int index) {
        auto* param = audioProcessor.apvts.getParameter("env_pitch_target");
        if (param)
        {
             // AudioParameterChoice takes normalized 0-1 range or index?
             // getParameter returns RangedAudioParameter. setValueNotifyingHost expects 0-1.
             // We need to convert index to 0-1.
             // AudioParameterChoice normalizes index / (numChoices - 1)
             // Choices are 3: 0, 1, 2. Max is 2.
             float value = (float)index / 2.0f;
             param->setValueNotifyingHost(value);
        }
    };

    pitchTargetAttach = std::make_unique<PitchTargetAttachment>(*this);
    pitchDepthAttach = std::make_unique<SliderAttachment>(apvts, "env_pitch_depth", pitchDepth.slider);
    setupKnob(pitchDepth, "env_pitch_depth");
    pitchAAttach = std::make_unique<SliderAttachment>(apvts, "env_pitch_attack", pitchA.slider);
    setupKnob(pitchA, "env_pitch_attack");
    pitchDAttach = std::make_unique<SliderAttachment>(apvts, "env_pitch_decay", pitchD.slider);
    setupKnob(pitchD, "env_pitch_decay");
    pitchSAttach = std::make_unique<SliderAttachment>(apvts, "env_pitch_sustain", pitchS.slider);
    setupKnob(pitchS, "env_pitch_sustain");
    pitchRAttach = std::make_unique<SliderAttachment>(apvts, "env_pitch_release", pitchR.slider);
    setupKnob(pitchR, "env_pitch_release");

    fltAAttach = std::make_unique<SliderAttachment>(apvts, "env1_attack", fltA.slider);
    setupKnob(fltA, "env1_attack");
    fltDAttach = std::make_unique<SliderAttachment>(apvts, "env1_decay", fltD.slider);
    setupKnob(fltD, "env1_decay");
    fltSAttach = std::make_unique<SliderAttachment>(apvts, "env1_sustain", fltS.slider);
    setupKnob(fltS, "env1_sustain");
    fltRAttach = std::make_unique<SliderAttachment>(apvts, "env1_release", fltR.slider);
    setupKnob(fltR, "env1_release");

    ampAAttach = std::make_unique<SliderAttachment>(apvts, "env2_attack", ampA.slider);
    setupKnob(ampA, "env2_attack");
    ampDAttach = std::make_unique<SliderAttachment>(apvts, "env2_decay", ampD.slider);
    setupKnob(ampD, "env2_decay");
    ampSAttach = std::make_unique<SliderAttachment>(apvts, "env2_sustain", ampS.slider);
    setupKnob(ampS, "env2_sustain");
    ampRAttach = std::make_unique<SliderAttachment>(apvts, "env2_release", ampR.slider);
    setupKnob(ampR, "env2_release");
    
    glissTimeAttach = std::make_unique<SliderAttachment>(apvts, "gliss_time", glissTime.slider);
    setupKnob(glissTime, "gliss_time");
    glissRteAttach = std::make_unique<ButtonAttachment>(apvts, "gliss_rte", glissRteBtn);
    glissTmeAttach = std::make_unique<ButtonAttachment>(apvts, "gliss_tme", glissTmeBtn);
    glissLogAttach = std::make_unique<ButtonAttachment>(apvts, "gliss_log", glissLogBtn);
    glissOnGatLegAttach = std::make_unique<ButtonAttachment>(apvts, "gliss_on_gat_leg", glissOnGatLegBtn);

    holdAttach = std::make_unique<ButtonAttachment>(apvts, "hold_mode", holdBtn);

    auto setVoiceMode = [this](int mode) {
        auto* param = audioProcessor.apvts.getParameter("voice_mode");
        if (!param) return;
        
        float normalizedValue = param->getNormalisableRange().convertTo0to1((float)mode);
        param->setValueNotifyingHost(normalizedValue);
    };

    monoLBtn.onClick = [this, setVoiceMode] { 
        float currentMode = *audioProcessor.apvts.getRawParameterValue("voice_mode");
        if ((int)currentMode == 0) { monoLBtn.setToggleState(true, juce::dontSendNotification); return; }
        setVoiceMode(0); 
    };
    monoBtn.onClick = [this, setVoiceMode] { 
        float currentMode = *audioProcessor.apvts.getRawParameterValue("voice_mode");
        if ((int)currentMode == 1) { monoBtn.setToggleState(true, juce::dontSendNotification); return; }
        setVoiceMode(1); 
    };
    paraLBtn.onClick = [this, setVoiceMode] { 
        float currentMode = *audioProcessor.apvts.getRawParameterValue("voice_mode");
        if ((int)currentMode == 2) { paraLBtn.setToggleState(true, juce::dontSendNotification); return; }
        setVoiceMode(2); 
    };
    paraBtn.onClick = [this, setVoiceMode] { 
        float currentMode = *audioProcessor.apvts.getRawParameterValue("voice_mode");
        if ((int)currentMode == 3) { paraBtn.setToggleState(true, juce::dontSendNotification); return; }
        setVoiceMode(3); 
    };
    polyBtn.onClick = [this, setVoiceMode] { 
        float currentMode = *audioProcessor.apvts.getRawParameterValue("voice_mode");
        if ((int)currentMode == 4) { polyBtn.setToggleState(true, juce::dontSendNotification); return; }
        setVoiceMode(4); 
    };

    voiceModeAttach = std::make_unique<VoiceModeAttachment>(*this);

    // LFO 1 Attachments
    lfo1RateAttach = std::make_unique<SliderAttachment>(apvts, "lfo1_rate", lfo1Rate.slider);
    setupKnob(lfo1Rate, "lfo1_rate");
    lfo1WaveAttach = std::make_unique<SliderAttachment>(apvts, "lfo1_wave", lfo1Wave.slider);
    setupKnob(lfo1Wave, "lfo1_wave");
    lfo1SyncAttach = std::make_unique<ButtonAttachment>(apvts, "lfo1_sync", lfo1SyncBtn);

    // LFO 2 Attachments
    lfo2RateAttach = std::make_unique<SliderAttachment>(apvts, "lfo2_rate", lfo2Rate.slider);
    setupKnob(lfo2Rate, "lfo2_rate");
    lfo2WaveAttach = std::make_unique<SliderAttachment>(apvts, "lfo2_wave", lfo2Wave.slider);
    setupKnob(lfo2Wave, "lfo2_wave");
    lfo2SyncAttach = std::make_unique<ButtonAttachment>(apvts, "lfo2_sync", lfo2SyncBtn);

    lfo1PitchAttach = std::make_unique<SliderAttachment>(apvts, "lfo1_pitch", lfo1Pitch.slider);
    setupKnob(lfo1Pitch, "lfo1_pitch");
    lfo1FilterAttach = std::make_unique<SliderAttachment>(apvts, "lfo1_filter", lfo1Filter.slider);
    setupKnob(lfo1Filter, "lfo1_filter");
    lfo1AmpAttach = std::make_unique<SliderAttachment>(apvts, "lfo1_amp", lfo1Amp.slider);
    setupKnob(lfo1Amp, "lfo1_amp");

    lfo2PitchAttach = std::make_unique<SliderAttachment>(apvts, "lfo2_pitch", lfo2Pitch.slider);
    setupKnob(lfo2Pitch, "lfo2_pitch");
    lfo2FilterAttach = std::make_unique<SliderAttachment>(apvts, "lfo2_filter", lfo2Filter.slider);
    setupKnob(lfo2Filter, "lfo2_filter");
    lfo2AmpAttach = std::make_unique<SliderAttachment>(apvts, "lfo2_amp", lfo2Amp.slider);
    setupKnob(lfo2Amp, "lfo2_amp");

    velPitchAttach = std::make_unique<SliderAttachment>(apvts, "vel_pitch", velPitch.slider);
    setupKnob(velPitch, "vel_pitch");
    velFilterAttach = std::make_unique<SliderAttachment>(apvts, "vel_filter", velFilter.slider);
    setupKnob(velFilter, "vel_filter");
    velAmpAttach = std::make_unique<SliderAttachment>(apvts, "vel_amp", velAmp.slider);
    setupKnob(velAmp, "vel_amp");

    atPitchAttach = std::make_unique<SliderAttachment>(apvts, "at_pitch", atPitch.slider);
    setupKnob(atPitch, "at_pitch");
    atFilterAttach = std::make_unique<SliderAttachment>(apvts, "at_filter", atFilter.slider);
    setupKnob(atFilter, "at_filter");
    atAmpAttach = std::make_unique<SliderAttachment>(apvts, "at_amp", atAmp.slider);
    setupKnob(atAmp, "at_amp");

    mwPitchAttach = std::make_unique<SliderAttachment>(apvts, "mw_pitch", mwPitch.slider);
    setupKnob(mwPitch, "mw_pitch");
    mwFilterAttach = std::make_unique<SliderAttachment>(apvts, "mw_filter", mwFilter.slider);
    setupKnob(mwFilter, "mw_filter");
    mwAmpAttach = std::make_unique<SliderAttachment>(apvts, "mw_amp", mwAmp.slider);
    setupKnob(mwAmp, "mw_amp");

    pbPitchAttach = std::make_unique<SliderAttachment>(apvts, "pb_pitch", pbPitch.slider);
    setupKnob(pbPitch, "pb_pitch");
    pbFilterAttach = std::make_unique<SliderAttachment>(apvts, "pb_filter", pbFilter.slider);
    setupKnob(pbFilter, "pb_filter");
    pbAmpAttach = std::make_unique<SliderAttachment>(apvts, "pb_amp", pbAmp.slider);
    setupKnob(pbAmp, "pb_amp");

    lfo1MwAttach = std::make_unique<ButtonAttachment>(apvts, "mw_enable", lfo1MwBtn);  // Global MW Enable
    lfo2MwAttach = std::make_unique<ButtonAttachment>(apvts, "lfo2_mw", lfo2MwBtn);
    
    // Setup parameter value tooltip
    addAndMakeVisible(parameterValueTooltip);
    parameterValueTooltip.setFont(juce::Font(16.0f, juce::Font::bold));
    parameterValueTooltip.setJustificationType(juce::Justification::centred);
    parameterValueTooltip.setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.8f));
    parameterValueTooltip.setColour(juce::Label::textColourId, juce::Colours::white);
    parameterValueTooltip.setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.3f));
    parameterValueTooltip.setVisible(false);
    
    tooltipTimer.onTimeout = [this] { hideParameterTooltip(); };

    // Setup Patch Management Section
    addAndMakeVisible(patchManagementSection);
    
    patchManagementSection.addAndMakeVisible(patchNameBox);
    patchNameBox.setFont(juce::Font(32.0f, juce::Font::bold));
    patchNameBox.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    patchNameBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xFF2B3A45));
    patchNameBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xFF00FFFF).withAlpha(0.6f));
    patchNameBox.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xFF00FFFF));
    patchNameBox.setBorder(juce::BorderSize<int>(2));
    patchNameBox.setInputRestrictions(256, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -_()[].");
    patchNameBox.setReturnKeyStartsNewLine(false);
    patchNameBox.setText("   Untitled Patch");
    patchNameBox.applyFontToAllText(juce::Font(32.0f, juce::Font::bold));
    
    patchManagementSection.addAndMakeVisible(savePatchBtn);
    savePatchBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF00FFFF).withAlpha(0.3f));
    savePatchBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF00FFFF));
    savePatchBtn.setButtonText("SAVE");
    savePatchBtn.onClick = [this] { showSaveDialog(); };
    
    patchManagementSection.addAndMakeVisible(loadPatchBtn);
    loadPatchBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF00FFFF).withAlpha(0.3f));
    loadPatchBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF00FFFF));
    loadPatchBtn.setButtonText("LOAD");
    loadPatchBtn.onClick = [this] { showLoadDialog(); }; 
    
    patchManagementSection.addAndMakeVisible(newPatchBtn);
    newPatchBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF00FFFF).withAlpha(0.3f));
    newPatchBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xFF00FFFF));
    newPatchBtn.setButtonText("NEW");
    newPatchBtn.onClick = [this] { 
        DBG("New button clicked");
        audioProcessor.resetToDefaults();
        patchNameBox.setText("   Untitled Patch");
        patchNameBox.applyFontToAllText(juce::Font(32.0f, juce::Font::bold));
    };

    setSize (1300, 850);
}

Neon37AudioProcessorEditor::~Neon37AudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void Neon37AudioProcessorEditor::paint (juce::Graphics& g)
{
    // Subsequent 37 style background - blue-gray charcoal
    g.fillAll (juce::Colour(0xFF2B3A45));
}

void Neon37AudioProcessorEditor::showParameterTooltip(const juce::String& text, juce::Component* source)
{
    // If switching to a different parameter, immediately hide the old tooltip
    if (lastActiveKnob != source && lastActiveKnob != nullptr)
    {
        hideParameterTooltip();
    }
    
    lastActiveKnob = source;
    parameterValueTooltip.setText(text, juce::dontSendNotification);
    parameterValueTooltip.setVisible(true);
    parameterValueTooltip.toFront(false);
    
    // Restart the 3-second timer
    tooltipTimer.stopTimer();
    tooltipTimer.startTimer(3000);
}

void Neon37AudioProcessorEditor::hideParameterTooltip()
{
    parameterValueTooltip.setVisible(false);
    lastActiveKnob = nullptr;
    tooltipTimer.stopTimer();
}

void Neon37AudioProcessorEditor::showErrorDialog(const juce::String& errorMessage)
{
    // Show error dialog with plain text that can be copied
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                          "Error",
                                          errorMessage,
                                          "OK",
                                          this);
}

void Neon37AudioProcessorEditor::showInfoDialog(const juce::String& message)
{
    // Show info dialog with plain text that can be copied
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                          "Success",
                                          message,
                                          "OK",
                                          this);
}

void Neon37AudioProcessorEditor::showSaveDialog()
{
    juce::File presetsDir = audioProcessor.getPresetsDirectory();
    
    fileChooser = std::make_unique<juce::FileChooser>("Save Patch", presetsDir, "*.xml");
    
    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc) {
            juce::File targetFile = fc.getResult();
            if (targetFile != juce::File())
            {
                if (!targetFile.getFileExtension().equalsIgnoreCase(".xml"))
                    targetFile = targetFile.withFileExtension(".xml");
                
                if (audioProcessor.savePresetToFile(targetFile))
                {
                    juce::String displayName = targetFile.getFileNameWithoutExtension();
                    patchNameBox.setText("   " + displayName);
                    patchNameBox.applyFontToAllText(juce::Font(32.0f, juce::Font::bold));
                    showInfoDialog("Patch saved: " + displayName);
                }
                else
                {
                    showErrorDialog("Failed to save patch");
                }
            }
        });
}

void Neon37AudioProcessorEditor::showLoadDialog()
{
    juce::File presetsDir = audioProcessor.getPresetsDirectory();
    
    fileChooser = std::make_unique<juce::FileChooser>("Load Patch", presetsDir, "*.xml");
    
    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc) {
            if (fc.getResult().existsAsFile())
            {
                juce::File selectedFile = fc.getResult();
                if (audioProcessor.loadPresetFromFile(selectedFile))
                {
                    juce::String patchName = selectedFile.getFileNameWithoutExtension();
                    patchNameBox.setText("   " + patchName);
                    patchNameBox.applyFontToAllText(juce::Font(32.0f, juce::Font::bold));
                    showInfoDialog("Patch loaded: " + patchName);
                }
                else
                {
                    showErrorDialog("Failed to load patch");
                }
            }
        });
}

void Neon37AudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // Patch Management Section - Full width at top
    auto patchArea = area.removeFromTop(90);
    patchManagementSection.setBounds(patchArea);
    
    auto patchContent = patchArea.reduced(10);
    auto patchNameArea = patchContent.removeFromLeft(patchContent.getWidth() - 280);
    patchNameBox.setBounds(patchNameArea);
    
    auto buttonArea = patchContent;
    savePatchBtn.setBounds(buttonArea.removeFromLeft(85).reduced(2));
    loadPatchBtn.setBounds(buttonArea.removeFromLeft(85).reduced(2));
    newPatchBtn.setBounds(buttonArea.removeFromLeft(85).reduced(2));
    
    // Remaining area for main UI (shifted down by 90 pixels)
    area.reduce(15, 15);
    
    // Master section - wider for better readability
    auto masterArea = area.removeFromLeft(130);
    masterSection.setBounds(masterArea.reduced(4));
    
    auto sideContent = masterSection.getLocalBounds().withTrimmedTop(40).reduced(15);
    masterVolume.setBounds(sideContent.removeFromTop(90));
    masterFreq.setBounds(sideContent.removeFromTop(90));
    
    auto tuneArea = sideContent.removeFromTop(90);
    masterTune.setBounds(tuneArea);
    
    // Gliss controls in master section
    auto glissArea = sideContent.removeFromTop(100);
    glissTime.setBounds(glissArea.removeFromTop(80));
    auto glissBtns = glissArea;
    glissRteBtn.setBounds(glissBtns.removeFromLeft(glissBtns.getWidth() / 4).reduced(2));
    glissTmeBtn.setBounds(glissBtns.removeFromLeft(glissBtns.getWidth() / 3).reduced(2));
    glissLogBtn.setBounds(glissBtns.removeFromLeft(glissBtns.getWidth() / 2).reduced(2));
    glissOnGatLegBtn.setBounds(glissBtns.reduced(2));
    
    auto sideBtns = sideContent.removeFromTop(170);
    holdBtn.setBounds(sideBtns.removeFromTop(24).reduced(5, 2));
    modeLabel.setBounds(sideBtns.removeFromTop(18).reduced(5, 2));
    monoLBtn.setBounds(sideBtns.removeFromTop(24).reduced(5, 2));
    monoBtn.setBounds(sideBtns.removeFromTop(24).reduced(5, 2));
    paraLBtn.setBounds(sideBtns.removeFromTop(24).reduced(5, 2));
    paraBtn.setBounds(sideBtns.removeFromTop(24).reduced(5, 2));
    polyBtn.setBounds(sideBtns.removeFromTop(24).reduced(5, 2));
    
    logo.setBounds(masterSection.getLocalBounds().removeFromBottom(100));
    
    auto mainArea = area;
    
    // Main area for OSC, MIXER, FILTER, ENVELOPE
    oscillatorAndMixerSection.setBounds(mainArea.removeFromLeft(300).reduced(4));
    auto oscMixArea = oscillatorAndMixerSection.getLocalBounds();
    oscillatorSection.setBounds(oscMixArea.removeFromTop(oscMixArea.getHeight() / 2));
    mixerSection.setBounds(oscMixArea);  // Now contains the bottom half after removeFromTop
    
    // Filter column split: Filter on top, LFO on bottom
    auto filterCol = mainArea.removeFromLeft(220);
    filterSection.setBounds(filterCol.removeFromTop(filterCol.getHeight() / 2).reduced(4));
    mod1Section.setBounds(filterCol.reduced(4));

    // Layout LFOs in their new home
    auto lfoArea = mod1Section.getLocalBounds().withTrimmedTop(35).reduced(10);
    auto lfoRowH = lfoArea.getHeight() / 2;
    
    auto lfo1Row = lfoArea.removeFromTop(lfoRowH);
    lfo1Rate.setBounds(lfo1Row.removeFromLeft(70));
    lfo1SyncBtn.setBounds(lfo1Row.removeFromLeft(50).withSizeKeepingCentre(40, 20));
    lfo1Wave.setBounds(lfo1Row.removeFromLeft(70));
    
    auto lfo2Row = lfoArea;
    lfo2Rate.setBounds(lfo2Row.removeFromLeft(70));
    lfo2SyncBtn.setBounds(lfo2Row.removeFromLeft(50).withSizeKeepingCentre(40, 20));
    lfo2Wave.setBounds(lfo2Row.removeFromLeft(70));
    
    // Layout knobs within sections - split oscillator section in half
    auto oscArea = oscillatorSection.getLocalBounds().withTrimmedTop(40).reduced(15);
    
    // Top half for Osc 1
    auto osc1Area = oscArea.removeFromTop(oscArea.getHeight() / 2);
    osc1Area.removeFromBottom(8); // Space for separator
    auto oscRow1 = osc1Area.withSizeKeepingCentre(osc1Area.getWidth(), 90);
    osc1Wave.setBounds(oscRow1.removeFromLeft(70));
    osc1Octave.setBounds(oscRow1.removeFromLeft(70));
    osc1Semitones.setBounds(oscRow1.removeFromLeft(70));
    osc1Fine.setBounds(oscRow1.removeFromLeft(70));
    
    // Bottom half for Osc 2
    auto osc2Area = oscArea;
    osc2Area.removeFromTop(8); // Space for separator
    auto oscRow2 = osc2Area.withSizeKeepingCentre(osc2Area.getWidth(), 90);
    // Sneak Sync switch between Wave and Octave
    osc2Wave.setBounds(oscRow2.removeFromLeft(70));
    oscSyncBtn.setBounds(oscRow2.removeFromLeft(30).withSizeKeepingCentre(30, 20)); // Sneaky Sync
    osc2Octave.setBounds(oscRow2.removeFromLeft(55)); // Reduced from 70
    osc2Semitones.setBounds(oscRow2.removeFromLeft(70));
    osc2Fine.setBounds(oscRow2.removeFromLeft(70));

    auto mixArea = mixerSection.getLocalBounds().withTrimmedTop(40).reduced(15);
    auto mixRowH = 90;  // Height for each row
    
    // Row 1: Osc1 and Osc2 side by side
    auto row1 = mixArea.removeFromTop(mixRowH);
    mixOsc1.setBounds(row1.removeFromLeft(row1.getWidth() / 2));
    mixOsc2.setBounds(row1);
    
    // Row 2: Sub and Noise side by side
    auto row2 = mixArea.removeFromTop(mixRowH);
    mixSub1.setBounds(row2.removeFromLeft(row2.getWidth() / 2));
    mixNoise.setBounds(row2);

    auto filterArea = filterSection.getLocalBounds().withTrimmedTop(40).reduced(15);
    filterCutoff.setBounds(filterArea.removeFromTop(90));
    auto filterRow2 = filterArea.removeFromTop(90);
    filterRes.setBounds(filterRow2.removeFromLeft(filterRow2.getWidth() / 2));
    filterDrive.setBounds(filterRow2);
    auto filterRow3 = filterArea.removeFromTop(90);
    // filterEgDepth removed from here
    filterKeyTrk.setBounds(filterRow3.removeFromLeft(filterRow3.getWidth() / 2));

    // Pitch ENV Section
    pitchEnvSection.setBounds(mainArea.removeFromLeft(110).reduced(4));
    auto pitchEnvArea = pitchEnvSection.getLocalBounds().withTrimmedTop(40).reduced(15);
    pitchTargetSelector.setBounds(pitchEnvArea.removeFromTop(50).reduced(5));
    pitchDepth.setBounds(pitchEnvArea.removeFromTop(90));
    pitchA.setBounds(pitchEnvArea.removeFromTop(90));
    pitchD.setBounds(pitchEnvArea.removeFromTop(90));
    pitchS.setBounds(pitchEnvArea.removeFromTop(90));
    pitchR.setBounds(pitchEnvArea.removeFromTop(90));

    // ENV1 section
    env1Section.setBounds(mainArea.removeFromLeft(110).reduced(4));
    auto env1Area = env1Section.getLocalBounds().withTrimmedTop(40).reduced(15);
    filterEgDepth.setBounds(env1Area.removeFromTop(90)); // Filter Env Depth moved here
    fltA.setBounds(env1Area.removeFromTop(90));
    fltD.setBounds(env1Area.removeFromTop(90));
    fltS.setBounds(env1Area.removeFromTop(90));
    fltR.setBounds(env1Area.removeFromTop(90));
    
    // ENV2 section
    env2Section.setBounds(mainArea.removeFromLeft(110).reduced(4));
    auto env2Area = env2Section.getLocalBounds().withTrimmedTop(40).reduced(15);
    ampA.setBounds(env2Area.removeFromTop(90));
    ampD.setBounds(env2Area.removeFromTop(90));
    ampS.setBounds(env2Area.removeFromTop(90));
    ampR.setBounds(env2Area.removeFromTop(90));
    
    // Modulation section - now next to ENV2
    modBottomSection.setBounds(mainArea.removeFromLeft(270).reduced(4));
    auto modBArea = modBottomSection.getLocalBounds().withTrimmedTop(35).reduced(10);
    auto layoutModRow = [&](juce::Rectangle<int> row, juce::Label& label, juce::Component* input, Knob& k1, Knob& k2, Knob& k3, SmallButton& mwBtn) {
        label.setBounds(row.removeFromTop(20));
        auto controls = row;
        if (input) input->setBounds(controls.removeFromLeft(50).withSizeKeepingCentre(45, 20));
        else {
            auto inputArea = controls.removeFromLeft(50);
            mwBtn.setBounds(inputArea.withSizeKeepingCentre(40, 20));
        }
        
        auto knobW = controls.getWidth() / 3;
        k1.setBounds(controls.removeFromLeft(knobW));
        k2.setBounds(controls.removeFromLeft(knobW));
        k3.setBounds(controls.removeFromLeft(knobW));
    };

    auto rowH = modBArea.getHeight() / 6;  // Now 6 rows: LFO1, LFO2, Vel, AT, MW, PB
    layoutModRow(modBArea.removeFromTop(rowH), lfo1Label, nullptr, lfo1Pitch, lfo1Filter, lfo1Amp, lfo1MwBtn);
    layoutModRow(modBArea.removeFromTop(rowH), lfo2Label, nullptr, lfo2Pitch, lfo2Filter, lfo2Amp, lfo2MwBtn);
    
    // Velocity row - compact
    {
        auto row = modBArea.removeFromTop(rowH);
        velLabel.setBounds(row.removeFromTop(15));
        auto knobW = row.getWidth() / 3;
        velPitch.setBounds(row.removeFromLeft(knobW));
        velFilter.setBounds(row.removeFromLeft(knobW));
        velAmp.setBounds(row.removeFromLeft(knobW));
    }
    
    // Aftertouch row - compact
    {
        auto row = modBArea.removeFromTop(rowH);
        atLabel.setBounds(row.removeFromTop(15));
        auto knobW = row.getWidth() / 3;
        atPitch.setBounds(row.removeFromLeft(knobW));
        atFilter.setBounds(row.removeFromLeft(knobW));
        atAmp.setBounds(row.removeFromLeft(knobW));
    }
    
    // Mod Wheel row - compact
    {
        auto row = modBArea.removeFromTop(rowH);
        mwLabel.setBounds(row.removeFromTop(15));
        auto knobW = row.getWidth() / 3;
        mwPitch.setBounds(row.removeFromLeft(knobW));
        mwFilter.setBounds(row.removeFromLeft(knobW));
        mwAmp.setBounds(row.removeFromLeft(knobW));
    }
    
    // Pitch Bend row - 3 knobs (Pitch, Filter, Amp)
    {
        auto row = modBArea.removeFromTop(rowH);
        pbLabel.setBounds(row.removeFromTop(15));
        auto knobW = row.getWidth() / 3;
        pbPitch.setBounds(row.removeFromLeft(knobW));
        pbFilter.setBounds(row.removeFromLeft(knobW));
        pbAmp.setBounds(row.removeFromLeft(knobW));
    }
    
    // Position parameter value tooltip at the top center of the window
    parameterValueTooltip.setBounds(getWidth() / 2 - 150, 10, 300, 35);
}
