# QMK Userspace

This is the QMK Userspace for my custom keyboard keymaps.

## Supported Keyboards

### Corne v3 (crkbd)
- Split keyboard with wired connection
- 3x6 key matrix with 3 thumb keys per side

### Charybdis Mini (BastardKB)
- Splinktegrated controller, compatible with Splinky v3
- 3x6 layout with five thumb keys and a right-hand trackball
- [Keymap, build commands, and tests](keyboards/bastardkb/charybdis/3x6/keymaps/hearter/README.md)
- `flash_latest.sh` remains Corne-specific

## Flashing Instructions

Prerequisites:
- GitHub CLI (`gh`) installed and authenticated (`gh auth login`) or `GH_TOKEN` set
  - Arch: `sudo pacman -S github-cli`
- QMK CLI (`qmk`) installed for Corne flashing
  - Arch: `sudo pacman -S qmk`

Use the `flash_latest.sh` script to flash your keyboard:

```bash
# Flash Corne (default)
./flash_latest.sh

# Flash with latest successful build
./flash_latest.sh --latest-successful
```

## Building Firmware

Firmware is automatically built by GitHub Actions when changes are pushed to the repository.

You can read more about compiling QMK firmware on the official docs:
- [QMK Documentation](https://docs.qmk.fm/#/newbs_getting_started)
