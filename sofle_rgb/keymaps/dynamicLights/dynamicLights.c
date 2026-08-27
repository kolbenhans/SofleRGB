#include QMK_KEYBOARD_H
#include "transactions.h"
#include "dynamicLights.h"
#include <string.h>

#if defined(RGB_MATRIX_ENABLE)

// ─── Hardware constants ───────────────────────────────────────────────────────

#define KEY_ROWS      10
#define KEY_COLS      6
#define KEY_LED_COUNT 72

// ─── Startup animation ───────────────────────────────────────────────────────

#define STARTUP_STEP_MS    10
#define STARTUP_TAIL_WIDTH 84

// ─── Cache ───────────────────────────────────────────────────────────────────

#define CACHE_INVALID_COLOR      0xFF
#define KEYMAP_CHECK_INTERVAL_MS 1000

// ─── Layer helpers ────────────────────────────────────────────────────────────

#define LAYER_ALL     0xFFFFFFFFUL
#define LAYER_MASK(n) (1UL << (n))
#define L_ALL         LAYER_ALL
#define L(n)          LAYER_MASK(n)
#define L_RANGE(lo, hi) (((2UL << (hi)) - 1UL) & ~((1UL << (lo)) - 1UL))

// ─── Color palette ────────────────────────────────────────────────────────────

enum color_id {
    CLR_OFF = 0,
    CLR_RED,
    CLR_LIGHTRED,
    CLR_YELLOW,
    CLR_PINK,
    CLR_GREEN,
    CLR_DARKGREEN,
    CLR_MINT,
    CLR_LEMONGREEN,
    CLR_BLUE,
    CLR_LIGHTBLUE,
    CLR_CYAN,
    CLR_PURPLE,
    CLR_ROSE,
    CLR_WHITE,
    CLR_ORANGE,
    CLR_LIGHTORANGE,
    CLR_DARKORANGE,
    CLR_GREY,
    CLR_LAYERSW,  // Color for all layer switch keys (MO(x),TO(x),TT(x), ...)
    CLR_HMR_GUI,
    CLR_HMR_ALT,
    CLR_HMR_CTL,
    CLR_HMR_SFT,
    FX_BLINK_EEPROM,
    FX_BLINK_BOOT,
    FX_BLINK_REBOOT,

    CLR_COUNT
};

typedef struct { uint8_t r, g, b; } rgb_color_t;

static const rgb_color_t color_palette[] = {
    [CLR_OFF]         = {   0,   0,   0 },
    [CLR_RED]         = { 255,   0,   0 },
    [CLR_LIGHTRED]    = { 255,  10,  10 },
    [CLR_YELLOW]      = { 255, 210,   0 },
    [CLR_PINK]        = { 255,   0, 128 },
    [CLR_GREEN]       = {   0, 255,   0 },
    [CLR_DARKGREEN]   = {   0,  40,   0 },
    [CLR_MINT]        = {  51, 255, 153 },
    [CLR_LEMONGREEN]  = { 174, 255,  0 },
    [CLR_BLUE]        = {   0,  80, 255 },
    [CLR_LIGHTBLUE]   = {   0,  90, 255 },
    [CLR_CYAN]        = {   0, 255, 255 },
    [CLR_PURPLE]      = {  76,   0, 153 },
    [CLR_ROSE]        = { 255,  25, 120 },
    [CLR_WHITE]       = { 255, 255, 255 },
    [CLR_ORANGE]      = { 255,  60,   0 },
    [CLR_LIGHTORANGE] = { 255, 204, 153 },
    [CLR_DARKORANGE]  = {  64,  12,   0 },
    [CLR_GREY]        = {  24,  24,  24 },
    [CLR_LAYERSW]     = {  24,  24,  24 },  // Layer Switch keys MO(x), TO(x), TT(x), ...
    [CLR_HMR_GUI]     = { 255, 210,   0 },  // yellow — LGUI/RGUI
    [CLR_HMR_ALT]     = {   0, 255,   0 },  // green  — LALT/RALT
    [CLR_HMR_CTL]     = {   0,  80, 255 },  // blue   — LCTL/RCTL
    [CLR_HMR_SFT]     = { 255,   0,   0 },  // red    — LSFT/RSFT
};

// ─── Color rules ─────────────────────────────────────────────────────────────

typedef struct {
    uint16_t keycode;
    uint8_t  color_id;
    uint32_t layer_mask;
} key_color_rule_t;

static const key_color_rule_t key_color_rules[] = {
    // Action keys — always visible on all layers
    { KC_ENT,        CLR_CYAN,       L_ALL },
    { KC_SPC,        CLR_PURPLE,     L_ALL },
    { KC_LEFT,       CLR_RED,        L_ALL },
    { KC_DOWN,       CLR_BLUE,       L_ALL },
    { KC_UP,         CLR_GREEN,      L_ALL },
    { KC_RGHT,       CLR_YELLOW,     L_ALL },
    { KC_MUTE,       CLR_RED,        L_ALL },
    { KC_VOLU,       CLR_LEMONGREEN, L_ALL },
    { KC_VOLD,       CLR_LIGHTRED,   L_ALL },
    { C(S(KC_TAB)),  CLR_PINK,       L_ALL },
    { C(KC_TAB),     CLR_ORANGE,     L_ALL },
    { TD(3),         CLR_DARKORANGE, L_ALL },
    { KC_BSPC,       CLR_PINK,       L_RANGE(1, 4) },
    { KC_DEL,        CLR_PINK,       L_RANGE(1, 4) },
    // Numpad
    { KC_P0,   CLR_ORANGE, L_RANGE(1, 4) },
    { KC_P1,   CLR_ORANGE, L_RANGE(1, 4) },
    { KC_P2,   CLR_ORANGE, L_RANGE(1, 4) },
    { KC_P3,   CLR_ORANGE, L_RANGE(1, 4) },
    { KC_P4,   CLR_ORANGE, L_RANGE(1, 4) },
    { KC_P5,   CLR_PINK,   L_RANGE(1, 4) },
    { KC_P6,   CLR_ORANGE, L_RANGE(1, 4) },
    { KC_P7,   CLR_ORANGE, L_RANGE(1, 4) },
    { KC_P8,   CLR_ORANGE, L_RANGE(1, 4) },
    { KC_P9,   CLR_ORANGE, L_RANGE(1, 4) },
    { KC_PPLS, CLR_GREEN,  L_RANGE(1, 4) },
    { KC_PAST, CLR_GREEN,  L_RANGE(1, 4) },
    { KC_PMNS, CLR_RED,    L_RANGE(1, 4) },
    { KC_PSLS, CLR_RED,    L_RANGE(1, 4) },
    { KC_PCMM, CLR_GREY,   L_RANGE(1, 4) },
    { KC_PDOT, CLR_GREY,   L_RANGE(1, 4) },
    { KC_PSCR, CLR_GREY,   L_RANGE(1, 4) },
    // Gaming layer
    { KC_W,    CLR_RED,        L(_GAMING) },
    { KC_A,    CLR_RED,        L(_GAMING) },
    { KC_S,    CLR_RED,        L(_GAMING) },
    { KC_D,    CLR_RED,        L(_GAMING) },
    { KC_Q,    CLR_BLUE,       L(_GAMING) },
    { KC_E,    CLR_BLUE,       L(_GAMING) },
    { KC_H,    CLR_GREY,       L(_GAMING) },
    { KC_R,    CLR_GREY,       L(_GAMING) },
    { KC_F,    CLR_GREY,       L(_GAMING) },
    { KC_G,    CLR_GREY,       L(_GAMING) },
    { KC_B,    CLR_GREY,       L(_GAMING) },
    { KC_M,    CLR_GREY,       L(_GAMING) },
    { KC_Y,    CLR_GREY,       L(_GAMING) },
    { KC_1,    CLR_GREY,       L(_GAMING) },
    { KC_2,    CLR_GREY,       L(_GAMING) },
    { KC_3,    CLR_GREY,       L(_GAMING) },
    { KC_4,    CLR_GREY,       L(_GAMING) },
    { KC_5,    CLR_GREY,       L(_GAMING) },
    { KC_Z,    CLR_ORANGE,     L(_GAMING) },
    { KC_X,    CLR_ORANGE,     L(_GAMING) },
    { KC_C,    CLR_ORANGE,     L(_GAMING) },
    { KC_LALT, CLR_DARKORANGE, L(_GAMING) },
    { KC_TAB,  CLR_DARKORANGE, L(_GAMING) },
    { KC_LGUI, CLR_DARKORANGE, L(_GAMING) },
    { KC_LSFT, CLR_DARKORANGE, L(_GAMING) },
    { KC_LCTL, CLR_DARKORANGE, L(_GAMING) },
};

// ─── State ───────────────────────────────────────────────────────────────────

#define SYNC_CHUNK_SIZE 24

static uint8_t synced_color_ids[KEY_LED_COUNT];
static bool    synced_color_ids_valid = false;

static struct {
    uint32_t anim_timer;
    bool     done;
} startup;

static struct {
    uint8_t       color_ids[KEY_LED_COUNT];
    layer_state_t layer_state;
    uint8_t       rgb_value;
    uint8_t       led_state_raw;
    bool          valid;
} cache;

static uint32_t keymap_check_timer = 0;
static uint32_t keymap_checksum    = 0;

// ─── Color helpers ────────────────────────────────────────────────────────────

static uint8_t slow_blink_pick(uint8_t a, uint8_t b) {
    return (timer_read32() / 800) % 2 ? a : b;
}

static void apply_color(uint8_t led, uint8_t color_id) {
    switch (color_id) {
        case FX_BLINK_EEPROM: color_id = slow_blink_pick(CLR_RED,    CLR_DARKORANGE); break;
        case FX_BLINK_BOOT:   color_id = slow_blink_pick(CLR_ORANGE, CLR_DARKORANGE); break;
        case FX_BLINK_REBOOT: color_id = slow_blink_pick(CLR_GREEN,  CLR_DARKGREEN);  break;
        default: break;
    }

    if (color_id == CLR_OFF || color_id >= CLR_COUNT) {
        rgb_matrix_set_color(led, 0, 0, 0);
        return;
    }

    uint8_t     v = rgb_matrix_config.hsv.v;
    rgb_color_t c = color_palette[color_id];
    rgb_matrix_set_color(led,
        (uint16_t)c.r * v / 255,
        (uint16_t)c.g * v / 255,
        (uint16_t)c.b * v / 255);
}

// ─── Keycode → color ─────────────────────────────────────────────────────────

static bool is_layer_switch_keycode(uint16_t keycode) {
    switch (keycode) {
        case MO(1): case MO(2): case MO(3): case MO(4): case MO(5):
        case TG(1): case TG(2): case TG(3): case TG(4): case TG(5):
        case TO(1): case TO(2): case TO(3): case TO(4): case TO(5):
        case OSL(1): case OSL(2): case OSL(3): case OSL(4):
        case TT(1): case TT(2): case TT(3): case TT(4):
            return true;
        default:
            return false;
    }
}

static uint8_t color_for_keycode(uint16_t keycode, uint8_t layer) {
    if (keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) {
        // Bit layout of mod byte: [4]=right-side [3]=GUI [2]=ALT [1]=SHIFT [0]=CTRL
        uint8_t mods = (keycode >> 8) & 0x1F;
        if      (mods & 0x08) return CLR_HMR_GUI;
        else if (mods & 0x04) return CLR_HMR_ALT;
        else if (mods & 0x01) return CLR_HMR_CTL;
        else if (mods & 0x02) return CLR_HMR_SFT;
        keycode = keycode & 0xFF;
    } else if (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX) {
        return CLR_LAYERSW;
    }

    if (is_layer_switch_keycode(keycode)) return CLR_LAYERSW;
    if (keycode == EE_CLR)               return FX_BLINK_EEPROM;
    if (keycode == QK_BOOT)              return FX_BLINK_BOOT;
    if (keycode == QK_REBOOT)            return FX_BLINK_REBOOT;
    if (keycode == KC_CAPS) return host_keyboard_led_state().caps_lock ? CLR_WHITE : CLR_OFF;
    if (keycode == KC_NUM)  return host_keyboard_led_state().num_lock  ? CLR_WHITE : CLR_OFF;

    for (uint8_t i = 0; i < ARRAY_SIZE(key_color_rules); i++) {
        const key_color_rule_t *r = &key_color_rules[i];
        if (r->keycode == keycode &&
            (r->layer_mask == LAYER_ALL || (r->layer_mask & LAYER_MASK(layer)))) {
            return r->color_id;
        }
    }

    return CLR_OFF;
}

// ─── Split sync ──────────────────────────────────────────────────────────────

static void light_sync_a_handler(uint8_t in_buflen, const void *in_data,
                                  uint8_t out_buflen, void *out_data) {
    if (in_buflen != SYNC_CHUNK_SIZE || !in_data) return;
    memcpy(&synced_color_ids[0], in_data, SYNC_CHUNK_SIZE);
}

static void light_sync_b_handler(uint8_t in_buflen, const void *in_data,
                                  uint8_t out_buflen, void *out_data) {
    if (in_buflen != SYNC_CHUNK_SIZE || !in_data) return;
    memcpy(&synced_color_ids[SYNC_CHUNK_SIZE], in_data, SYNC_CHUNK_SIZE);
}

static void light_sync_c_handler(uint8_t in_buflen, const void *in_data,
                                  uint8_t out_buflen, void *out_data) {
    if (in_buflen != SYNC_CHUNK_SIZE || !in_data) return;
    memcpy(&synced_color_ids[SYNC_CHUNK_SIZE * 2], in_data, SYNC_CHUNK_SIZE);
    synced_color_ids_valid = true;
}

static void send_light_sync(void) {
    transaction_rpc_send(USER_SYNC_LIGHTS_A, SYNC_CHUNK_SIZE, &cache.color_ids[0]);
    transaction_rpc_send(USER_SYNC_LIGHTS_B, SYNC_CHUNK_SIZE, &cache.color_ids[SYNC_CHUNK_SIZE]);
    transaction_rpc_send(USER_SYNC_LIGHTS_C, SYNC_CHUNK_SIZE, &cache.color_ids[SYNC_CHUNK_SIZE * 2]);
}

// ─── Cache ───────────────────────────────────────────────────────────────────

static void cache_rebuild(void) {
    uint8_t layer = get_highest_layer(layer_state);

    for (uint8_t i = 0; i < KEY_LED_COUNT; i++) {
        cache.color_ids[i] = CACHE_INVALID_COLOR;
        rgb_matrix_set_color(i, 0, 0, 0);
    }

    for (uint8_t row = 0; row < KEY_ROWS; row++) {
        for (uint8_t col = 0; col < KEY_COLS; col++) {
            uint8_t led = g_led_config.matrix_co[row][col];
            if (led == NO_LED) continue;

            uint16_t kc          = dynamic_keymap_get_keycode(layer, row, col);
            cache.color_ids[led] = color_for_keycode(kc, layer);
        }
    }

    cache.layer_state   = layer_state;
    cache.rgb_value     = rgb_matrix_config.hsv.v;
    cache.led_state_raw = host_keyboard_led_state().raw;
    cache.valid         = true;
}

static void cache_flush_range(uint8_t led_min, uint8_t led_max) {
    for (uint8_t led = led_min; led < led_max && led < KEY_LED_COUNT; led++) {
        if (cache.color_ids[led] != CACHE_INVALID_COLOR) {
            apply_color(led, cache.color_ids[led]);
        }
    }
}

static void synced_cache_flush_range(uint8_t led_min, uint8_t led_max) {
    if (!synced_color_ids_valid) return;
    for (uint8_t led = led_min; led < led_max && led < KEY_LED_COUNT; led++) {
        apply_color(led, synced_color_ids[led]);
    }
}

// ─── Keymap change detection ─────────────────────────────────────────────────

static uint32_t calculate_keymap_checksum(void) {
    uint8_t  layer = get_highest_layer(layer_state);
    uint32_t hash  = 2166136261UL;

    for (uint8_t row = 0; row < KEY_ROWS; row++) {
        for (uint8_t col = 0; col < KEY_COLS; col++) {
            if (g_led_config.matrix_co[row][col] == NO_LED) continue;
            uint16_t kc = dynamic_keymap_get_keycode(layer, row, col);
            hash ^= (uint8_t)(kc & 0xFF); hash *= 16777619UL;
            hash ^= (uint8_t)(kc >> 8);   hash *= 16777619UL;
        }
    }

    return hash;
}

static void check_keymap_changed(void) {
    if (timer_elapsed32(keymap_check_timer) < KEYMAP_CHECK_INTERVAL_MS) return;
    keymap_check_timer = timer_read32();

    uint32_t cs = calculate_keymap_checksum();
    if (keymap_checksum == 0) { keymap_checksum = cs; return; }
    if (keymap_checksum != cs) {
        keymap_checksum = cs;
        cache.valid     = false;
    }
}

// ─── Render dispatch ─────────────────────────────────────────────────────────

static void render_lighting_range(uint8_t led_min, uint8_t led_max) {
    if (!is_keyboard_master()) {
        synced_cache_flush_range(led_min, led_max);
        return;
    }

    bool stale =
        !cache.valid ||
        cache.layer_state   != layer_state ||
        cache.rgb_value     != rgb_matrix_config.hsv.v ||
        cache.led_state_raw != host_keyboard_led_state().raw;

    if (stale) {
        cache_rebuild();
        send_light_sync();
    } else {
        check_keymap_changed();
    }

    cache_flush_range(led_min, led_max);
}

// ─── Startup animation ───────────────────────────────────────────────────────

static void startup_tick(uint8_t led_min, uint8_t led_max) {
    if (startup.anim_timer == 0) startup.anim_timer = timer_read32();

    int16_t  head    = (int16_t)(timer_elapsed32(startup.anim_timer) / STARTUP_STEP_MS * 10);
    uint16_t max_pos = 0;

    for (uint8_t led = 0; led < KEY_LED_COUNT; led++) {
        uint8_t  x       = g_led_config.point[led].x;
        uint8_t  y       = g_led_config.point[led].y;
        uint8_t  row     = y / 13;
        uint16_t local_x = (row % 2) ? 220 - x : x;
        uint16_t pos     = (uint16_t)(row * 240) + local_x;
        if (pos > max_pos) max_pos = pos;
    }

    for (uint8_t led = led_min; led < led_max && led < KEY_LED_COUNT; led++) {
        uint8_t  x       = g_led_config.point[led].x;
        uint8_t  y       = g_led_config.point[led].y;
        uint8_t  row     = y / 13;
        uint16_t local_x = (row % 2) ? 220 - x : x;
        uint16_t pos     = (uint16_t)(row * 240) + local_x;
        int16_t  dist    = head - (int16_t)pos;

        if (dist < 0 || dist >= STARTUP_TAIL_WIDTH) {
            rgb_matrix_set_color(led, 0, 0, 0);
            continue;
        }

        uint8_t value = 255 - (uint8_t)((uint16_t)dist * 255 / STARTUP_TAIL_WIDTH);
        uint8_t hue   = (uint8_t)(timer_elapsed32(startup.anim_timer) / 8) + (uint8_t)(pos / 2);
        RGB     rgb   = hsv_to_rgb((HSV){ hue, 255, value });
        rgb_matrix_set_color(led, rgb.r, rgb.g, rgb.b);
    }

    if (head > (int16_t)(max_pos + STARTUP_TAIL_WIDTH)) {
        startup.done = true;
        cache.valid  = false;
    }
}

static void startup_reset(void) {
    startup.anim_timer = 0;
    startup.done       = false;
    cache.valid        = false;
}

static void startup_sync_handler(uint8_t in_buflen, const void *in_data,
                                  uint8_t out_buflen, void *out_data) {
    startup_reset();
}

// ─── Public API ──────────────────────────────────────────────────────────────
// The g_direct_mode_colors relay (viz_frame split-sync + housekeeping) that
// used to share this file's single _user hook now lives in the
// kolbenhans/viz_relay module (its own housekeeping_task_viz_relay /
// keyboard_post_init_viz_relay) — this keymap's engine is unrelated to it
// and stays untouched.

void keyboard_post_init_user(void) {
    transaction_register_rpc(USER_SYNC_LIGHTS_A,          light_sync_a_handler);
    transaction_register_rpc(USER_SYNC_LIGHTS_B,          light_sync_b_handler);
    transaction_register_rpc(USER_SYNC_LIGHTS_C,          light_sync_c_handler);
    transaction_register_rpc(USER_DYNAMIC_LIGHTS_STARTUP, startup_sync_handler);
    startup_reset();
}

void dynamic_lights_on_mode_enter(void) {
    startup_reset();
    if (is_keyboard_master()) {
        transaction_rpc_send(USER_DYNAMIC_LIGHTS_STARTUP, 0, NULL);
    }
}

void dynamic_lights_render(uint8_t led_min, uint8_t led_max) {
    if (!startup.done) {
        startup_tick(led_min, led_max);
        return;
    }
    render_lighting_range(led_min, led_max);
}

#endif // RGB_MATRIX_ENABLE
