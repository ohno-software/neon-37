# Neon37 Documentation Audit Report
Date: 2026-01-10

## Executive Summary
The current documentation (`Neon37_Specification.md`) significantly lags behind the codebase (`Source/PluginProcessor.cpp`, `Source/PluginEditor.cpp`). The codebase implements a more advanced synthesizer (Neon37) than the one described in the specification (Subsequent 37 recreation).

## Discrepancies Found

### 1. Project Naming
- **Spec:** "Subsequent 37 Synthesizer"
- **Code:** "Neon37-r" (internally), "Neon37" (project name)

### 2. Polyphony & Voice Modes
- **Spec:** Paraphonic (1-2 voices).
- **Code:** Full Polyphony supported.
  - Modes: Mono-L, Mono, Para-L, Para, Poly.
  - Voice count: 8 voices (`MAX_VOICES = 8`).
  - Implementation: Per-voice filters and envelopes in poly mode.

### 3. Oscillators
- **Spec:** Waveforms: Sine, Triangle, Sawtooth, Square, Pulse.
- **Code:** Waveforms: Sine, Triangle, Sawtooth, Square, 25% Pulse, 10% Pulse.
- **Missing in Spec:**
  - Fine tuning parameters (±50 cents).
  - Semitone offset (±12).
  - Octave range details (-3 to +3).

### 4. Modulation Matrix
- **Spec:** Mentions modulation but lacks detail on the extensive matrix.
- **Code:** 
  - Modulation Sources: LFO 1, LFO 2, Velocity, Aftertouch, Mod Wheel, Pitch Bend.
  - Modulation Targets: Pitch, Filter, Amp (per source).
  - Global Mod Wheel Enable toggles.

### 5. Arpeggiator
- **Spec:** Not mentioned.
- **Code:** Implemented (`arp_on` parameter, though logic might be basic or partially implemented, the parameter exists).

### 6. Filter
- **Spec:** Drive 1-10 (implied standard).
- **Code:** Drive range 1.0 to 25.0.

### 7. LFO
- **Spec:** "Triangle, Square, Sawtooth, Ramp, S&H".
- **Code:** Triangle, Ramp Up, Ramp Down, Square, S&H. 
  - Note: "Sawtooth" is ambiguous in Spec, Code has both Ramps.

## Recommendations
1.  Rename `Neon37_Specification.md` to `Legacy_Subsequent37_Specification.md` or archive it.
2.  Create a new `Neon37_Reference_Manual.md` reflecting the actual Polyphonic/Paraphonic hybrid engine.
3.  Update parameters list to match `createParameterLayout` exactly.
