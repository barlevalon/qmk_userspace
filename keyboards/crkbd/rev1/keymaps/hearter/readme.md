# Hearter's Corne v3 Keymap

This is a port of my Charybdis keymap adapted for the Corne v3 keyboard.

## Features
- 7 layers (Base, Gaming, Number, Symbol, Navigation, Media, Function)
- Home row mods for comfortable modifier access
- Dedicated navigation and media controls
- Special shortcut keys for common tasks

## Installation
1. Place this directory in your QMK userspace or in the `keyboards/crkbd/keymaps/` directory
2. Flash each half separately with the TRRS cable disconnected:
   - Left: `qmk flash -kb crkbd/rev1 -km hearter -bl avrdude-split-left`
   - Right: `qmk flash -kb crkbd/rev1 -km hearter -bl avrdude-split-right`

The split-left/right bootloaders write `EE_HANDS` side identity to EEPROM.