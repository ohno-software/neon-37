# Neon-37 Preset Creation Guide for AI Agents

## Overview

This document guides AI agents (such as Claude Haiku) to create a comprehensive set of presets covering the General MIDI sound palette for the Neon-37 software synthesizer.

**Important**: Neon-37 has Save/Load preset functionality that handles `.xml` preset files. Each preset should be created as a separate XML file using the JUCE AudioProcessorValueTreeState format.

---

## Preset File Format

Presets are saved as XML files with the following structure:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Neon37Preset name="Preset Name" category="Category">
  <Parameters>
    <PARAM id="parameter_id" value="0.5"/>
    <!-- ... more parameters ... -->
  </Parameters>
</Neon37Preset>
```

---

## Complete Parameter Reference

### Master Section
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `master_volume` | Output volume | -60.0 to +10.0 dB | 0.0 | Exponential scale |
| `master_tune` | Master tuning | 400.0 to 480.0 Hz | 440.0 | A4 reference |

### Oscillator 1
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `osc1_wave` | Waveform selection | 0-5 (int) | 2 | 0=Sine, 1=Triangle, 2=Sawtooth, 3=Square, 4=25% Pulse, 5=10% Pulse |
| `osc1_octave` | Octave transpose | -3 to +3 (int) | 0 | Relative to 8' |
| `osc1_semitones` | Semitone transpose | -12 to +12 (int) | 0 | |
| `osc1_fine` | Fine tuning | -0.5 to +0.5 | 0.0 | In semitones (±50 cents) |

### Oscillator 2
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `osc2_wave` | Waveform selection | 0-5 (int) | 2 | Same as osc1_wave |
| `osc2_octave` | Octave transpose | -3 to +3 (int) | 0 | |
| `osc2_semitones` | Semitone transpose | -12 to +12 (int) | 0 | |
| `osc2_fine` | Fine tuning | -0.5 to +0.5 | 0.0 | |

### Global Oscillator Controls
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `osc_freq` | OSC2 frequency offset | -7.0 to +7.0 | 0.0 | Semitones relative to OSC1 |
| `osc_beat` | Beat frequency | -3.5 to +3.5 | 0.0 | Hz detuning |
| `hard_sync` | Hard sync enable | 0 or 1 | 0 | Bool: syncs OSC2 to OSC1 |

### Mixer
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `mixer_osc1` | OSC1 level | -60.0 to +10.0 dB | 0.0 | Exponential |
| `mixer_sub1` | Sub oscillator level | -60.0 to +10.0 dB | -60.0 | Square, 1 oct below OSC1 |
| `mixer_osc2` | OSC2 level | -60.0 to +10.0 dB | -60.0 | |
| `mixer_noise` | Noise level | -60.0 to +10.0 dB | -60.0 | White noise |
| `mixer_return` | External input | -60.0 to +10.0 dB | -60.0 | Not typically used |

### Filter
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `cutoff` | Filter cutoff | 20.0 to 20000.0 Hz | 20000.0 | Exponential scale |
| `resonance` | Filter resonance | 0.0 to 1.2 | 0.0 | >1.0 = self-oscillation |
| `drive` | Filter drive | 1.0 to 25.0 | 1.0 | Saturation |
| `eg_depth` | Envelope depth | -100.0 to +100.0 | 0.0 | Filter env amount |
| `key_track` | Keyboard tracking | 0.0 to 2.0 | 0.0 | Filter follows pitch |

### Envelope 1 (Filter/Mod)
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `env1_attack` | Attack time | 0.001 to 10.0 s | 0.001 | Exponential |
| `env1_decay` | Decay time | 0.003 to 10.0 s | 0.05 | Exponential |
| `env1_sustain` | Sustain level | 0.0 to 1.0 | 1.0 | Linear |
| `env1_release` | Release time | 0.003 to 10.0 s | 0.05 | Exponential |

### Envelope 2 (Amplitude)
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `env2_attack` | Attack time | 0.003 to 10.0 s | 0.003 | Exponential |
| `env2_decay` | Decay time | 0.003 to 10.0 s | 0.05 | Exponential |
| `env2_sustain` | Sustain level | 0.0 to 1.0 | 1.0 | Linear |
| `env2_release` | Release time | 0.003 to 10.0 s | 0.05 | Exponential |

### Envelope Options
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `env_exp_curv` | Exponential curves | 0 or 1 | 1 | Bool: use exp curves |

### Voice Mode
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `voice_mode` | Voice architecture | 0-4 (int) | 0 | 0=Mono-L, 1=Mono, 2=Para-L, 3=Para, 4=Poly |
| `hold_mode` | Hold notes | 0 or 1 | 0 | Bool |

### Glide/Portamento
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `gliss_time` | Glide time | 0.0 to 10.0 s | 0.0 | 0 = off |
| `gliss_rte` | Rate mode | 0 or 1 | 0 | Bool |
| `gliss_tme` | Time mode | 0 or 1 | 1 | Bool (default) |
| `gliss_log` | Log mode | 0 or 1 | 0 | Bool |
| `gliss_on_gat_leg` | Gate legato | 0 or 1 | 0 | Bool |

### LFO 1
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `lfo1_rate` | LFO speed | 0.01 to 100.0 Hz | 0.1 | Exponential |
| `lfo1_sync` | Tempo sync | 0 or 1 | 0 | Bool |
| `lfo1_sync_val` | Sync division | 0-10 (int) | 0 | 0=1/64 ... 6=1/1 ... 10=8/1 |
| `lfo1_wave` | Waveform | 0-4 (int) | 0 | 0=Triangle, 1=RampUp, 2=RampDown, 3=Square, 4=S&H |
| `lfo1_pitch` | Pitch modulation | 0.0 to 1.0 | 0.0 | |
| `lfo1_filter` | Filter modulation | 0.0 to 1.0 | 0.0 | |
| `lfo1_amp` | Amp modulation | 0.0 to 1.0 | 0.0 | |

### LFO 2
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `lfo2_rate` | LFO speed | 0.01 to 100.0 Hz | 0.1 | Exponential |
| `lfo2_sync` | Tempo sync | 0 or 1 | 0 | Bool |
| `lfo2_sync_val` | Sync division | 0-10 (int) | 0 | |
| `lfo2_wave` | Waveform | 0-4 (int) | 0 | |
| `lfo2_pitch` | Pitch modulation | 0.0 to 1.0 | 0.0 | |
| `lfo2_filter` | Filter modulation | 0.0 to 1.0 | 0.0 | |
| `lfo2_amp` | Amp modulation | 0.0 to 1.0 | 0.0 | |

### Velocity Modulation
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `vel_pitch` | Velocity → Pitch | -12.0 to +12.0 | 0.0 | Semitones |
| `vel_filter` | Velocity → Filter | -5.0 to +5.0 | 0.0 | Multiplier |
| `vel_amp` | Velocity → Amp | -2.0 to +2.0 | 0.0 | Multiplier |

### Aftertouch Modulation
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `at_pitch` | Aftertouch → Pitch | -12.0 to +12.0 | 0.0 | Semitones |
| `at_filter` | Aftertouch → Filter | -5.0 to +5.0 | 0.0 | Multiplier |
| `at_amp` | Aftertouch → Amp | -2.0 to +2.0 | 0.0 | Multiplier |

### Pitch Bend
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `pb_pitch` | Pitch bend range | 1.0 to 12.0 | 2.0 | Semitones |

### Mod Wheel
| Parameter ID | Description | Range | Default | Notes |
|-------------|-------------|-------|---------|-------|
| `mw_enable` | Mod wheel enable | 0 or 1 | 0 | Global MW on/off |
| `lfo1_mw` | LFO1 MW control | 0 or 1 | 0 | |
| `lfo2_mw` | LFO2 MW control | 0 or 1 | 0 | |

---

## Preset Categories & Target Sounds

Create presets in these categories, organized in numbered folders:

### 001-008: Piano-like
Not naturally suited to subtractive synthesis, but create plucky, bell-like tones:
- **001_Synth_Piano**: Plucky attack, fast decay, some sustain
- **002_EP_Clean**: Electric piano simulation with bell-like tone
- **003_EP_Bright**: Brighter electric piano with resonance
- **004_Clav**: Clavinet-style - tight, punchy, filtered
- **005_Harpsichord**: Bright pluck, no sustain
- **006_Celesta**: Bell-like, high octave, soft attack
- **007_Glockenspiel**: Metallic, high pitched, percussive
- **008_Music_Box**: Delicate, high, soft

### 009-016: Chromatic Percussion
- **009_Vibraphone**: Soft attack, tremolo (LFO amp), sustain
- **010_Marimba**: Mellow, wooden, short decay
- **011_Xylophone**: Bright, short, percussive
- **012_Tubular_Bells**: Resonant, long decay, filter sweep
- **013_Dulcimer**: Plucky, bright, medium decay
- **014_Steel_Drums**: Filtered resonance, metallic
- **015_Kalimba**: Soft pluck, hollow tone
- **016_Agogo**: High, metallic, short

### 017-024: Organ
- **017_Drawbar_Organ**: Full, sustained, slight movement
- **018_Perc_Organ**: Attack transient, then sustained
- **019_Rock_Organ**: Distorted/driven organ
- **020_Church_Organ**: Full, majestic, slow attack
- **021_Reed_Organ**: Thinner, harmonium-like
- **022_Accordion**: Tremolo, multiple oscillators
- **023_Harmonica**: Nasal, vibrato
- **024_Tango_Accordion**: Similar to accordion, more attack

### 025-032: Guitar-like
- **025_Nylon_Guitar**: Soft, mellow, plucky
- **026_Steel_Guitar**: Brighter pluck, more presence
- **027_Jazz_Guitar**: Warm, rounded, clean
- **028_Clean_Electric**: Clear, slight chorus/detuning
- **029_Muted_Guitar**: Very short decay, tight
- **030_Overdriven**: Distorted, sustained, driven filter
- **031_Distortion_Guitar**: Heavy drive, aggressive
- **032_Guitar_Harmonics**: High, bell-like, delicate

### 033-040: Bass
- **033_Acoustic_Bass**: Warm, round, medium decay
- **034_Finger_Bass**: Classic synth bass, punchy
- **035_Pick_Bass**: Brighter attack, percussive
- **036_Fretless_Bass**: Smooth attack, slight glide
- **037_Slap_Bass_1**: Punchy, filter envelope pop
- **038_Slap_Bass_2**: More aggressive slap
- **039_Synth_Bass_1**: Classic Moog bass
- **040_Synth_Bass_2**: Modern, deeper, sub-heavy

### 041-048: Strings
- **041_Violin**: High, vibrato (LFO pitch), sustained
- **042_Viola**: Mid-range, warmer than violin
- **043_Cello**: Low, rich, warm
- **044_Contrabass**: Very low, deep
- **045_Tremolo_Strings**: LFO on amplitude
- **046_Pizzicato**: Plucky strings, short decay
- **047_Orchestral_Harp**: Plucky, resonant, sweep
- **048_Timpani**: Low, boomy, percussive

### 049-056: Ensemble
- **049_String_Ensemble_1**: Lush, slow attack, detuned
- **050_String_Ensemble_2**: Brighter ensemble
- **051_Synth_Strings_1**: Classic analog strings
- **052_Synth_Strings_2**: More modern strings
- **053_Choir_Aahs**: Pad-like, vowel filter
- **054_Voice_Oohs**: Softer choir
- **055_Synth_Voice**: Vocal-ish pad
- **056_Orchestra_Hit**: Stab, short, powerful

### 057-064: Brass
- **057_Trumpet**: Bright, attack, vibrato
- **058_Trombone**: Lower, warmer brass
- **059_Tuba**: Low, round brass
- **060_Muted_Trumpet**: Filtered, nasal brass
- **061_French_Horn**: Warm, round, noble
- **062_Brass_Section**: Multiple brass, powerful
- **063_Synth_Brass_1**: Classic synth brass
- **064_Synth_Brass_2**: More aggressive synth brass

### 065-072: Reed
- **065_Soprano_Sax**: High, nasal, vibrato
- **066_Alto_Sax**: Mid sax, jazzy
- **067_Tenor_Sax**: Lower, warmer sax
- **068_Baritone_Sax**: Deep, rich sax
- **069_Oboe**: Nasal, thin, vibrato
- **070_English_Horn**: Warmer oboe
- **071_Bassoon**: Low reed, dark
- **072_Clarinet**: Hollow, woody

### 073-080: Pipe
- **073_Piccolo**: Very high, bright, flutey
- **074_Flute**: Pure, soft, breathy
- **075_Recorder**: Simple, pure tone
- **076_Pan_Flute**: Airy, breathy
- **077_Blown_Bottle**: Very breathy, noise-heavy
- **078_Shakuhachi**: Breathy, expressive
- **079_Whistle**: Pure, high, simple
- **080_Ocarina**: Soft, pure, round

### 081-088: Synth Lead
- **081_Square_Lead**: Classic square wave lead
- **082_Sawtooth_Lead**: Bright, cutting lead
- **083_Calliope_Lead**: Bright, organ-like
- **084_Chiff_Lead**: Attack transient lead
- **085_Charang**: Distorted, aggressive lead
- **086_Voice_Lead**: Vocal-ish lead
- **087_Fifth_Lead**: Lead with fifth harmony
- **088_Bass_Lead**: Fat, heavy lead+bass

### 089-096: Synth Pad
- **089_New_Age_Pad**: Warm, evolving
- **090_Warm_Pad**: Rich, round pad
- **091_Polysynth_Pad**: Classic analog pad
- **092_Choir_Pad**: Vocal, airy pad
- **093_Bowed_Pad**: String-like, sustained
- **094_Metallic_Pad**: Bright, resonant
- **095_Halo_Pad**: Ethereal, spacious
- **096_Sweep_Pad**: Filter sweep movement

### 097-104: Synth Effects
- **097_Rain**: Noise-based, filtered
- **098_Soundtrack**: Atmospheric, evolving
- **099_Crystal**: Bright, bell-like
- **100_Atmosphere**: Pad, textural
- **101_Brightness**: High, shimmering
- **102_Goblins**: Dark, moving
- **103_Echoes**: Delayed feel, movement
- **104_SciFi**: Experimental, modulated

### 105-112: Ethnic
- **105_Sitar**: Resonant, plucky, harmonics
- **106_Banjo**: Bright, plucky
- **107_Shamisen**: Twangy, bright
- **108_Koto**: Plucky, resonant
- **109_Kalimba_Ethnic**: Soft pluck (duplicate ok)
- **110_Bagpipe**: Drone + melody, nasal
- **111_Fiddle**: Folk violin
- **112_Shanai**: Nasal, Eastern

### 113-120: Percussive
- **113_Tinkle_Bell**: High, metallic
- **114_Agogo_Bells**: Metallic percussion
- **115_Steel_Drums_Perc**: Caribbean steel
- **116_Woodblock**: Short, woody
- **117_Taiko_Drum**: Low, boomy
- **118_Melodic_Tom**: Tuned tom
- **119_Synth_Drum**: Electronic drum
- **120_Reverse_Cymbal**: Reversed swell

### 121-128: Sound Effects
- **121_Guitar_Fret**: Noise, texture
- **122_Breath_Noise**: Breathy texture
- **123_Seashore**: Noise sweep
- **124_Bird_Tweet**: High, chirpy
- **125_Telephone_Ring**: Dual tone
- **126_Helicopter**: LFO modulated noise
- **127_Applause**: Noise texture
- **128_Gunshot**: Sharp attack, noise

---

## Preset Design Guidelines

### General Principles

1. **Start with voice mode**: 
   - Bass/Lead → `voice_mode: 0` (Mono-L) or `1` (Mono)
   - Pads/Strings → `voice_mode: 3` (Para) or `4` (Poly)
   - Plucky sounds → `voice_mode: 4` (Poly)

2. **Oscillator selection**:
   - Warm/bass → Sawtooth (2) or Square (3)
   - Bright/cutting → Sawtooth (2) or Pulse (4, 5)
   - Pure/flute-like → Sine (0) or Triangle (1)
   - Hollow/woody → Square (3) or Triangle (1)

3. **Sub oscillator usage**:
   - Bass sounds: `mixer_sub1: -10 to 0 dB`
   - Most other sounds: `mixer_sub1: -60 dB` (off)

4. **Filter envelope for character**:
   - Plucky: Fast attack, medium decay, low sustain, short release
   - Pads: Slow attack, short decay, high sustain, long release
   - Brass: Medium attack, short decay, medium sustain

5. **Amplitude envelope**:
   - Plucky/Percussive: Very fast attack (0.003), medium decay, 0 sustain
   - Sustained: Fast attack (0.01-0.05), full sustain (1.0)
   - Pads: Slow attack (0.5-2.0), full sustain, long release

6. **LFO for life**:
   - Vibrato: `lfo1_pitch: 0.1-0.3`, `lfo1_rate: 5-7 Hz`
   - Tremolo: `lfo1_amp: 0.2-0.5`, `lfo1_rate: 4-8 Hz`
   - Filter movement: `lfo1_filter: 0.1-0.3`, `lfo1_rate: 0.5-2 Hz`

7. **Expression**:
   - Most sounds: `pb_pitch: 2` (standard pitch bend)
   - Leads: Add `vel_filter: 0.5` for dynamics
   - Expressive: Add `at_pitch: 2` for vibrato on aftertouch

---

## Example Preset: Classic Moog Bass

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Neon37Preset name="039_Synth_Bass_1" category="Bass">
  <Parameters>
    <!-- Master -->
    <PARAM id="master_volume" value="0.0"/>
    <PARAM id="master_tune" value="440.0"/>
    
    <!-- Oscillator 1 -->
    <PARAM id="osc1_wave" value="2"/>
    <PARAM id="osc1_octave" value="-1"/>
    <PARAM id="osc1_semitones" value="0"/>
    <PARAM id="osc1_fine" value="0.0"/>
    
    <!-- Oscillator 2 - Off -->
    <PARAM id="osc2_wave" value="2"/>
    <PARAM id="osc2_octave" value="0"/>
    <PARAM id="osc2_semitones" value="0"/>
    <PARAM id="osc2_fine" value="0.0"/>
    
    <!-- Mixer -->
    <PARAM id="mixer_osc1" value="0.0"/>
    <PARAM id="mixer_sub1" value="-12.0"/>
    <PARAM id="mixer_osc2" value="-60.0"/>
    <PARAM id="mixer_noise" value="-60.0"/>
    <PARAM id="mixer_return" value="-60.0"/>
    
    <!-- Filter -->
    <PARAM id="cutoff" value="800.0"/>
    <PARAM id="resonance" value="0.3"/>
    <PARAM id="drive" value="1.5"/>
    <PARAM id="eg_depth" value="50.0"/>
    <PARAM id="key_track" value="0.5"/>
    
    <!-- Filter Envelope -->
    <PARAM id="env1_attack" value="0.001"/>
    <PARAM id="env1_decay" value="0.3"/>
    <PARAM id="env1_sustain" value="0.0"/>
    <PARAM id="env1_release" value="0.1"/>
    
    <!-- Amp Envelope -->
    <PARAM id="env2_attack" value="0.003"/>
    <PARAM id="env2_decay" value="0.1"/>
    <PARAM id="env2_sustain" value="0.8"/>
    <PARAM id="env2_release" value="0.15"/>
    
    <!-- Voice Mode -->
    <PARAM id="voice_mode" value="0"/>
    <PARAM id="hold_mode" value="0"/>
    <PARAM id="gliss_time" value="0.0"/>
    
    <!-- LFOs - Minimal -->
    <PARAM id="lfo1_rate" value="0.1"/>
    <PARAM id="lfo1_wave" value="0"/>
    <PARAM id="lfo1_pitch" value="0.0"/>
    <PARAM id="lfo1_filter" value="0.0"/>
    <PARAM id="lfo1_amp" value="0.0"/>
    
    <!-- Expression -->
    <PARAM id="vel_filter" value="0.5"/>
    <PARAM id="vel_amp" value="0.3"/>
    <PARAM id="pb_pitch" value="2.0"/>
  </Parameters>
</Neon37Preset>
```

---

## File Organization

Create presets in this folder structure:

```
presets/
├── 001_Piano/
│   ├── 001_Synth_Piano.xml
│   ├── 002_EP_Clean.xml
│   └── ...
├── 002_Chromatic/
│   ├── 009_Vibraphone.xml
│   └── ...
├── 003_Organ/
├── 004_Guitar/
├── 005_Bass/
├── 006_Strings/
├── 007_Ensemble/
├── 008_Brass/
├── 009_Reed/
├── 010_Pipe/
├── 011_Synth_Lead/
├── 012_Synth_Pad/
├── 013_Synth_FX/
├── 014_Ethnic/
├── 015_Percussive/
└── 016_Sound_FX/
```

---

## Quality Checklist

For each preset, verify:

1. ✅ All parameters have valid values within their ranges
2. ✅ `voice_mode` is appropriate for the sound type
3. ✅ At least one mixer channel is above -60 dB
4. ✅ `cutoff` is not at 20000 Hz unless intentionally fully open
5. ✅ Envelopes create appropriate amplitude shape
6. ✅ Sound has appropriate character for its GM category
7. ✅ XML is well-formed with proper escaping

---

## Limitations to Consider

Neon-37 is a **subtractive synthesizer** with **Moog-style ladder filter**. Some GM sounds are better suited than others:

### Well-Suited
- ✅ Synth bass, synth leads, synth pads
- ✅ Analog-style brass and strings
- ✅ Organs (with multiple oscillators)
- ✅ Plucky sounds (with fast envelopes)
- ✅ Sound effects with filter sweeps

### Challenging (Approximate)
- ⚠️ Acoustic piano (use plucky, bell-like approach)
- ⚠️ Acoustic guitar (use filtered pluck)
- ⚠️ Realistic orchestral (stylized analog versions)
- ⚠️ Drum sounds (use noise + filter)

---

## Implementation Notes

1. **Save location**: Presets should be saved to the user's preset folder or bundled with the plugin
2. **Loading**: Use the plugin's existing Load functionality to import `.xml` files
3. **Testing**: Each preset should be loaded and played to verify it sounds as intended
4. **Naming**: Use consistent `###_Name` format for sorting

---

## Summary

Create **128 presets** covering the General MIDI palette, organized in 16 category folders. Each preset is an XML file with all necessary parameters. Focus on making musically useful sounds that leverage Neon-37's Moog-style character while approximating the GM sound categories.

The synthesizer's strengths are in bass, leads, pads, and analog-style timbres. Use creative programming to approximate acoustic instruments while embracing the analog synth aesthetic.
