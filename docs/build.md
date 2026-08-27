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

## Community Modules

`keyColors` / `dynamicLights` / `comboRGB` all use QMK Community Modules
instead of vendoring their own lighting code — the modules must exist under
`vial-qmk/modules/` before compiling, same install step for each:

```bash
cd ~/projects/vial-qmk
git submodule add https://github.com/srwi/qmk-modules.git modules/srwi
git submodule add https://github.com/kolbenhans/qmk-modules.git modules/kolbenhans
git submodule update --init --recursive
```

| Module | Provides | Used by |
|---|---|---|
| `srwi/keypeek_layer_notify` | live layer/key state → [KeyPeek](https://github.com/srwi/keypeek) (optional companion app) | `keyColors`, `dynamicLights`, `comboRGB` |
| `kolbenhans/key_colors` | WebGUI per-key/per-layer color engine (EEPROM-backed, delta-synced) | `keyColors`, `comboRGB` |
| `kolbenhans/viz_relay` | entry-wave transition + `g_direct_mode_colors` relay for the Python viz-frame-tools pipeline | `dynamicLights`, `comboRGB` |

`vial` and `default` need none of these.

## Build

```bash
cd ~/projects/vial-qmk
qmk compile -kb sofle_rgb -km <vial|keyColors|comboRGB|dynamicLights>
```

Output: `.build/sofle_rgb_<keymap>.uf2`

Already in bootloader mode? `qmk flash` instead of `qmk compile` skips the copy step. Split keyboard — flash **both halves individually**, see [Flashing Guide](flashing.md).

## Hardware

RP2040, UF2 bootloader, 72 RGB LEDs total (36/half). Split-sync RPC IDs are now mostly module-owned (`kolbenhans/key_colors`, `kolbenhans/viz_relay`) rather than keymap-level — see [Development Notes](development.md) for the current split and a gotcha with this vial-qmk checkout's version.
