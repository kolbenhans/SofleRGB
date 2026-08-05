# Build Guide

**Windows:** install [QMK MSYS](https://msys.qmk.fm/) — includes git, QMK CLI, toolchain, nothing else needed.
**Linux/macOS:** `pip install qmk`, then [QMK Getting Started](https://docs.qmk.fm/newbs_getting_started) for the toolchain.

```bash
# vial-qmk
git clone --depth 1 https://github.com/vial-kb/vial-qmk ~/projects/vial-qmk
cd ~/projects/vial-qmk
git submodule update --init --recursive --depth 1
python3 -m pip install -r requirements.txt

# this repo
git clone --recursive https://github.com/kolbenhans/SofleRGB.git ~/projects/SofleRGB

# link keyboard source into vial-qmk
ln -s ~/projects/SofleRGB/sofle_rgb ~/projects/vial-qmk/keyboards/sofle_rgb
```

Windows without QMK MSYS, use `mklink /D` instead of `ln -s` (needs admin).

## signalrgb keymap only

```bash
cd ~/projects/vial-qmk
git submodule add https://github.com/SRGBmods/QMK_Community_Module modules/signalrgb
git submodule update --init --recursive -- modules/signalrgb
```
Must be a real submodule, not a symlink — QMK's module lookup silently fails on symlinked module folders (Python 3.13+).

## keyColors / dynamicLights / comboRGB keymaps only

These report live layer/key state to [KeyPeek](https://github.com/srwi/keypeek) (optional companion desktop app):

```bash
cd ~/projects/vial-qmk
git submodule add https://github.com/srwi/qmk-modules.git modules/srwi
git submodule update --init --recursive -- modules/srwi
```

## Build

```bash
cd ~/projects/vial-qmk
qmk compile -kb sofle_rgb -km <vial|keyColors|comboRGB|signalrgb|dynamicLights>
```

Output: `.build/sofle_rgb_<keymap>.uf2`

Already in bootloader mode? `qmk flash` instead of `qmk compile` skips the copy step. Split keyboard — flash **both halves individually**, see [Flashing Guide](flashing.md).

## Hardware

RP2040, UF2 bootloader, 72 RGB LEDs total (36/half). Each keymap defines its own `SPLIT_TRANSACTION_IDS_USER` in its own `config.h` — see [Development Notes](development.md).
