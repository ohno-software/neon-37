#include "PluginProcessor.h"
#include "PluginEditor.h"

Neon37AudioProcessorEditor::Neon37AudioProcessorEditor (Neon37AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&customLookAndFeel);

    addAndMakeVisible(masterSection);
    masterSection.addAndMakeVisible(logo);
    
    addAndMakeVisible(oscillatorSection);
    oscillatorSection.addAndMakeVisible(osc1Wave);
    oscillatorSection.addAndMakeVisible(osc1Octave);
    oscillatorSection.addAndMakeVisible(osc1Semitones);
    oscillatorSection.addAndMakeVisible(osc1Fine);
    oscillatorSection.addAndMakeVisible(osc2Wave);
    oscillatorSection.addAndMakeVisible(osc2Octave);
    oscillatorSection.addAndMakeVisible(osc2Semitones);
    oscillatorSection.addAndMakeVisible(osc2Fine);

    addAndMakeVisible(mixerSection);
    mixerSection.addAndMakeVisible(mixOsc1);
    mixerSection.addAndMakeVisible(mixSub1);
    mixerSection.addAndMakeVisible(mixOsc2);
    mixerSection.addAndMakeVisible(mixNoise);
    mixerSection.addAndMakeVisible(mixReturn);

    addAndMakeVisible(filterSection);
    filterSection.addAndMakeVisible(filterCutoff);
    filterSection.addAndMakeVisible(filterRes);
    filterSection.addAndMakeVisible(filterDrive);
    filterSection.addAndMakeVisible(filterEgDepth);
    filterSection.addAndMakeVisible(filterKeyTrk);

    addAndMakeVisible(envelopeSection);
    
    envelopeSection.addAndMakeVisible(env1Label);
    env1Label.setText("ENV 1 - FILTER/MOD", juce::dontSendNotification);
    env1Label.setFont(juce::Font(11.0f, juce::Font::bold));
    env1Label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    env1Label.setJustificationType(juce::Justification::centred);
    
    envelopeSection.addAndMakeVisible(env2Label);
    env2Label.setText("ENV 2 - AMP", juce::dontSendNotification);
    env2Label.setFont(juce::Font(11.0f, juce::Font::bold));
    env2Label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    env2Label.setJustificationType(juce::Justification::centred);
    
    envelopeSection.addAndMakeVisible(fltA);
    envelopeSection.addAndMakeVisible(fltD);
    envelopeSection.addAndMakeVisible(fltS);
    envelopeSection.addAndMakeVisible(fltR);
    envelopeSection.addAndMakeVisible(ampA);
    envelopeSection.addAndMakeVisible(ampD);
    envelopeSection.addAndMakeVisible(ampS);
    envelopeSection.addAndMakeVisible(ampR);

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
    modBottomSection.addAndMakeVisible(velMwBtn);
    
    modBottomSection.addAndMakeVisible(atLabel);
    modBottomSection.addAndMakeVisible(atPitch);
    modBottomSection.addAndMakeVisible(atFilter);
    modBottomSection.addAndMakeVisible(atAmp);
    modBottomSection.addAndMakeVisible(atMwBtn);

    modBottomSection.addAndMakeVisible(pbLabel);
    modBottomSection.addAndMakeVisible(pbPitch);
    modBottomSection.addAndMakeVisible(pbFilter);
    modBottomSection.addAndMakeVisible(pbAmp);
    modBottomSection.addAndMakeVisible(pbMwBtn);
    
    masterSection.addAndMakeVisible(masterVolume);
    masterSection.addAndMakeVisible(masterFreq);
    masterSection.addAndMakeVisible(masterTune);
    masterSection.addAndMakeVisible(glissTime);
    masterSection.addAndMakeVisible(glissRteBtn);
    masterSection.addAndMakeVisible(glissTmeBtn);
    masterSection.addAndMakeVisible(glissLogBtn);
    masterSection.addAndMakeVisible(glissOnGatLegBtn);
    masterSection.addAndMakeVisible(holdBtn);
    masterSection.addAndMakeVisible(monoLBtn);
    masterSection.addAndMakeVisible(monoBtn);
    masterSection.addAndMakeVisible(paraLBtn);
    masterSection.addAndMakeVisible(paraBtn);
    masterSection.addAndMakeVisible(polyBtn);

    oscillatorSection.addAndMakeVisible(hardSyncBtn);
    oscillatorSection.addAndMakeVisible(keySyncBtn);

    envelopeSection.addAndMakeVisible(fltRetrigBtn);
    envelopeSection.addAndMakeVisible(fltResetBtn);
    envelopeSection.addAndMakeVisible(fltSyncBtn);
    envelopeSection.addAndMakeVisible(fltCycleBtn);
    envelopeSection.addAndMakeVisible(ampRetrigBtn);
    envelopeSection.addAndMakeVisible(ampResetBtn);
    envelopeSection.addAndMakeVisible(ampSyncBtn);
    envelopeSection.addAndMakeVisible(ampCycleBtn);
    envelopeSection.addAndMakeVisible(envHoldOnBtn);
    envelopeSection.addAndMakeVisible(envExpCurvBtn);

    mixerSection.addAndMakeVisible(osc1Led);
    mixerSection.addAndMakeVisible(osc2Led);
    osc1Led.on = true;

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

    mixOsc1Attach = std::make_unique<SliderAttachment>(apvts, "mixer_osc1", mixOsc1.slider);
    setupKnob(mixOsc1, "mixer_osc1");
    mixSub1Attach = std::make_unique<SliderAttachment>(apvts, "mixer_sub1", mixSub1.slider);
    setupKnob(mixSub1, "mixer_sub1");
    mixOsc2Attach = std::make_unique<SliderAttachment>(apvts, "mixer_osc2", mixOsc2.slider);
    setupKnob(mixOsc2, "mixer_osc2");
    mixNoiseAttach = std::make_unique<SliderAttachment>(apvts, "mixer_noise", mixNoise.slider);
    setupKnob(mixNoise, "mixer_noise");
    mixReturnAttach = std::make_unique<SliderAttachment>(apvts, "mixer_return", mixReturn.slider);
    setupKnob(mixReturn, "mixer_return");

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

    hardSyncAttach = std::make_unique<ButtonAttachment>(apvts, "hard_sync", hardSyncBtn);
    envExpCurvAttach = std::make_unique<ButtonAttachment>(apvts, "env_exp_curv", envExpCurvBtn);
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

    pbPitchAttach = std::make_unique<SliderAttachment>(apvts, "pb_pitch", pbPitch.slider);
    setupKnob(pbPitch, "pb_pitch");
    pbFilterAttach = std::make_unique<SliderAttachment>(apvts, "pb_filter", pbFilter.slider);
    setupKnob(pbFilter, "pb_filter");
    pbAmpAttach = std::make_unique<SliderAttachment>(apvts, "pb_amp", pbAmp.slider);
    setupKnob(pbAmp, "pb_amp");

    lfo1MwAttach = std::make_unique<ButtonAttachment>(apvts, "lfo1_mw", lfo1MwBtn);
    lfo2MwAttach = std::make_unique<ButtonAttachment>(apvts, "lfo2_mw", lfo2MwBtn);
    velMwAttach = std::make_unique<ButtonAttachment>(apvts, "vel_mw", velMwBtn);
    atMwAttach = std::make_unique<ButtonAttachment>(apvts, "at_mw", atMwBtn);
    pbMwAttach = std::make_unique<ButtonAttachment>(apvts, "pb_mw", pbMwBtn);
    
    // Setup parameter value tooltip
    addAndMakeVisible(parameterValueTooltip);
    parameterValueTooltip.setFont(juce::Font(16.0f, juce::Font::bold));
    parameterValueTooltip.setJustificationType(juce::Justification::centred);
    parameterValueTooltip.setColour(juce::Label::backgroundColourId, juce::Colours::black.withAlpha(0.8f));
    parameterValueTooltip.setColour(juce::Label::textColourId, juce::Colours::white);
    parameterValueTooltip.setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.3f));
    parameterValueTooltip.setVisible(false);
    
    tooltipTimer.onTimeout = [this] { hideParameterTooltip(); };

    setSize (1300, 750);
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

void Neon37AudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(15);
    
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
    
    auto sideBtns = sideContent.removeFromTop(160);
    holdBtn.setBounds(sideBtns.removeFromTop(24).reduced(5, 2));
    monoLBtn.setBounds(sideBtns.removeFromTop(24).reduced(5, 2));
    monoBtn.setBounds(sideBtns.removeFromTop(24).reduced(5, 2));
    paraLBtn.setBounds(sideBtns.removeFromTop(24).reduced(5, 2));
    paraBtn.setBounds(sideBtns.removeFromTop(24).reduced(5, 2));
    polyBtn.setBounds(sideBtns.removeFromTop(24).reduced(5, 2));
    
    logo.setBounds(masterSection.getLocalBounds().removeFromBottom(100));
    
    auto mainArea = area;
    
    // Right side MOD section - now full height for bigger knobs
    auto rightArea = mainArea.removeFromRight(280);
    modBottomSection.setBounds(rightArea.reduced(4));

    // Main area for OSC, MIXER, FILTER, ENVELOPE
    oscillatorSection.setBounds(mainArea.removeFromLeft(300).reduced(4));
    mixerSection.setBounds(mainArea.removeFromLeft(130).reduced(4));
    
    // Filter column split: Filter on top, LFO on bottom
    auto filterCol = mainArea.removeFromLeft(220);
    filterSection.setBounds(filterCol.removeFromTop(filterCol.getHeight() / 2).reduced(4));
    mod1Section.setBounds(filterCol.reduced(4));
    
    envelopeSection.setBounds(mainArea.reduced(4));

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

    // Layout Mod Bottom (Routing) - now with much more vertical space
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

    auto rowH = modBArea.getHeight() / 5;
    layoutModRow(modBArea.removeFromTop(rowH), lfo1Label, nullptr, lfo1Pitch, lfo1Filter, lfo1Amp, lfo1MwBtn);
    layoutModRow(modBArea.removeFromTop(rowH), lfo2Label, nullptr, lfo2Pitch, lfo2Filter, lfo2Amp, lfo2MwBtn);
    layoutModRow(modBArea.removeFromTop(rowH), velLabel, nullptr, velPitch, velFilter, velAmp, velMwBtn);
    layoutModRow(modBArea.removeFromTop(rowH), atLabel, nullptr, atPitch, atFilter, atAmp, atMwBtn);
    layoutModRow(modBArea.removeFromTop(rowH), pbLabel, nullptr, pbPitch, pbFilter, pbAmp, pbMwBtn);
    
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
    osc2Wave.setBounds(oscRow2.removeFromLeft(70));
    osc2Octave.setBounds(oscRow2.removeFromLeft(70));
    osc2Semitones.setBounds(oscRow2.removeFromLeft(70));
    osc2Fine.setBounds(oscRow2.removeFromLeft(70));
    
    auto oscBtns = oscArea.removeFromTop(40);
    hardSyncBtn.setBounds(oscBtns.removeFromTop(24).reduced(10, 3));
    keySyncBtn.setBounds(oscBtns.removeFromTop(24).reduced(10, 3));

    auto mixArea = mixerSection.getLocalBounds().withTrimmedTop(40).reduced(15);
    auto mixRowH = 75; // Increased from 55 for bigger knobs
    auto osc1Row = mixArea.removeFromTop(mixRowH);
    osc1Led.setBounds(osc1Row.removeFromRight(20).withSizeKeepingCentre(12, 12));
    mixOsc1.setBounds(osc1Row);
    
    mixSub1.setBounds(mixArea.removeFromTop(mixRowH));
    
    auto osc2Row = mixArea.removeFromTop(mixRowH);
    osc2Led.setBounds(osc2Row.removeFromRight(20).withSizeKeepingCentre(12, 12));
    mixOsc2.setBounds(osc2Row);
    
    mixNoise.setBounds(mixArea.removeFromTop(mixRowH));
    mixReturn.setBounds(mixArea.removeFromTop(mixRowH));

    auto filterArea = filterSection.getLocalBounds().withTrimmedTop(40).reduced(15);
    filterCutoff.setBounds(filterArea.removeFromTop(120).reduced(15, 0));
    auto filterRow2 = filterArea.removeFromTop(90);
    filterRes.setBounds(filterRow2.removeFromLeft(filterRow2.getWidth() / 2));
    filterDrive.setBounds(filterRow2);
    auto filterRow3 = filterArea.removeFromTop(90);
    filterEgDepth.setBounds(filterRow3.removeFromLeft(filterRow3.getWidth() / 2));
    filterKeyTrk.setBounds(filterRow3);

    auto envArea = envelopeSection.getLocalBounds().withTrimmedTop(40).reduced(15);
    env1Label.setBounds(envArea.removeFromTop(15));
    auto fltRowH = 75;
    auto fltADRow = envArea.removeFromTop(fltRowH);
        fltA.setBounds(fltADRow.removeFromLeft(fltADRow.getWidth() / 2));
        fltD.setBounds(fltADRow);
    auto fltSRRow = envArea.removeFromTop(fltRowH);
        fltS.setBounds(fltSRRow.removeFromLeft(fltSRRow.getWidth() / 2));
        fltR.setBounds(fltSRRow);
    
    auto fltBtns = envArea.removeFromTop(50);
    fltRetrigBtn.setBounds(fltBtns.removeFromLeft(fltBtns.getWidth() / 4).reduced(3));
    fltResetBtn.setBounds(fltBtns.removeFromLeft(fltBtns.getWidth() / 3).reduced(3));
    fltSyncBtn.setBounds(fltBtns.removeFromLeft(fltBtns.getWidth() / 2).reduced(3));
    fltCycleBtn.setBounds(fltBtns.reduced(3));

    auto envSpacerRow = envArea.removeFromTop(50);

    env2Label.setBounds(envArea.removeFromTop(15));
    auto ampRowH = 75;
    auto ampADRow = envArea.removeFromTop(ampRowH);
        ampA.setBounds(ampADRow.removeFromLeft(ampADRow.getWidth() / 2));
        ampD.setBounds(ampADRow);
    auto ampSRRow = envArea.removeFromTop(ampRowH);
        ampS.setBounds(ampSRRow.removeFromLeft(ampSRRow.getWidth() / 2));   
        ampR.setBounds(ampSRRow);
    
    auto ampBtns = envArea.removeFromTop(50);
    ampRetrigBtn.setBounds(ampBtns.removeFromLeft(ampBtns.getWidth() / 4).reduced(3));
    ampResetBtn.setBounds(ampBtns.removeFromLeft(ampBtns.getWidth() / 3).reduced(3));
    ampSyncBtn.setBounds(ampBtns.removeFromLeft(ampBtns.getWidth() / 2).reduced(3));
    ampCycleBtn.setBounds(ampBtns.reduced(3));
    
    auto envBottomBtns = envArea.removeFromTop(30);
    envHoldOnBtn.setBounds(envBottomBtns.removeFromLeft(envBottomBtns.getWidth() / 2).reduced(15, 3));
    envExpCurvBtn.setBounds(envBottomBtns.reduced(15, 3));
    
    // Position parameter value tooltip at the top center of the window
    parameterValueTooltip.setBounds(getWidth() / 2 - 150, 10, 300, 35);
}
