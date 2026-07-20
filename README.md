# SofleRGB

Blank Vial QMK firmware source for the "sofle_panda" split keyboard (Pandakb Sofle, RP2040, VialRGB/OLED).

Stock VIA/Vial keymap with the full standard RGB Matrix effect set and OLED animations. No custom RGB effects, no host-side lighting tools — just the keyboard source as dropped into a `vial-qmk` tree.

## Contents

- `sofle_panda/` — keyboard source (drop into `vial-qmk/keyboards/sofle_panda`)
- `modules/signalrgb/` — [SignalRGB](https://signalrgb.com/) community module (see below)

## Build

```bash
git clone --depth 1 https://github.com/vial-kb/vial-qmk ~/projects/vial-qmk
cd ~/projects/vial-qmk && git submodule update --init --recursive --depth 1

ln -s ~/projects/SofleRGB/sofle_panda ~/projects/vial-qmk/keyboards/sofle_panda

mkdir -p ~/projects/vial-qmk/modules
ln -s ~/projects/SofleRGB/modules/signalrgb ~/projects/vial-qmk/modules/signalrgb

qmk compile -kb sofle_panda -km vial
```

Output: `.build/sofle_panda_vial.uf2`

## Flashing

Split keyboard — flash **both halves individually**, double-tap reset into the RP2040 UF2 bootloader.

## SignalRGB

Vendored copy of [SRGBmods/QMK_Community_Module](https://github.com/SRGBmods/QMK_Community_Module) (early WIP upstream), minus its `config.h` — that file `#undef`s most stock `ENABLE_RGB_MATRIX_*` effects to save flash, which conflicts with this repo's "keep the full stock effect set" goal. Enabled via `keymaps/vial/keymap.json` (`"modules": ["signalrgb"]`).

The module streams arbitrary per-LED colors over raw HID to the master half only. Since stock `SPLIT_RGB_MATRIX` only syncs mode/hsv/speed (not raw pixel data), `keymaps/vial/signalrgb_split.c` mirrors the remote half's colors to the slave over its own RPC transaction (`USER_SYNC_SIGNALRGB`) — same pattern the original dynamic-lightning firmware used for viz_frame.

`modules/signalrgb/qmk_version.h` has placeholder version bytes. From `~/projects/vial-qmk/modules/signalrgb/` (the symlinked path, so `gen-version.sh`'s `cd ../../` lands in the actual vial-qmk checkout), run `./gen-version.sh` to stamp the real version before relying on `GET_QMK_VERSION`.
