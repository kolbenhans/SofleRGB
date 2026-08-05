# SofleRGB

Vial-QMK firmware for the Sofle RGB — five keymaps, pick one, build it, flash it.

Don't want to build it yourself? Prebuilt `.uf2` files for every keymap are in [`firmware/`](firmware/) — skip straight to [Flashing](docs/flashing.md).

## Which keymap?

| Keymap | What it is |
|---|---|
| `vial` | Stock Vial, no custom lighting |
| `keycolors` | same webgui colors, no visualizer/python — leanest |
| `comboRGB` | Pick your own key colors via [browser WebGUI](docs/webgui-usage.md) + audio visualizer |
| `signalrgb` | Stock Vial + [SignalRGB](https://signalrgb.com/) PC sync |
| `customLights` | Keys auto-color by function, no setup. Includes [audio visualizer](docs/viz_frame.md) |

unsure? `keycolors` for just picking colors. `comborgb` if you also want the visualizer.

## quick start

```bash
git clone --depth 1 https://github.com/vial-kb/vial-qmk ~/projects/vial-qmk
cd ~/projects/vial-qmk
git submodule update --init --recursive --depth 1
python3 -m pip install -r requirements.txt

git clone --recursive https://github.com/kolbenhans/soflergb.git ~/projects/soflergb
ln -s ~/projects/soflergb/sofle_rgb ~/projects/vial-qmk/keyboards/sofle_rgb

cd ~/projects/vial-qmk
qmk compile -kb sofle_rgb -km keycolors   # or: vial, signalrgb, customlights, comborgb
```

copy `.build/sofle_rgb_<keymap>.uf2` onto the keyboard's uf2 drive — **both halves separately**. details: [build guide](docs/build.md), [flashing guide](docs/flashing.md).

**`keycolors` / `customlights` / `comborgb`**: the lighting effect is default at boot, but custom effects don't show up in Vial's effect list — if it ever switches off (another effect picked, EEPROM not fresh), bind `User 1` to a key in Vial (**User** tab → drag onto a key) to bring it back.

**`signalrgb` only**, one extra step:
```bash
cd ~/projects/vial-qmk
git submodule add https://github.com/srgbmods/qmk_community_module modules/signalrgb
git submodule update --init --recursive -- modules/signalrgb
```
(must be a real submodule, not a symlink — qmk's module lookup won't find symlinked module folders.)

**`comborgb` / `keycolors`**: open **https://webgui.212-227-193-242.sslip.io/** in chrome/edge, connect, click keys, pick colors, save. no install needed. run it locally instead: [webgui usage](docs/webgui-usage.md).

## contents

- `sofle_rgb/` — keyboard source
- `webgui/` — browser color picker (`comborgb`/`keycolors`)
- `python/` — audio visualizer gui (`customlights`/`comborgb`)

## docs

[build](docs/build.md) · [flashing](docs/flashing.md) · [development](docs/development.md) · [audio visualizer](docs/viz_frame.md) · [webgui usage](docs/webgui-usage.md)
