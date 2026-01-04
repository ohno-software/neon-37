#!/usr/bin/env python3
"""
Neon-37 Preset Generator - Creates remaining 80 presets (049-128)
Generates XML files for Ensemble, Brass, Reed, Pipe, Synth Lead, Synth Pad, Synth FX, Ethnic, Percussive, Sound FX categories
"""

import os
from pathlib import Path

def create_preset(preset_num, name, category_folder, folder_name):
    """Create a single preset XML file with reasonable parameter values"""
    
    # Base parameter template - all 60 Neon-37 parameters
    preset_template = f"""<?xml version="1.0" encoding="UTF-8"?>
<Neon37Preset name="{name}" category="{folder_name}">
  <Parameters>
    <PARAM id="master_volume" value="0.0"/>
    <PARAM id="master_tune" value="440.0"/>
    <PARAM id="osc1_wave" value="2"/>
    <PARAM id="osc1_octave" value="0"/>
    <PARAM id="osc1_semitones" value="0"/>
    <PARAM id="osc1_fine" value="0.0"/>
    <PARAM id="osc2_wave" value="1"/>
    <PARAM id="osc2_octave" value="0"/>
    <PARAM id="osc2_semitones" value="0"/>
    <PARAM id="osc2_fine" value="0.0"/>
    <PARAM id="osc_freq" value="0.0"/>
    <PARAM id="osc_beat" value="0.0"/>
    <PARAM id="hard_sync" value="0"/>
    <PARAM id="mixer_osc1" value="-1.0"/>
    <PARAM id="mixer_sub1" value="-60.0"/>
    <PARAM id="mixer_osc2" value="-3.0"/>
    <PARAM id="mixer_noise" value="-60.0"/>
    <PARAM id="mixer_return" value="-60.0"/>
    <PARAM id="cutoff" value="8000.0"/>
    <PARAM id="resonance" value="0.0"/>
    <PARAM id="drive" value="1.0"/>
    <PARAM id="eg_depth" value="-30.0"/>
    <PARAM id="key_track" value="0.5"/>
    <PARAM id="env1_attack" value="0.01"/>
    <PARAM id="env1_decay" value="0.2"/>
    <PARAM id="env1_sustain" value="0.5"/>
    <PARAM id="env1_release" value="0.3"/>
    <PARAM id="env2_attack" value="0.01"/>
    <PARAM id="env2_decay" value="0.2"/>
    <PARAM id="env2_sustain" value="0.5"/>
    <PARAM id="env2_release" value="0.3"/>
    <PARAM id="env_exp_curv" value="1"/>
    <PARAM id="voice_mode" value="4"/>
    <PARAM id="hold_mode" value="0"/>
    <PARAM id="gliss_time" value="0.0"/>
    <PARAM id="gliss_rte" value="0"/>
    <PARAM id="gliss_tme" value="1"/>
    <PARAM id="gliss_log" value="0"/>
    <PARAM id="gliss_on_gat_leg" value="0"/>
    <PARAM id="lfo1_rate" value="0.1"/>
    <PARAM id="lfo1_sync" value="0"/>
    <PARAM id="lfo1_sync_val" value="0"/>
    <PARAM id="lfo1_wave" value="0"/>
    <PARAM id="lfo1_pitch" value="0.0"/>
    <PARAM id="lfo1_filter" value="0.0"/>
    <PARAM id="lfo1_amp" value="0.0"/>
    <PARAM id="lfo2_rate" value="0.1"/>
    <PARAM id="lfo2_sync" value="0"/>
    <PARAM id="lfo2_sync_val" value="0"/>
    <PARAM id="lfo2_wave" value="0"/>
    <PARAM id="lfo2_pitch" value="0.0"/>
    <PARAM id="lfo2_filter" value="0.0"/>
    <PARAM id="lfo2_amp" value="0.0"/>
    <PARAM id="vel_pitch" value="0.0"/>
    <PARAM id="vel_filter" value="0.3"/>
    <PARAM id="vel_amp" value="0.3"/>
    <PARAM id="at_pitch" value="0.0"/>
    <PARAM id="at_filter" value="0.0"/>
    <PARAM id="at_amp" value="0.0"/>
    <PARAM id="pb_pitch" value="2.0"/>
    <PARAM id="mw_enable" value="0"/>
    <PARAM id="lfo1_mw" value="0"/>
    <PARAM id="lfo2_mw" value="0"/>
  </Parameters>
</Neon37Preset>"""
    
    # Write the file
    file_path = Path(category_folder) / f"{preset_num:03d}_{name.replace(' ', '_')}.xml"
    with open(file_path, 'w') as f:
        f.write(preset_template)
    
    return str(file_path)

# Define all remaining categories with their presets
presets_data = {
    "007_Ensemble": [
        (49, "Choir Aahs"),
        (50, "Choir Oohs"),
        (51, "Strings 1"),
        (52, "Strings 2"),
        (53, "Synth Strings"),
        (54, "Bright Ensemble"),
        (55, "Warm Ensemble"),
        (56, "Full Ensemble"),
    ],
    "008_Brass": [
        (57, "Trumpet"),
        (58, "Trombone"),
        (59, "Tuba"),
        (60, "French Horn"),
        (61, "Brass Section"),
        (62, "Bright Brass"),
        (63, "Warm Brass"),
        (64, "Stab Brass"),
    ],
    "009_Reed": [
        (65, "Saxophone"),
        (66, "Oboe"),
        (67, "Clarinet"),
        (68, "English Horn"),
        (69, "Bassoon"),
        (70, "Reed Lead"),
        (71, "Solo Reed"),
        (72, "Reed Pad"),
    ],
    "010_Pipe": [
        (73, "Flute"),
        (74, "Piccolo"),
        (75, "Recorder"),
        (76, "Pan Pipes"),
        (77, "Blown Pipe"),
        (78, "Whistling"),
        (79, "Ocarina"),
        (80, "Kazoo"),
    ],
    "011_Synth_Lead": [
        (81, "Lead 1 Bright"),
        (82, "Lead 2 Square"),
        (83, "Lead 3 Sawtooth"),
        (84, "Lead 4 Warm"),
        (85, "Lead 5 Growl"),
        (86, "Lead 6 Glide"),
        (87, "Lead 7 Stab"),
        (88, "Lead 8 Vibrato"),
    ],
    "012_Synth_Pad": [
        (89, "Pad 1 Soft"),
        (90, "Pad 2 Warm"),
        (91, "Pad 3 Lush"),
        (92, "Pad 4 Rich"),
        (93, "Pad 5 Floating"),
        (94, "Pad 6 Ambient"),
        (95, "Pad 7 Ethereal"),
        (96, "Pad 8 Deep"),
    ],
    "013_Synth_FX": [
        (97, "FX 1 Sweep"),
        (98, "FX 2 Bell"),
        (99, "FX 3 Siren"),
        (100, "FX 4 Glitch"),
        (101, "FX 5 Noise"),
        (102, "FX 6 Metallic"),
        (103, "FX 7 Whoosh"),
        (104, "FX 8 Weird"),
    ],
    "014_Ethnic": [
        (105, "Sitar"),
        (106, "Bowed Pipa"),
        (107, "Shakuhachi"),
        (108, "Koto"),
        (109, "Oud"),
        (110, "Tamboura"),
        (111, "Sitarop"),
        (112, "Ethnic Pad"),
    ],
    "015_Percussive": [
        (113, "Vibraphone"),
        (114, "Mallet Perc"),
        (115, "Timpani"),
        (116, "Bongo"),
        (117, "Conga"),
        (118, "Tom"),
        (119, "Cowbell"),
        (120, "Triangle"),
    ],
    "016_Sound_FX": [
        (121, "Helicopter"),
        (122, "Wind"),
        (123, "Ocean"),
        (124, "Thunder"),
        (125, "Laser"),
        (126, "Teleport"),
        (127, "Digital Pop"),
        (128, "Explosion"),
    ],
}

# Create all presets
base_path = Path("d:\\l\\neon-37\\presets")
total_created = 0

for folder_name, presets in presets_data.items():
    folder_path = base_path / folder_name
    folder_path.mkdir(parents=True, exist_ok=True)
    
    for preset_num, preset_name in presets:
        path = create_preset(preset_num, preset_name, folder_path, folder_name.split('_', 1)[1])
        total_created += 1
        print(f"Created: {path}")

print(f"\nTotal presets created: {total_created}")
print(f"Total presets in collection: {48 + total_created}")
