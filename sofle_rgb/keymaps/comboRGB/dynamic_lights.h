#pragma once

#include QMK_KEYBOARD_H

enum sofle_layers {
    _QWERTZ = 0,
    _RAISE,
    _NUMPAD,
    _AMBIENT,
    _GAMING
};

#define KC_CAPSLOCK KC_CAPS

void keyboard_post_init_user(void);
void dynamic_lights_on_mode_enter(void);
void dynamic_lights_render(uint8_t led_min, uint8_t led_max);

// Host-assigned per-key/per-layer colors (WebGUI, Raw HID 0x02/0xA5-0xA7).
void dynamic_lights_set_colors(uint8_t layer, uint8_t led_offset, uint8_t count, const uint8_t *rgb_bytes);
void dynamic_lights_get_colors(uint8_t layer, uint8_t led_offset, uint8_t count, uint8_t *out_rgb_bytes);
void dynamic_lights_commit_colors(void);

// Per-key/per-layer lock-state gating (WebGUI, Raw HID 0x02/0xA8-0xA9) — a
// flagged LED only shows its assigned color while the given lock is active
// (live host_keyboard_led_state(), independent of keymap/keycode content).
#define LOCK_FLAG_NUM  (1 << 0)
#define LOCK_FLAG_CAPS (1 << 1)
#define LOCK_FLAG_SCRL (1 << 2)
void dynamic_lights_set_lock_flags(uint8_t layer, uint8_t led, uint8_t flags);
void dynamic_lights_get_lock_flags(uint8_t layer, uint8_t led_offset, uint8_t count, uint8_t *out_flags);
