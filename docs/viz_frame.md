# viz_frame — Audio Visualizer

In `dynamicLights` and `comboRGB` only. Rendering (FFT, palette, frames) runs in Python on the host; firmware just displays the colors it receives over Raw HID.

```bash
git submodule update --init --recursive
pip install -r python/requirements.txt
python3 python/viz_gui.py
```

Auto-detects the keyboard, switches it into `viz_frame` mode automatically when you pick a render mode. That's the only way to activate it — custom effects don't show up in Vial's effect dropdown.

Split note: colors only reach the master half over USB; a dedicated RPC (`USER_SYNC_RGB_DIRECT`) pushes the slave's half across, regardless of which side is master.

More render modes, palettes, Ambient/WPWatch/Color-Shot: see the [viz-frame-tools](https://github.com/kolbenhans/viz-frame-tools) repo README.
