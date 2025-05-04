# QMK Userspace

This is the QMK Userspace for my custom keyboard keymaps.

## Supported Keyboards

### 1. Bastard Keyboards Charybdis 3x6
- Split keyboard with integrated trackball
- 3x6 key matrix with 3 thumb keys per side

### 2. Corne v3 (crkbd)
- Split keyboard with wired connection
- 3x6 key matrix with 3 thumb keys per side

## Flashing Instructions

Use the `flash_latest.sh` script to flash your keyboard:

```bash
# Flash Charybdis (default)
./flash_latest.sh 

# Flash Corne
./flash_latest.sh corne

# Flash with latest successful build
./flash_latest.sh corne --latest-successful
```

## Building Firmware

Firmware is automatically built by GitHub Actions when changes are pushed to the repository.

You can read more about compiling QMK firmware on the official docs:
- [Bastard Keyboards Documentation](https://docs.bastardkb.com/fw/compile-firmware.html)
- [QMK Documentation](https://docs.qmk.fm/#/newbs_getting_started)