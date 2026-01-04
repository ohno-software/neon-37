# NEON-37 User Manual
## A Musician's Guide to the Neon-37 Software Synthesizer

---

## Table of Contents
1. [Getting Started](#getting-started)
2. [Understanding the Interface](#understanding-the-interface)
3. [Voice Modes](#voice-modes)
4. [Core Sound Design](#core-sound-design)
5. [Modulation & Expression](#modulation--expression)
6. [Playing Techniques](#playing-techniques)
7. [Common Sounds](#common-sounds)
8. [Troubleshooting](#troubleshooting)

---

## Getting Started

### What is Neon-37?

Neon-37 is a software synthesizer inspired by the legendary Moog Subsequent 37 analog synthesizer. It captures the warm, lush, and powerful character that has defined electronic music for decades. Whether you want deep bass, screaming leads, or evolving pads, Neon-37 has the tools to get you there.

### First Time Setup

1. **Load the plugin** in your DAW (Digital Audio Workstation) as you would any other instrument
2. **Set your volume** to around -20dB to avoid surprises
3. **Try a preset** to familiarize yourself with the interface
4. **Play some notes** and experiment!

### Master Controls (Top Left)

- **VOLUME**: Controls the overall output level. Start at -20dB and adjust to your taste
- **MASTER TUNE**: Fine-tune the entire synth in cents (±100 cents = ±1 semitone)
- **VOICES**: Choose between 1 or 2 voice modes (more on this later)
- **TRANSPOSE**: Shift the keyboard up or down by whole octaves

---

## Understanding the Interface

### The Sound Path

Think of Neon-37 like a journey: sound is created by oscillators, shaped by a filter, and controlled by envelopes.

```
Oscillators → Mixer → Drive → Filter → Amplifier → Output
       ↑                              ↑
       └── Modulation Sources (LFO, Envelopes) ──┘
```

### Main Sections Explained

#### 1. **OSCILLATORS** (Top Center-Left)

Oscillators are the sound generators. Neon-37 has two main oscillators plus a sub oscillator (square wave one octave below Oscillator 1).

**For Each Oscillator:**
- **WAVE**: Choose the waveform shape (6 options!)
  - **Sine**: Pure, smooth, mellow tone
  - **Triangle**: Soft, warm sound (fewer harmonics than sawtooth)
  - **Sawtooth**: Bright, rich in harmonics - great for cutting through (default)
  - **Square**: Hollow, woody - classic synth lead sound
  - **25% Pulse**: Narrow pulse wave - more nasal, thinner character
  - **10% Pulse**: Ultra-narrow pulse - metallic, thin and aggressive

- **OCTAVE**: Range from -3 to +3 relative to center pitch
  - Think of these like piano octaves: 0 is standard pitch
  - Lower numbers = deeper sounds
  - Higher numbers = brighter sounds

- **SEMITONES**: Fine pitch in ±12 semitone steps
  - Use to transpose the oscillator by whole semitone intervals

- **FINE**: Ultra-fine detuning (±50 cents)
  - 1 cent = 1/100th of a semitone
  - Use for subtle detuning between oscillators

**Global Oscillator Controls:**
- **FREQUENCY**: Detune Oscillator 2 relative to Oscillator 1 (±7 semitones)
- **BEAT RATE**: Create a subtle "beating" effect between oscillators (±3.5Hz)
  - Lower values = slow beating effect
  - Use for rich, slightly detuned textures
- **HARD SYNC**: Makes Oscillator 2 "lock" to Oscillator 1, creating harsh, buzzing tones (perfect for aggressive lead sounds)
  - Modulate FREQUENCY while HARD SYNC is on for evolving textures
- **KEY SYNC**: Resets the oscillator waveforms each time you press a key, creating clean, punchy attacks (recommended for most sounds)

#### 2. **MIXER** (Left Center)

This is where you blend your sound sources together. Think of it like the faders on a mixing board.

- **OSC 1**: Volume of Oscillator 1
- **SUB 1**: Volume of the Sub Oscillator
- **OSC 2**: Volume of Oscillator 2
- **NOISE**: White noise generator (add texture, or use for hi-hat sounds)
- **RETURN**: Input for external audio (advanced users)

**Tip**: Keep faders at -60dB (all the way down) for sources you're not using. Mix in the ones you need.

#### 3. **FILTER** (Center)

The filter is the heart of the Moog sound. It removes high frequencies, letting you sculpt the tone.

- **CUTOFF**: How open the filter is (20Hz to 20kHz)
  - All the way up = bright, full highs (fully open)
  - Mid-range = warm, balanced
  - Low = dark, deep, muffled
  - Default: Wide open (20kHz) - you'll need to close it to hear filtering effects

- **RESONANCE**: Creates a "peak" at the cutoff frequency (0.0 to 1.2)
  - 0 = no emphasis, smooth filtering
  - 0.5 = strong peak (classic Moog character)
  - 1.0+ = intense resonance with self-oscillation (can sing like a synth lead)
  - **Tip**: Higher resonance = can self-oscillate to create bell/chime tones

- **DRIVE**: Adds distortion and warmth (1.0 to 25.0)
  - 1.0 = clean, transparent filtering
  - 2.0-5.0 = warm, slightly gritty Moog character
  - 5.0+ = aggressive, aggressive bass/lead sounds with saturation

- **EG DEPTH**: How much the Filter Envelope affects the filter opening/closing (-100 to +100)
  - Positive = envelope opens the filter
  - Negative = envelope closes the filter
  - 0 = no envelope effect

- **KEY TRACK**: Makes the filter respond to keyboard pitch (0 to 2.0)
  - 0 = filter stays fixed regardless of key
  - 1.0 = filter opens proportionally to higher keys (natural response)
  - 2.0 = extreme tracking (very bright on high notes)

#### 4. **ENVELOPES** (Center-Right)

Envelopes control how sounds evolve over time. Neon-37 has two complete ADSR envelopes.

**Envelope 1 (Filter Envelope):**
Controls the movement of the filter over time. Use this to create sweeping, dramatic filter changes.

**Envelope 2 (Amplitude Envelope):**
Controls the volume envelope - how loud the sound is from attack to release.

**ADSR Parameters (for each envelope):**
- **ATTACK**: How quickly the sound reaches full volume (0.001s to 10s)
  - 0.001s = instant (punchy, sharp attack)
  - 0.1-1s = gradual fade-in
  - 10s = slow, smooth swell-in (for pads)
  - **Tip**: Very fast attacks (< 0.01s) create percussive, snappy sounds

- **DECAY**: How quickly the sound drops after the initial peak (0.003s to 10s)
  - Short decay (0.1s) = percussive, snappy character
  - Long decay (1-5s) = smooth, evolving sounds

- **SUSTAIN**: The level the sound holds while you keep the note playing (0-100%)
  - 100% = stays at full volume (use for pads, drones)
  - 50% = drops to half volume (natural pluck-like behavior)
  - 0% = drops to silence (with decay) - creates one-shot percussive sounds

- **RELEASE**: How quickly the sound disappears after you let go of the key (0.003s to 10s)
  - 0ms = stops immediately (abrupt, unmusical)
  - 0.1-0.5s = quick fade
  - 1-3s = long, dramatic tail (for pads and strings)

**Special Envelope Controls:**
- **RETRIG**: Each new note restarts the envelope from the beginning (vs. gate-based)
- **RESET**: Manual reset button for the envelope
- **SYNC**: Sync envelope to DAW/host tempo
- **CYCLE**: Free-running cycle mode (envelope loops continuously)
- **HOLD ON**: Hold at sustain level indefinitely (ignores release)
- **EXP CURV**: Enabled by default. Creates natural exponential curves instead of linear ramps

#### 5. **MODULATION SECTIONS** (MOD 1 & MOD 2) (Right Side)

These are modulation sources - they create rhythmic or evolving changes to other parameters. Neon-37 has **two complete, independent LFO sections**.

**LFO (Low Frequency Oscillator):**
- **RATE**: How fast the modulation oscillates (0.01Hz to 100Hz)
  - 0.1Hz = very slow, dreamy changes (1 cycle per 10 seconds)
  - 1Hz = moderate speed
  - 10Hz+ = fast, vibrato-like modulation
- **WAVE**: Choose the modulation shape
  - **Triangle** = smooth, flowing modulation
  - **Ramp Up** = gradual rise then instant drop (creates "wah" effects)
  - **Ramp Down** = instant rise then gradual drop
  - **Square** = on/off, two-state modulation (useful for special effects)
  - **Sample & Hold** = random stepped modulation (creates glitchy effects)

**Tempo Sync:**
- **SYNC**: Toggle button to sync LFO to your DAW's tempo (instead of free-running)
- **SYNC VALUES**: When SYNC is on, choose note divisions (1/64 to 8/1)
  - Use this to make LFO speeds match your song's tempo

**LFO Depth Controls (0-1 range):**
- **PITCH DEPTH**: How much the LFO modulates oscillator pitch
  - Use for vibrato effects, tremolo effects
- **FILTER DEPTH**: How much the LFO modulates the filter cutoff
  - Creates wah/sweep effects, evolving brightness
- **AMP DEPTH**: How much the LFO modulates volume
  - Creates tremolo (volume wobble)

**Expression Modulation:**
- **MOD WHEEL**: Enable/disable mod wheel routing
  - When enabled, your mod wheel or expression pedal scales all modulation depths
  - **Depth**: 0-1 (how much effect the wheel has when moved)
- **VELOCITY**: Modulate based on how hard you play
  - **Pitch Depth**: ±12 semitones per velocity (snap to semitones)
  - **Filter Depth**: ±5 multiplier (continuous, can make filter more/less responsive)
  - **Amp Depth**: ±2 multiplier (continuous, can make velocity affect dynamics more/less)
- **AFTERTOUCH**: Modulate based on finger pressure after key press
  - **Pitch Depth**: ±12 semitones
  - **Filter Depth**: ±5 multiplier
  - **Amp Depth**: ±2 multiplier
  - **Note**: Works per-voice in Poly mode, per-voice in Para modes
- **PITCH BEND**: Standard pitch bend control (1-12 semitone range, default 2 semitones)

#### 6. **GLIDE/PORTAMENTO** (Bottom Left)

Glide smoothly between notes instead of jumping instantly.

- **TIME**: How long the glide takes (0 to 10s)
  - 0s = no glide (instant), OFF
  - 0.1-0.5s = musical glide effect
  - 1s+ = dramatic swooping slides
- **RTE/TME/LOG**: Different glide modes (experiment to find your preference)
  - **TME** (Time) is the default mode

**Special Glide Features:**
- **ON GAT LEG**: Glide only works during legato playing (holding notes continuously)
- These mode toggles let you choose glide characteristics for your style

---

## Voice Modes

### Choosing Your Voice Mode

Neon-37 offers 5 different voice modes, each optimized for different playing styles. Choose the mode that matches your music:

**Mono-L (Monophonic Legato)** - Best for smooth bass lines
- Plays only one note at a time (the lowest key held)
- Oscillators stay running continuously
- Pitch slides smoothly from note to note without clicking
- **Use**: Smooth bass lines, leads, expressive slides and bends
- **Characteristic**: Most natural legato feel, continuous tone

**Mono (Monophonic Retrigger)** - Best for punchy riffs
- Plays only one note at a time (the lowest key held)
- Each new note triggers a fresh envelope attack
- Sharp, punchy response on each new note
- **Use**: Punchy bass riffs, rhythmic patterns, drum-like percussion
- **Characteristic**: Percussive, articulate attack on every note

**Para-L (Paraphonic Legato)** - Best for 2-3 note chords with expression
- Plays up to 8 notes simultaneously
- All notes share one filter and one filter envelope (unified sound)
- Each note has its own oscillators and triggers its own amp envelope
- **Use**: 2-3 note chords, harmonic textures, chord stabs
- **Characteristic**: Unified filter sweep across all notes, clean attacking on each note

**Para (Paraphonic Gate)** - Best for sustained chords and pads
- Plays up to 8 notes simultaneously
- All notes share one filter, filter envelope, AND amp envelope
- Envelope triggers when first key presses, releases when last key lifts
- **Use**: String pads, sustained chords, lush textures
- **Characteristic**: Most unified feel, all notes rise and fall together

**Poly (Polyphonic)** - Best for full keyboard play and complex passages
- Plays up to 8 fully independent voices
- Each voice has complete oscillators, filter, and envelopes
- Like a traditional polyphonic synthesizer
- **Use**: Full polyphonic play, complex passages, normal synth behavior
- **Characteristic**: Maximum expressiveness, each note is completely independent

#### 7. **ARPEGGIATOR** (Bottom Right)

Plays your held notes in a rhythmic pattern automatically.

- **ON/ALT/DIR**: Enable, alternate, or reverse the arpeggio pattern
- **RATE**: Tempo of the arpeggio
- **RANGE**: How many octaves the arpeggio spans
- **GATE**: How long each note plays
- **PATTERN**: Different arpeggio sequences
- **SWING**: Add swing/shuffle to the arpeggio timing

---

## Core Sound Design

### Building Your First Sound: Deep Bass

1. **Set the Oscillators:**
   - OSC 1: Sawtooth, Octave 16'
   - OSC 2: Off (set to -60dB in mixer)
   - SUB 1: Turned on (around -15dB)

2. **Set the Mixer:**
   - OSC 1: 0dB
   - SUB 1: -10dB to -15dB (adds depth)
   - Everything else: -60dB

3. **Set the Filter:**
   - CUTOFF: Closed (around 20-30%)
   - RESONANCE: 20-30% (adds character)
   - DRIVE: 1.0-1.5
   - EG DEPTH: +3

4. **Set Envelope 1 (Filter):**
   - ATTACK: 0.5s
   - DECAY: 1s
   - SUSTAIN: 0%
   - RELEASE: 0.5s
   - This creates a sweeping filter effect

5. **Set Envelope 2 (Amplitude):**
   - ATTACK: 0.05s (quick punch)
   - DECAY: 0.1s
   - SUSTAIN: 100% (stays open while playing)
   - RELEASE: 0.1s

Play a low note and hear the filter sweep up and then settle. Adjust the CUTOFF and EG DEPTH to taste!

### Building Your Second Sound: Screaming Lead

1. **Set the Oscillators:**
   - OSC 1: Sawtooth, Octave 4'
   - OSC 2: Square, Octave 4'
   - HARD SYNC: On
   - FREQUENCY: +3 to +5 semitones (detune OSC 2 slightly)

2. **Set the Mixer:**
   - OSC 1: -5dB
   - OSC 2: -5dB
   - SUB 1: -60dB (off)
   - NOISE: -60dB (off)

3. **Set the Filter:**
   - CUTOFF: Open (80%)
   - RESONANCE: 40-50% (adds presence)
   - DRIVE: 2.0-3.0
   - EG DEPTH: +2

4. **Set Envelope 1 (Filter):**
   - ATTACK: 0.01s (instant bright attack)
   - DECAY: 0.2s
   - SUSTAIN: 70%
   - RELEASE: 0.3s

5. **Set Envelope 2 (Amplitude):**
   - ATTACK: 0.01s
   - DECAY: 0.05s
   - SUSTAIN: 100%
   - RELEASE: 0.5s

6. **Add Modulation with MOD 1:**
   - LFO RATE: 5-7Hz
   - WAVE: Triangle
   - PITCH DEPTH: 0.3-0.5 (adds vibrato)
   - FILTER DEPTH: 0.2 (adds movement)

Play higher notes and hear the attitude!

### Building Your Third Sound: Evolving Pad

1. **Set the Oscillators:**
   - OSC 1: Triangle, Octave 8'
   - OSC 2: Sine, Octave 8'
   - BEAT RATE: 0.2Hz (very subtle detuning)

2. **Set the Mixer:**
   - OSC 1: 0dB
   - OSC 2: -10dB (quieter to maintain blend)
   - SUB 1: -60dB (off)

3. **Set the Filter:**
   - CUTOFF: 60-70%
   - RESONANCE: 10% (minimal)
   - DRIVE: 1.0
   - EG DEPTH: +1

4. **Set Envelope 1 (Filter):**
   - ATTACK: 2s (very slow, dreamy opening)
   - DECAY: 0.5s
   - SUSTAIN: 60%
   - RELEASE: 2s

5. **Set Envelope 2 (Amplitude):**
   - ATTACK: 1s
   - DECAY: 0.1s
   - SUSTAIN: 100%
   - RELEASE: 1s

6. **Add Movement with MOD 1:**
   - LFO RATE: 0.5Hz (very slow)
   - WAVE: Triangle
   - FILTER DEPTH: 0.3 (creates gentle wah effect)

Let notes ring out—the pad should gradually bloom and evolve.

---

## Modulation & Expression

### Using the Mod Wheel

The Mod Wheel (or expression controller) is one of the most expressive tools you have:

1. Push **MOD 1** or **MOD 2** to affect it
2. Set **WHEEL** depth to control how much effect the wheel has
3. Move your controller's mod wheel while playing to modulate:
   - **PITCH DEPTH**: Creates vibrato (wobbling pitch)
   - **FILTER DEPTH**: Creates wah/sweep effects
   - **MOD DEPTH**: Modulates other modulation sources

**Performance Tip**: Assign Filter Depth to your mod wheel for dramatic, expressive filter sweeps during solos.

### Using Velocity

Velocity is how hard you hit each key. Neon-37 can respond to this in several ways:

1. Set **VEL DEPTH** on your envelopes
2. Set **VELOCITY** depth on modulation sections
3. Play keys hard or soft to control:
   - Attack sharpness
   - Filter brightness
   - Overall dynamics

**Performance Tip**: Play dynamically! Hard strikes create aggressive attacks, soft touches create gentle swells.

### Using Aftertouch

If your keyboard has aftertouch, you can modulate sounds after pressing a key:

1. Press and hold a note
2. Apply pressure (press deeper) with your finger
3. **AFTERTOUCH** depth in modulation sections controls the effect

---

## Playing Techniques

### Voice Modes in Practice

**Choosing Mono-L for Bass**:
1. Set VOICE MODE to "Mono-L"
2. Play a bass line with smooth slides
3. You'll notice the pitch glides smoothly between notes without clicking

**Choosing Para-L for Pads**:
1. Set VOICE MODE to "Para-L"
2. Hold down a 3-note chord
3. Each note attacks separately, but they all share the filter sweep
4. Great for lush, unified textures

**Choosing Poly for Melodies**:
1. Set VOICE MODE to "Poly"
2. Play a full polyphonic passage
3. Each note is independent with its own envelope
4. Can play up to 8 simultaneous notes

### Using Hard Sync

Hard Sync creates unique, buzzy textures:

1. Turn on **HARD SYNC** button in the Oscillator section
2. Play a note
3. Use **FREQUENCY** knob to detune OSC 2
4. You'll hear buzzing, metallic, aggressive tones
5. **Try**: Modulating FREQUENCY with MOD 1 while HARD SYNC is on for evolving harsh sounds

**Best for**: Lead sounds, special effects, aggressive bass

### Using Glide for Expression

Create smooth, expressive slides:

1. Set **GLISS TIME** to 0.3-0.5 seconds
2. Play two notes in sequence (hold the first, then press the second)
3. The pitch glides smoothly between them
4. Works best in Mono-L mode for the smoothest effect

**Best for**: Violin-like leads, basslines, expressive solos

### Using the Mod Wheel

The Mod Wheel is your primary live performance tool:

1. Assign MOD 1 or MOD 2 to effect parameters
2. Set **WHEEL ENABLE** to ON
3. Set the modulation depths (PITCH, FILTER, AMP)
4. Move your mod wheel while playing to modulate:
   - **PITCH DEPTH**: Creates vibrato (wobbling pitch)
   - **FILTER DEPTH**: Creates wah/sweep effects (filter brightness changes)
   - **AMP DEPTH**: Creates volume modulation (tremolo)

**Performance Tip**: Set FILTER DEPTH on MOD 1 for dramatic, expressive filter sweeps during solos.

### Using Velocity for Dynamics

Velocity is how hard you hit each key. Neon-37 can respond to this in several ways:

1. Set **VEL PITCH**, **VEL FILTER**, or **VEL AMP** values
2. Play keys hard or soft to control:
   - Pitch changes (for expressive bends)
   - Filter brightness (harder = brighter)
   - Volume dynamics (harder = louder)

**Performance Tip**: Play dynamically! Hard strikes create aggressive attacks, soft touches create gentle swells.

---

## Common Sounds

### Classic Moog Bass
**Setup**:
- Voice Mode: **Mono-L** (for smooth playing)
- Osc 1: **Sawtooth**, Octave: **-3** (16')
- Osc 2: **Off** (-60dB in mixer)
- Sub 1: **-12dB** (turned on for extra weight)
- Filter: Cutoff **20%**, Resonance **40%**, Drive **1.0**
- Env 1: A **0.05s**, D **0.1s**, S **0%**, R **0.1s**
  - This closes the filter after attack for a classic sweep
- Env 2: A **0.05s**, D **0.05s**, S **100%**, R **0.3s**

**Result**: Punchy, deep, warm bass with filter sweep

### Screaming Lead
**Setup**:
- Voice Mode: **Mono** (for punchy retrigger)
- Osc 1: **Sawtooth**, Octave: **0**
- Osc 2: **Square**, Octave: **0**
- Hard Sync: **ON**
- Frequency: **+3 semitones** (detune OSC 2)
- Mixer: Osc 1 **-3dB**, Osc 2 **-3dB**
- Filter: Cutoff **80%**, Resonance **50%**, Drive **2.0**
- Env 1: A **0.01s**, D **0.2s**, S **70%**, R **0.3s**
- Env 2: A **0.01s**, D **0.05s**, S **100%**, R **0.5s**
- MOD 1: Pitch Depth **0.3**, Rate **5Hz**, Wave **Triangle**

**Result**: Piercing, expressive lead with vibrato and movement

### Lush Pad
**Setup**:
- Voice Mode: **Para** (for unified expression)
- Osc 1: **Triangle**, Octave: **0**
- Osc 2: **Sine**, Octave: **0**
- Beat Rate: **0.2Hz** (subtle detuning)
- Mixer: Osc 1 **0dB**, Osc 2 **-10dB**
- Filter: Cutoff **60%**, Resonance **10%**, Drive **1.0**
- Env 1: A **2s**, D **0.5s**, S **60%**, R **2s**
- Env 2: A **1s**, D **0.1s**, S **100%**, R **1s**
- MOD 1: Filter Depth **0.3**, Rate **0.5Hz**, Wave **Triangle**

**Result**: Smooth, evolving pad that blooms slowly

### Bright Pluck
**Setup**:
- Voice Mode: **Mono-L**
- Osc 1: **Triangle**, Octave: **0**, Fine **+20 cents**
- Osc 2: **Off**
- Sub 1: **Off**
- Filter: Cutoff **100%** (open), Resonance **0%**, Drive **1.0**
- EG Depth: **-50** (inverted: closes filter on decay)
- Env 1: A **0.01s**, D **0.8s**, S **0%**, R **0.1s**
- Env 2: A **0.01s**, D **0.2s**, S **0%**, R **0.1s**

**Result**: Percussive pluck with filter closing over time

### Vocal Sweep Lead
**Setup**:
- Voice Mode: **Mono**
- Osc 1: **Pulse 25%**, Octave: **0**
- Osc 2: **Off**
- Sub 1: **Off**
- Filter: Cutoff **40%**, Resonance **60%**, Drive **1.5**
- EG Depth: **+80** (opens filter dramatically)
- Env 1: A **0.1s**, D **0.5s**, S **20%**, R **0.3s**
- Env 2: A **0.05s**, D **0.1s**, S **100%**, R **0.3s**

**Result**: Expressive vocal-like lead with dramatic filter sweep

---

## Troubleshooting

### Sound is too quiet
- Check **MASTER VOLUME** at top left (raise it from -20dB toward 0dB)
- Check mixer faders—are key sources at -60dB?
- Raise **OSC 1** or **SUB 1** fader above -60dB
- Check if you're in Poly mode (multiple voices = quieter overall)

### Sound is too bright/harsh
- Lower **CUTOFF** on filter
- Lower **RESONANCE** on filter
- Lower **DRIVE**
- Try **KEY SYNC ON** to ensure clean phase reset
- Turn down high oscillator octaves

### Sound has no attack/sounds mushy
- Raise **ATTACK** setting in Envelope 2 (amplitude) if you want slower fade-in
- **Lowering** ATTACK creates snappier attacks (shorter fade-in)
- Enable **KEY SYNC** on oscillators (resets phase for clean attacks)
- Check that **DECAY** isn't too long

### Notes don't glide/slide
- Check if **GLISS TIME** is at 0s (no glide)
- Raise GLISS TIME value to 0.2-0.5s
- Verify you're playing **Mono-L** or **Mono** mode (best for glide)
- Glide works by sliding pitch between sequential notes

### Filter not doing anything
- CUTOFF might be at 100% (fully open) - close it to around 50% first
- Check **EG DEPTH** - set to +50 to hear envelope effects
- Make sure **ENV 1** (Filter Envelope) has some ATTACK or DECAY time
- Check that SUSTAIN in ENV 1 isn't at 100% (you won't hear the sweep if it stays open)

### Oscillators sound thin/weak
- Turn up **OSC 1** fader in mixer (check it's not at -60dB)
- Try enabling **SUB 1** around -12dB to add low end
- Enable **HARD SYNC** for more aggressive tones
- Use **Sawtooth** or **Square** waves instead of **Sine**

### LFO not creating visible effect
- Increase **PITCH DEPTH** or **FILTER DEPTH** on the LFO
- Increase LFO **RATE** if it's very slow
- Check **SYNC** isn't off (should be OFF for normal LFO operation, ON for tempo sync)
- Make sure FILTER DEPTH or PITCH DEPTH isn't at 0

### Filter Envelope not sweeping
- Set **EG DEPTH** to a value greater than 0 (try +50)
- Make sure Envelope 1 has some **DECAY** time (it's not zero)
- Check that **SUSTAIN** isn't at 100% in ENV 1 (you need movement to hear the sweep)
- Try lowering CUTOFF to around 30% first, then adjust EG DEPTH

### Sound cuts off abruptly
- Raise **RELEASE** in Envelope 2 (amplitude)
- Check you're not in a voice mode that's causing notes to stop (try **Poly**)
- Verify max voice count - if playing more than 8 notes, older notes steal voices

### Mod Wheel doesn't work
- Check **WHEEL ENABLE** toggle is ON
- Make sure you set a depth value (PITCH DEPTH, FILTER DEPTH, or AMP DEPTH > 0)
- Try moving the mod wheel on your keyboard to see if it's detected
- Some keyboards send CC7 (Volume) instead of CC1 (Mod Wheel) - check your keyboard settings

### Velocity not working
- Set **VEL PITCH**, **VEL FILTER**, or **VEL AMP** to non-zero values
- Play keys harder vs. softer to test velocity response
- Make sure your keyboard transmits velocity data
- Try different voice modes to hear velocity effects

---

## Tips for Getting the Most from Neon-37

1. **Start Simple**: Create sounds with one oscillator first, then add complexity
2. **Use Your Ears**: Trust your ear more than the numbers
3. **Experiment with Detuning**: Slight detuning between oscillators creates richness
4. **Layer Modulation**: Combine multiple modulation sources for complex, evolving textures
5. **Learn the Filter**: Spend time understanding how CUTOFF, RESONANCE, and ENVELOPE interact
6. **Record Everything**: Save your favorite settings as presets for quick recall
7. **Play Expressively**: Use velocity, mod wheel, and aftertouch to bring sounds to life
8. **Work with Your DAW**: Sync arpeggiator to host BPM for rhythmic integration

---

## Advanced Tips

### Creating Evolving Textures
- Assign MOD 1 and MOD 2 to different parameters
- Set them to different LFO rates (e.g., 1Hz and 1.3Hz)
- The slight mismatch creates slowly-evolving, never-repeating textures

### Using Resonance for Expression
- High resonance + modulated CUTOFF = dramatic filter resonance peaks
- This can create vocal-like, talking effects
- Try RESONANCE at 1.0+ with MOD 1 FILTER DEPTH at 0.5

### Hard Sync Modulation
- Modulate **FREQUENCY** while HARD SYNC is on for aggressive, evolving buzzing
- Try syncing the modulation speed to your DAW tempo
- Great for sci-fi sounds and experimental textures

### Multi-Layer Modulation
- Use velocity to control how much the LFO affects the filter
- Use aftertouch to control LFO pitch depth for expressive vibrato
- Combine pitch bend with modulation for complex, expressive leads

### Filter Drive as a Tool
- DRIVE **isn't just distortion** - it shapes the filter resonance character
- Low DRIVE (1.0-2.0) = clean Moog tone
- High DRIVE (10.0-25.0) = warm, saturated, slightly aggressive
- Experiment at different RESONANCE values for different character

### Pulse Width Modulation
- Use **25% Pulse** or **10% Pulse** waveforms for tonal richness
- Modulate PITCH with LFO for pseudo-PWM effects
- 10% Pulse sounds especially thin/aggressive - great for leads

### Keyboard Tracking for Intelligence
- Set **KEY TRACK** to 1.0 or higher for natural brightness response
- High notes stay bright, low notes stay dark (as nature intended)
- KEY TRACK of 2.0 = extreme effect (very bright on high keys)

---

## Final Thoughts

Neon-37 is built to inspire creativity and deliver the warmth and character of classic analog synthesis. The key to mastering any synthesizer is experimentation—don't be afraid to turn knobs, try new combinations, and trust your ears.

**Quick Start Tips:**
1. Start with simple sounds (one oscillator, open filter)
2. Understand envelopes before adding modulation
3. Use MOD 1 to add movement to simple sounds
4. Choose voice modes based on your playing style
5. Save sounds you love (they're all preset-able)

**Common Beginner Mistakes to Avoid:**
- Leaving CUTOFF wide open (you can't hear the filter)
- Setting all mixer faders too low (-60dB = silent)
- Forgetting to enable LFO depths (high rate but 0 depth = no effect)
- Using too much ATTACK (slow fade makes notes sound weak)
- Not enabling KEY SYNC (causes clicks and pops)

Remember: Every control on Neon-37 is there for a reason. Experiment fearlessly, and you'll discover sounds that inspire your music.

Happy synthesizing! 🎹

---

**Neon-37 User Manual v2.0**
*Updated to reflect actual implementation with 5 voice modes, 2 full LFO sections, and advanced modulation capabilities*
