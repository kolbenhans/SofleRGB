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
