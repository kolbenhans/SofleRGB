# Development Notes

| Keymap | Lighting system |
|---|---|
| `customLights` | Keycode-derived (below) |
| `comboRGB`, `keyColors` | WebGUI-driven, EEPROM-backed — see [webgui-usage.md](webgui-usage.md) |
| `vial`, `signalrgb` | Stock effects only |

## customLights — keycode-derived lighting

`sofle_rgb/keymaps/customLights/dynamicLights.c` + `rgb_matrix_user.inc`. Registered as RGB Matrix effect `dynamic_lights`, default mode at boot, re-activatable via `USER01` keycode.

Colors keys by function, read from the active Vial keymap — no manual per-key setup. Cache rebuilds only on layer/brightness/lock-state change or Vial keymap edit (FNV-1a checksum, checked 1×/sec).

To change which function gets which color, edit `key_color_rules[]` in `dynamicLights.c` directly (keycode → color mapping) and reflash — there's no runtime/GUI config for this build, that's what `comboRGB`/`keyColors` are for.

`g_led_config.matrix_co[row][col]` gives the LED for a matrix position; must match the LAYOUT definition (Sofle RGB right half: col 0 = outer, col 5 = inner) or reactive effects (Splash etc.) fire from the wrong spot. `g_led_config.point[led].x/y` drives the coordinate-based startup comet.

Split: master rebuilds+syncs the color cache to slave via 3× 24-byte RPC packets, only on rebuild. Each half's own Vial EEPROM stays independent — only lighting state syncs, not EEPROM.

## Split Transaction IDs

```c
USER_SYNC_LIGHTS_A/B/C       // color IDs, 24 each
USER_DYNAMIC_LIGHTS_STARTUP  // startup comet trigger
USER_SYNC_RGB_DIRECT         // viz_frame slave-half push
USER_ENTRY_WAVE_STARTUP      // viz_frame entry animation trigger
```

`SPLIT_TRANSACTION_IDS_USER` is **keymap-level**, defined in each keymap's own `config.h` — not board-level. Each keymap registers different RPC handlers; putting it in the board `config.h` breaks every other keymap's build (`undeclared identifier` at link time).

## viz_frame internals

`entry_wave.c` + `rgb_matrix_user.inc`, in `customLights`/`comboRGB`. `g_direct_mode_colors[]` fills via Raw HID on master only.

- `RPC_M2S_BUFFER_SIZE` default (32B) is too small for the half-buffer push (36 LEDs × 3B = 108B) — `transaction_rpc_send()` silently fails over the limit, no error. Set to 112.
- Sync must branch on `is_keyboard_left()`, not a fixed offset — hardcoding "right half = remote" breaks when the right half is master.
