# Charybdis Mini Hearter keymap

This keymap targets the Charybdis Mini 3x6 with Splinktegrated hardware marked "Compatible with splinky v3".

## Hardware and build target

- QMK target: `bastardkb/charybdis/3x6/elitec`.
- Controller conversion: `CONVERT_TO = rp2040_ce` in `rules.mk`.
- USB connection: right half, selected by `MASTER_RIGHT`.
- Trackball: PMW3360 on the right half, with the inherited rotation and axis inversion.
- Bootloader: RP2040, with a 1000 ms double-reset window and the GP17 status LED.

The Elite-C target name describes the connector pinout. The converter builds RP2040 firmware, not AVR firmware.

[BastardKB's Splinktegrated pin table](https://github.com/Bastardkb/bastardkb.github.io/blob/main/hw/pins.md) documents the physical pin assignments.

## Base thumb keys

Positions run from left to right as displayed in `keymap.c`.

| Position | Tap | Hold |
| --- | --- | --- |
| Left outer | Escape | Media |
| Left middle | Space | Navigation |
| Left inner | Tab | Function |
| Right inner | Enter | Symbols |
| Right outer | Backspace | Numbers |

The Mini has five thumb keys, rather than the Corne's six. Delete is on the Tab position while Navigation is active. Shift+Backspace also retains the existing Delete behavior.

## Layers and controls

- Base has the Corne's QWERTY layout and home-row modifiers.
- Gaming has plain letters, left Control, left Shift, and left thumb Space, Alt, and Escape. Hold Escape for Numbers.
- Numbers and Symbols retain the left-hand keypad arrangement. Symbols uses ordinary Plus, not keypad Plus.
- Navigation has arrows, page navigation, Home, End, and Caps Word.
- Media has playback, volume, mute, and Print Screen on the W position.
- Function has F1 through F12 on the right and RGB matrix controls on the left.
- Pointer retains trackball DPI controls, mouse buttons, drag-scroll, and sniping.

On Base, the top-right outer key sends Super+Ctrl+X on a tap. Holding it for 170 ms presses F9 until release for dictation. Tap followed by hold still activates F9, and other keypresses do not shorten the hold threshold. Tab held with O remains an alternative F9 binding.

On Base, the bottom-right outer key sends Hyper+Space on a single tap. Double-tapping selects Gaming. In Gaming, double-tapping the same key returns to Base.

Holding Z or slash activates Pointer and enables sniping. Pointer thumb buttons are right, left, and middle click on the left half, then middle and left click on the right half. Automatic pointer-layer activation from trackball motion remains disabled.

The ZMK-only Tap, Button, and Mouse layers are not copied. Trackball controls remain on the existing Pointer layer.

## Home-row modifiers

| Hand | Keys | Hold modifiers |
| --- | --- | --- |
| Left | A, S, D, F | Control, Alt, GUI, Shift |
| Right | J, K, L, Quote | Shift, GUI, left Alt, Control |

Timing matches the wired Corne's current configuration:

| Setting | Value |
| --- | --- |
| `TAPPING_TERM` | 170 ms |
| `QUICK_TAP_TERM` | 100 ms, disabled for the dictation key |
| `FLOW_TAP_TERM` | 150 ms |
| `RETRO_TAPPING_PER_KEY` | Enabled except for pointer holds and dictation |
| `PERMISSIVE_HOLD_PER_KEY` | Enabled except for dictation |
| `CHORDAL_HOLD` | Enabled |

Chordal Hold protects same-hand home-row rolls. Layer-taps are exempt so thumb layers and pointer controls still support same-hand chords. Flow Tap uses QMK's default key filter, except Z and slash pointer holds remain available immediately after typing. Retro tapping excludes those two keys so using the trackball does not type a character on release.

The retired per-finger timing offsets and immediate Shift or GUI hold callbacks remain absent.

## Build and test commands

Run these commands from the userspace root with QMK configured:

```sh
qmk compile -kb bastardkb/charybdis/3x6/elitec -km hearter
make test-charybdis
```

The build produces `bastardkb_charybdis_3x6_elitec_hearter.uf2` in the userspace root. Both halves use the same file.

The native tests use QMK's tap-hold engine, generated matrix layout, and this keymap. They cover home-row rolls, modifier chords, Flow Tap, retro tapping, pointer-layer access, function keys, Delete, and gaming tap dances. The trackball sensor setter is a test stub. Physical split transport, trackball movement, and LEDs still require checks on the keyboard.

`flash_latest.sh` is Corne-specific. It does not flash this board.
