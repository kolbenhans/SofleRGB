RGB_MATRIX_EFFECT(dynamic_lights)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// ─── Dynamic Lights ───────────────────────────────────────────────────────────
// The viz_frame effect (entry-wave + g_direct_mode_colors relay) that used to
// live in this file too now comes from the kolbenhans/viz_relay module's own
// rgb_matrix_module.inc.

extern void dynamic_lights_render(uint8_t led_min, uint8_t led_max);

static bool dynamic_lights(effect_params_t *params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    dynamic_lights_render(led_min, led_max);
    return rgb_matrix_check_finished_leds(led_max);
}

#endif
