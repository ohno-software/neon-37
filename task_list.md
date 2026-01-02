# Neon-37 Implementation Task List

This document tracks the progress of the Neon-37 synthesizer UI implementation against the [Neon37_Specification.md](Neon37_Specification.md) and the reference hardware image.

## ✅ Correctly Implemented
- [x] **Basic Project Structure**: CMake configuration, PluginProcessor, and PluginEditor setup.
- [x] **Color Palette**: Dark background (#0A0A0A) with Neon Teal (#00CED1) accents.
- [x] **Sectional Layout**: All major sections (Oscillator, Mixer, Filter, Envelopes, Mod, Gliss, Arp) are defined and positioned.
- [x] **Branding**: "NEON", "NEON-37", and "NEON AUDIO" labels are present in the sidebar.
- [x] **Circular Knobs**: Updated `CustomLookAndFeel` to ensure 1:1 aspect ratio and added 3D shading/gradients.
- [x] **Knob Pointers**: Added sharp teal pointer lines with a subtle glow.
- [x] **Tick Marks**: Added scale markings around all knobs.
- [x] **Buttons/Toggles**: Added all missing buttons across all sections (HOLD, DAW, SYNC, RETRIG, etc.).
- [x] **Routing Indicators**: Implemented the `1`, `1+2`, `2` selection logic in MOD1 and MOD2.
- [x] **Arpeggiator Swing**: Replaced the knob with a horizontal slider.
- [x] **Value Labels**: Added specific values (16', 8', etc.) to Octave and Envelope controls.
- [x] **Waveform Icons**: Added Sine, Saw, and Square icons to the Oscillator `WAVE` selectors.
- [x] **Master Tune Lock**: Added the lock icon next to the Master Tune knob.
- [x] **Readability & Typography**: Increased font sizes (13pt-15pt) and used bold weights for high contrast.
- [x] **Layout & Spacing**: Expanded window to 1300x750 and added padding between elements.

## 🛠️ Needs Implementation / Refinement

### 1. Audio Engine & Parameters (High Priority)
- [ ] **Parameter Layout**: Update `createParameterLayout()` with specific ranges and defaults (Volume 0dB, Freq 440Hz, etc.).
- [ ] **Voice Architecture**: Implement `Neon37Voice` class (Oscillators, Envelopes, Filter).
- [ ] **Moog Filter**: Implement a 24dB/oct Ladder Filter.
- [ ] **Oscillators**: Implement Sine, Triangle, Saw, Square, and Pulse waveforms.
- [ ] **MIDI Handling**: Implement basic MIDI note on/off and pitch bend.

### 2. UI Connectivity (High Priority)
- [ ] **APVTS Attachments**: Connect all UI knobs and buttons to the `AudioProcessorValueTreeState`.
- [ ] **Parameter Display**: Ensure knobs show their current values in a tooltip or label.

### 3. Advanced DSP (Medium Priority)
- [ ] **Hard Sync**: Implement oscillator hard sync logic.
- [ ] **Noise & Mixer**: Implement the noise generator and mixer gain stages.
- [ ] **Modulation**: Connect LFOs and Envelopes to targets.

## 🚀 Next Steps
1.  **Define Parameters**: Update `PluginProcessor.cpp` with the full list of parameters and their default values.
2.  **Implement Voice**: Create the basic oscillator and envelope logic in the processor.
3.  **Connect UI**: Add `SliderAttachment` and `ButtonAttachment` to the `PluginEditor`.
