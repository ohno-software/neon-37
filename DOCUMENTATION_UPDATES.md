# Neon-37 Documentation Updates

## Summary
Both the Specification and User Manual have been updated to reflect the actual implementation of Neon-37, addressing significant differences between what was originally specified and what is actually implemented.

## Key Changes

### Neon37_Specification.md Updates

1. **Title & Introduction**
   - Changed from "Subsequent 37" to "Neon-37"
   - Updated description to reflect it's a "recreation with additional features and voice modes"

2. **Oscillator Waveforms**
   - Updated to reflect actual 6 waveforms: Sine, Triangle, Sawtooth, Square, 25% Pulse, 10% Pulse
   - Added OCTAVE range (-3 to +3)
   - Added SEMITONES (±12) and FINE (±50 cents) controls

3. **Filter Section**
   - Updated DRIVE range from 1.0-10.0 to **1.0-25.0** (more aggressive options)
   - Updated RESONANCE range from 0-100% to **0.0-1.2** (enables self-oscillation)
   - Updated EG DEPTH from -5/+5 to **-100 to +100**
   - Clarified cutoff range (20Hz to 20kHz, exponential)

4. **Envelopes**
   - Updated attack range from 0.003s to **0.001s minimum**
   - Added all special envelope controls: RETRIG, RESET, SYNC, CYCLE, HOLD ON, EXP CURV
   - Clarified exponential curve behavior (enabled by default)

5. **Voice Modes - MAJOR UPDATE**
   - **Removed**: Old "Mono Architecture" and "Scanned Keyboard" description
   - **Added**: Complete description of 5 voice modes:
     - **Mono-L**: Monophonic legato, lowest-note priority, free-running oscillators
     - **Mono**: Monophonic with envelope retrigger on each note
     - **Para-L**: Paraphonic (up to 8 voices), shared filter, phase-reset per note
     - **Para**: Paraphonic, shared gate and envelopes for all voices
     - **Poly**: Full polyphonic, 8 independent voices with complete signal chains per voice
   - Removed old mermaid diagram

6. **Modulation Sections**
   - Added LFO SYNC and SYNC VALUES (tempo-sync with note divisions 1/64 to 8/1)
   - Clarified LFO waveforms: Triangle, Ramp Up, Ramp Down, Square, Sample & Hold
   - Expanded MOD WHEEL section with enable toggle
   - Added detailed VELOCITY modulation (pitch: ±12 semitones, filter/amp: ±5 / ±2 multipliers)
   - Added detailed AFTERTOUCH modulation (per-note in poly/para modes)
   - Added PITCH BEND (1-12 semitone range)

7. **Features Section**
   - Updated to emphasize 5 voice modes instead of just paraphonic
   - Added Drive/Saturation as key feature
   - Mentioned advanced waveforms (pulse variants)

---

### USER_MANUAL.md Updates

1. **Added "Voice Modes" Section**
   - New dedicated section explaining all 5 voice modes with clear use cases
   - Guidance on when to use each mode
   - Explains differences between legato and retrigger variants

2. **Updated Oscillators Section**
   - Added all 6 waveforms with descriptions
   - Explained OCTAVE range (-3 to +3) instead of fixed 16'/8'/4'/2'
   - Added SEMITONES and FINE tuning explanations
   - Clarified BEAT RATE and FREQUENCY controls

3. **Updated Filter Section**
   - Corrected CUTOFF range (20Hz to 20kHz)
   - Updated RESONANCE range (0.0 to 1.2 with self-oscillation note)
   - Updated DRIVE range (1.0 to 25.0)
   - Expanded EG DEPTH range and explanation (-100 to +100)

4. **Updated Envelopes Section**
   - Corrected time ranges (attack: 0.001s to 10s)
   - Added all special controls with explanations
   - Emphasized exponential curve behavior

5. **Updated Glide Section**
   - Expanded TIME range (0 to 10s)
   - Added explanation of glide modes (RTE/TME/LOG)

6. **Updated Modulation Sections**
   - Added complete LFO details (rate, waveforms, sync options)
   - Expanded DEPTH controls section
   - Added detailed MOD WHEEL explanation (enable, scale behavior)
   - Added detailed VELOCITY modulation with ranges
   - Added detailed AFTERTOUCH modulation
   - Added PITCH BEND information

7. **Updated Common Sounds**
   - Provided complete, accurate parameter settings
   - Included voice mode recommendations
   - Added 5 example sounds instead of generic descriptions:
     - Classic Moog Bass
     - Screaming Lead
     - Lush Pad
     - Bright Pluck
     - Vocal Sweep Lead

8. **Removed Arpeggiator Section**
   - Arpeggiator not actually implemented in the synth
   - Replaced with detailed voice mode information

9. **Expanded Troubleshooting**
   - Added specific issues related to voice modes
   - Added glide troubleshooting
   - Added velocity and aftertouch troubleshooting
   - Corrected parameter ranges for all suggestions
   - Added guidance for poly mode voice stealing

10. **Updated Advanced Tips**
    - Added filter drive character explanation
    - Added pulse width modulation info
    - Added keyboard tracking guidance
    - Reorganized for clarity

---

## Parameters That Changed Significantly

| Parameter | Old Spec | New/Actual | Notes |
|-----------|----------|-----------|-------|
| Oscillator Octave | Fixed 16'/8'/4'/2' | Range -3 to +3 | Much more flexible |
| Filter CUTOFF | 20Hz-20kHz, default ~100% | 20Hz-20kHz, default wide open | Same range, clearer |
| Filter RESONANCE | 0-100% | 0.0-1.2 | Enables self-oscillation beyond unity |
| Filter DRIVE | 1.0-10.0 | 1.0-25.0 | More aggressive options |
| EG DEPTH | -5 to +5 | -100 to +100 | Much finer control |
| Envelope Attack Min | 0.003s | 0.001s | Faster attacks possible |
| Voice Count | Max 2 (para) | Max 8 (all modes) | Huge expansion |
| Voice Modes | Not detailed | 5 distinct modes | Major feature addition |
| LFO Sync | Mentioned | Full tempo sync with divisions | Much more flexible |
| Mod Wheel | Always enabled | Toggle enable/disable | More control |

---

## Implementation vs. Specification

### What's Accurate in Both Documents Now:
- ✅ 6 oscillator waveforms (Sine, Triangle, Saw, Square, 25% Pulse, 10% Pulse)
- ✅ 2 independent LFO sections with full modulation routing
- ✅ Moog ladder filter with 24dB/oct characteristic
- ✅ 2 complete ADSR envelopes with special modes
- ✅ Sub oscillator one octave below OSC 1
- ✅ Hard sync and key sync capabilities
- ✅ Velocity, aftertouch, and pitch bend modulation
- ✅ Master tune, volume, and transpose controls

### Major Differences from Original Spec:
- 🔄 Voice modes: Expanded from 2 (mono/para) to 5 (Mono-L, Mono, Para-L, Para, Poly)
- 🔄 Polyphony: Up to 8 voices instead of "2 notes" max
- 🔄 LFO features: Full tempo sync with note divisions (not just free-running)
- 🔄 Modulation depth: Much wider ranges (-100 to +100 for some parameters)
- 🔄 Oscillator tuning: Continuous octave and semitone ranges (not fixed choices)

---

## Document Status

Both documents are now **consistent with the actual implementation** and provide accurate, practical guidance for musicians using Neon-37.

- **Neon37_Specification.md**: Technical specification
- **USER_MANUAL.md**: Musician-friendly guide with examples and tips
- **DOCUMENTATION_UPDATES.md**: This summary of changes
