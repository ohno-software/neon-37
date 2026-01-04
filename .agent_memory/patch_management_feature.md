# Patch Management Feature ("Programmer")

## Overview
Implement patch save/load functionality for Neon37 synth presets in JSON format.

## Requirements

### 1. File Location
- **Path**: `%USERPROFILE%\.neonaudio\neon37\patches`
- Auto-create directory structure if it doesn't exist
- User presets only (no factory presets at launch)

### 2. Patch Name Input
- Textbox in patch management section (top of UI)
- Only allow Windows filename compatible characters
- Invalid keypresses do nothing (silently rejected)
- Processor converts patch name → filename automatically

### 3. File Format
- `.json` files containing only parameter data
- No metadata (date, author, description)
- Simple flat structure with parameter ID → value mappings

### 4. UI Controls
- **Patch Name Box**: Large textbox for user input
- **SAVE Button**: Save current state with entered name (overwrites if exists)
- **LOAD Button**: File browser dialog to select .json preset
- **NEW Button**: Reset entire interface to default values

### 5. Load Behavior
- If user tries to LOAD without saving current patch:
  - Show confirmation dialog: "Save current patch before loading?"
  - If YES: Show save dialog (like Save button)
  - If NO: Proceed with load
  - If CANCEL: Do nothing

### 6. Error Handling
- Display errors in modal dialog with plain text area
- Show complete stack trace for debugging
- Text must be selectable and copyable (standard Windows selection)
- No special formatting or controls needed

## Implementation Status
- [x] Feature specification documented
- [x] Patch name textbox validation (Windows filename characters only)
- [x] File I/O with directory creation (.neonaudio/neon37/patches)
- [x] Save/Load/New button handlers
- [x] Error dialog implementation
- [ ] Confirmation dialog for unsaved changes before load
