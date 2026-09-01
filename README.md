# SofleRGB

Custom **Vial-QMK firmware for the Sofle RGB** with multiple keymap variants, customizable per-key RGB lighting, and an optional audio visualizer.

Choose the configuration that fits your setup:

* **`vial`** — standard Vial firmware
* **`keyColors`** — per-key RGB colors configurable through a browser-based WebGUI
* **`comboRGB`** — per-key RGB lighting combined with an audio visualizer

Precompiled `.uf2` firmware files are included, so you can use the firmware without setting up a QMK build environment.

---

## Keymaps

| Keymap      | Description                                             |
| ----------- | ------------------------------------------------------- |
| `vial`      | Standard Vial configuration without custom RGB features |
| `keyColors` | Custom per-key RGB colors configured through the WebGUI |
| `comboRGB`  | `keyColors` plus an audio-reactive visualizer           |

The `keyColors` and `comboRGB` variants use additional QMK modules. The required modules are documented in [`qmk-modules`](https://github.com/kolbenhans/qmk-modules).

---

## Prebuilt Firmware

Don't want to build the firmware yourself?

Precompiled `.uf2` files are available in the [`firmware/`](firmware/) directory.

The prebuilt `keyColors` and `comboRGB` firmware already include the required `keypeek` module, so no additional module setup is required when using the precompiled firmware.

See the [Flashing Guide](docs/flashing.md) for instructions on flashing the firmware to both halves of the keyboard.

---

## Features

### Vial

The `vial` keymap provides a standard Vial-QMK configuration for the Sofle RGB.

Use the [Vial](https://get.vial.today/) application to configure keymaps, layers, and other supported keyboard settings without recompiling the firmware.

### Per-Key RGB

The `keyColors` keymap adds individual RGB color control for the keyboard's LEDs.

Instead of relying exclusively on predefined RGB Matrix effects, each key can be assigned its own color.

Colors can be configured through the browser-based WebGUI.

See the [WebGUI Usage Guide](docs/webgui.md) for details.

### Audio Visualizer

The `comboRGB` keymap combines the per-key RGB system with an audio-reactive visualizer.

The visualizer can drive the keyboard's RGB LEDs based on audio data received from the host system.

See the [Audio Visualizer documentation](docs/audio_visualizer.md) for setup and usage instructions.

---

## Quick Start

The easiest way to build the firmware is to use the Vial-QMK repository.

### 1. Clone Vial-QMK

```bash
git clone --depth 1 https://github.com/vial-kb/vial-qmk ~/projects/vial-qmk
cd ~/projects/vial-qmk

git submodule update --init --recursive --depth 1
python3 -m pip install -r requirements.txt
```

### 2. Clone this repository

```bash
git clone --recursive https://github.com/kolbenhans/SofleRGB.git ~/projects/soflergb
```

### 3. Link the keyboard source

```bash
ln -s ~/projects/soflergb/sofle_rgb \
      ~/projects/vial-qmk/keyboards/sofle_rgb
```

### 4. Build the firmware

```bash
cd ~/projects/vial-qmk

qmk compile -kb sofle_rgb -km keyColors
```

Replace `keyColors` with the desired keymap:

```text
vial
keyColors
comboRGB
```

For example:

```bash
qmk compile -kb sofle_rgb -km comboRGB
```

The resulting firmware will be placed in:

```text
.build/sofle_rgb_<keymap>.uf2
```

See the [Build Guide](docs/build.md) for more detailed information.

---

## Flashing

The Sofle RGB uses UF2 bootloader flashing.

After compiling, copy the generated `.uf2` file to the keyboard's UF2 drive.

**Both halves need to be flashed separately.**

For detailed instructions, including entering bootloader mode and flashing each half, see the [Flashing Guide](docs/flashing.md).

---

## WebGUI

The `keyColors` and `comboRGB` keymaps provide a browser-based interface for configuring per-key RGB colors.

The WebGUI allows you to customize the individual LEDs without modifying the firmware source code or recompiling the keyboard.

See the [WebGUI Usage Guide](docs/webgui.md).

---

## Audio Visualizer

The `comboRGB` keymap adds an audio visualizer to the per-key RGB system.

The keyboard receives audio data from a host application and uses that data to drive the RGB animation.

This allows the Sofle RGB to react to music and other system audio in real time.

The audio visualizer consists of two parts:

```text
Host system
    │
    │ audio / FFT data
    ▼
Host visualizer
    │
    │ keyboard communication
    ▼
Sofle RGB
    │
    ▼
RGB Matrix / per-key LEDs
```

For the complete setup and host-side requirements, see the [Audio Visualizer documentation](docs/audio_visualizer.md).

---

## Repository Structure

```text
SofleRGB/
├── docs/
│   ├── build.md
│   ├── flashing.md
│   ├── audio_visualizer.md
│   └── webgui.md
│
├── firmware/
│   └── *.uf2
│
└── sofle_rgb/
    └── keyboard source
```

### `sofle_rgb/`

Contains the Sofle RGB keyboard definition and the custom firmware implementation.

### `firmware/`

Contains precompiled UF2 firmware images for the available keymaps.

### `docs/`

Contains detailed documentation for building, flashing, configuring the WebGUI, and using the audio visualizer.

---

## Building From Source

This repository is designed to be used together with [Vial-QMK](https://github.com/vial-kb/vial-qmk).

The keyboard source is kept separate from the QMK tree and linked into the appropriate `keyboards/` directory during development.

This makes it possible to keep the custom keyboard implementation in its own repository while using the complete Vial-QMK build system.

For development and build details, see the [Build Guide](docs/build.md).

---

## QMK Modules

Some functionality used by the custom keymaps is provided through additional QMK modules.

The required modules are maintained separately:

[**kolbenhans/qmk-modules**](https://github.com/kolbenhans/qmk-modules)

The precompiled `keyColors` and `comboRGB` firmware already contains the required module functionality, so modules only need to be configured when building the firmware yourself.

---

## Documentation

| Topic            | Documentation                                |
| ---------------- | -------------------------------------------- |
| Building         | [Build Guide](docs/build.md)                 |
| Flashing         | [Flashing Guide](docs/flashing.md)           |
| WebGUI           | [WebGUI Usage](docs/webgui.md)               |
| Audio Visualizer | [Audio Visualizer](docs/audio_visualizer.md) |

---

## License

This project is based on the Sofle RGB keyboard firmware and the Vial-QMK ecosystem.

See the repository's [`LICENSE`](LICENSE) file for the applicable license information.
