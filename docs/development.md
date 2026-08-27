# Development Notes

| Keymap | Lighting system |
|---|---|
| `dynamicLights` | Keycode-derived (below) + `kolbenhans/viz_relay` module |
| `comboRGB` | `kolbenhans/key_colors` + `kolbenhans/viz_relay` modules — see [webgui-usage.md](webgui-usage.md) |
| `keyColors` | `kolbenhans/key_colors` module only — see [webgui-usage.md](webgui-usage.md) |
| `vial` | Stock effects only |

`key_colors` and `viz_relay` used to be vendored per-keymap (`key_colors.c`,
`dynamic_lights.c`, `entry_wave.c`) — they're now real QMK Community Modules
(`github.com/kolbenhans/qmk-modules`), consumed via each keymap's
`keymap.json` `"modules"` array. Only the Raw HID opcode dispatch
(`raw_hid_receive_kb`) and any board/keymap-specific sizing stayed
keymap-level, since modules can't hook Raw HID receive — see each keymap's
`*_hid.c`/`rgb_glue.c`/`viz_glue.c` glue file.

## dynamicLights — keycode-derived lighting

`sofle_rgb/keymaps/dynamicLights/dynamicLights.c` + `rgb_matrix_user.inc`. Registered as RGB Matrix effect `dynamic_lights`, default mode at boot, re-activatable via `USER01` keycode.

Colors keys by function, read from the active Vial keymap — no manual per-key setup. Cache rebuilds only on layer/brightness/lock-state change or Vial keymap edit (FNV-1a checksum, checked 1×/sec).

To change which function gets which color, edit `key_color_rules[]` in `dynamicLights.c` directly (keycode → color mapping) and reflash — there's no runtime/GUI config for this build, that's what `comboRGB`/`keyColors` are for.

`g_led_config.matrix_co[row][col]` gives the LED for a matrix position; must match the LAYOUT definition (Sofle RGB right half: col 0 = outer, col 5 = inner) or reactive effects (Splash etc.) fire from the wrong spot. `g_led_config.point[led].x/y` drives the coordinate-based startup comet.

Split: master rebuilds+syncs the color cache to slave via 3× 24-byte RPC packets, only on rebuild (this keymap's own engine — untouched by the module split below). Each half's own Vial EEPROM stays independent — only lighting state syncs, not EEPROM.

## Split Transaction IDs

`dynamicLights`'s own keycode-derived engine (unaffected by the module split):
```c
USER_SYNC_LIGHTS_A/B/C       // color IDs, 24 each
USER_DYNAMIC_LIGHTS_STARTUP  // startup comet trigger
```

`kolbenhans/key_colors` module (used by `keyColors`/`comboRGB`) — **delta-sync**, not a full-cache broadcast: each color/blink/lock-flag change forwards just that one small host-sized chunk to the other half, which applies it directly to its own tables and resolves its own render cache (needs `SPLIT_LED_STATE_ENABLE`, so lock-gated LEDs read correct state on both halves):
```c
KEY_COLORS_COLORS_DELTA       // one SET_KEY_COLORS_CHUNK, mirrored
KEY_COLORS_BLINK_DELTA        // one SET_BLINK_COLORS_CHUNK, mirrored
KEY_COLORS_LOCK_FLAGS_DELTA   // one (layer, led, flags) triple, mirrored
KEY_COLORS_COMMIT             // "persist your own tables to your own EEPROM"
KEY_COLORS_STARTUP            // startup comet trigger
```

`kolbenhans/viz_relay` module (used by `dynamicLights`/`comboRGB`):
```c
VIZ_RELAY_SYNC_RGB_DIRECT     // viz_frame slave-half push
VIZ_RELAY_ENTRY_WAVE_STARTUP  // viz_frame entry animation trigger
```

**Gotcha with this vial-qmk checkout**: it predates upstream QMK's
`SPLIT_TRANSACTION_IDS_MODULE_*` mechanism (modules declaring their own RPC
IDs). Both modules' `config.h` declare it anyway for forward-compat, but this
fork silently ignores it — so every keymap consuming either module must
*also* list that module's RPC ID names in its own `SPLIT_TRANSACTION_IDS_USER`
(see each migrated keymap's `config.h` for the exact list; the module's `.c`
only needs the names to exist as enum constants somewhere, doesn't care
which `config.h` declared them). Drop this shim once vial-qmk catches up.

`SPLIT_TRANSACTION_IDS_USER` is still **keymap-level**, defined in each
keymap's own `config.h` — not board-level, same reasoning as before (each
keymap pulls in a different module combination).

## viz_frame internals

Now in `kolbenhans/viz_relay` module (`viz_relay.c` + its own
`rgb_matrix_module.inc`), not vendored per-keymap. `g_direct_mode_colors[]`
fills via Raw HID FASTSET on master only; the module's `housekeeping_task_viz_relay()`
pushes the slave's half over the split link.

- `RPC_M2S_BUFFER_SIZE` default (32B) is too small for the half-buffer push (36 LEDs × 3B = 108B) — `transaction_rpc_send()` silently fails over the limit, no error. Set to 112 (board-level `config.h`, not the module — it's genuinely per-board).
- Sync must branch on `is_keyboard_left()`, not a fixed offset — hardcoding "right half = remote" breaks when the right half is master.
- Mode-switch dispatch (Raw HID `0x02/0xA3` → `viz_relay_trigger()`, `0x02/0xA4` → back to whichever lighting engine that keymap uses) stays keymap-level (`viz_glue.c` in `dynamicLights`, `rgb_glue.c` in `comboRGB`) — modules can't hook `raw_hid_receive_kb`.
