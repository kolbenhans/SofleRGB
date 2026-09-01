# SofleRGB

Vial-QMK firmware for the Sofle RGB — three keymaps, pick one, build it, flash it.

Don't want to build it yourself? Prebuilt `.uf2` files are in [`firmware/`](firmware/) — skip to [Flashing](docs/flashing.md).
The precompiled firmware ..keyColors & ...comboRGB both include the keypeek module, so this will work directly.

## Which keymap?

| Keymap | What it is |
|---|---|
| `vial` | Stock Vial, no custom lighting |
| `keyColors` | Pick your own key colors via [browser WebGUI](docs/webgui-usage.md) |
| `comboRGB` | Same, plus [audio visualizer](docs/audio-visualizer.md) |

## Quick start

```bash
git clone --depth 1 https://github.com/vial-kb/vial-qmk ~/projects/vial-qmk
cd ~/projects/vial-qmk
git submodule update --init --recursive --depth 1
python3 -m pip install -r requirements.txt

git clone --recursive https://github.com/kolbenhans/soflergb.git ~/projects/soflergb
ln -s ~/projects/soflergb/sofle_rgb ~/projects/vial-qmk/keyboards/sofle_rgb

cd ~/projects/vial-qmk
qmk compile -kb sofle_rgb -km keyColors   # or: vial, comboRGB
```

Copy `.build/sofle_rgb_<keymap>.uf2` onto the keyboard's UF2 drive — **both halves separately**. Details: [Build Guide](docs/build.md), [Flashing Guide](docs/flashing.md).

`keyColors`/`comboRGB` need modules — see [kolbenhans/qmk-modules](https://github.com/kolbenhans/qmk-modules).

## Contents

- `sofle_rgb/` — keyboard source

## Docs

[Build](docs/build.md) · [Flashing](docs/flashing.md) · [Audio Visualizer](docs/audio-visualizer.md) · [WebGUI Usage](docs/webgui-usage.md)
