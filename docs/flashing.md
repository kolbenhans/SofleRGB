# Flashing Guide

## Linux: USB permissions

See [QMK's udev rules guide](https://docs.qmk.fm/faq_build#linux-udev-rules).

## Build & flash

```bash
qmk compile -kb sofle_rgb -km <keymap>
```
Copy `.build/sofle_rgb_<keymap>.uf2` onto the keyboard's UF2 drive. **Both halves, individually.**

## Entering bootloader mode

- Double-press the reset button on the PCB, or
- Press a key bound to `QK_BOOT` (assignable in Vial)

Keyboard appears as a USB drive — drop the `.uf2` file on it.
