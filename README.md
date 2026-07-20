# SofleRGB

Blank Vial QMK firmware source for the "sofle_panda" split keyboard (Pandakb Sofle, RP2040, VialRGB/OLED).

Stock VIA/Vial keymap with the full standard RGB Matrix effect set and OLED animations. No custom RGB effects, no host-side lighting tools — just the keyboard source as dropped into a `vial-qmk` tree.

## Contents

- `sofle_panda/` — keyboard source (drop into `vial-qmk/keyboards/sofle_panda`)

## Build

```bash
git clone --depth 1 https://github.com/vial-kb/vial-qmk ~/projects/vial-qmk
cd ~/projects/vial-qmk && git submodule update --init --recursive --depth 1

ln -s ~/projects/SofleRGB/sofle_panda ~/projects/vial-qmk/keyboards/sofle_panda

qmk compile -kb sofle_panda -km vial
```

Output: `.build/sofle_panda_vial.uf2`

## Flashing

Split keyboard — flash **both halves individually**, double-tap reset into the RP2040 UF2 bootloader.
