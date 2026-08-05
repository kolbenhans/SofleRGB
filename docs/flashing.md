# Flashing Guide

## Linux: USB permissions

```bash
sudo cp ~/projects/vial-qmk/util/udev/50-qmk.rules /etc/udev/rules.d/
echo 'KERNEL=="hidraw*", ATTRS{idVendor}=="fc32", ATTRS{idProduct}=="0287", MODE="0666"' | sudo tee /etc/udev/rules.d/99-sofle-rgb.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```
Reconnect the keyboard after.

## Build & flash

```bash
qmk compile -kb sofle_rgb -km <keymap>
```
Copy `.build/sofle_rgb_<keymap>.uf2` onto the keyboard's UF2 drive. **Both halves, individually.**

## Entering bootloader mode

- Double-press the reset button on the PCB, or
- Press a key bound to `QK_BOOT` (assignable in Vial)

Keyboard appears as a USB drive — drop the `.uf2` file on it.
