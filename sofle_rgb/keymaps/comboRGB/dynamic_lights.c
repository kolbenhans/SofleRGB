#include QMK_KEYBOARD_H
#include "transactions.h"
#include "dynamic_lights.h"
#include "entry_wave.h"
#include <string.h>

#ifdef RGB_MATRIX_EFFECT_VIALRGB_DIRECT
extern HSV g_direct_mode_colors[RGB_MATRIX_LED_COUNT];
#endif

#if defined(RGB_MATRIX_ENABLE)

// ─── Hardware constants ───────────────────────────────────────────────────────

#define KEY_LED_COUNT 72

// ─── Startup animation ───────────────────────────────────────────────────────

#define STARTUP_STEP_MS    10
#define STARTUP_TAIL_WIDTH 84

// ─── Per-key/per-layer color table ────────────────────────────────────────────
// Host (WebGUI) owns this data: assigned via Raw HID 0x02/0xA5, persisted via
// 0x02/0xA6, read back via 0x02/0xA7. Firmware never derives color from
// keycode/keymap content — only from LED index + active layer.

typedef struct { uint8_t r, g, b; } rgb_color_t;

static rgb_color_t key_colors[DYNAMIC_KEYMAP_LAYER_COUNT][KEY_LED_COUNT];
static uint8_t      key_lock_flags[DYNAMIC_KEYMAP_LAYER_COUNT][KEY_LED_COUNT];
static bool         colors_dirty = false;

#define SYNC_CHUNK_LEDS (KEY_LED_COUNT / 3)
#define SYNC_CHUNK_SIZE (SYNC_CHUNK_LEDS * sizeof(rgb_color_t))

static rgb_color_t synced_colors[KEY_LED_COUNT];
static bool         synced_colors_valid = false;

static struct {
    uint32_t anim_timer;
    bool     done;
} startup;

static struct {
    rgb_color_t   colors[KEY_LED_COUNT];
    layer_state_t layer_state;
    uint8_t       led_state_raw; // host_keyboard_led_state().raw — für Lock-Flag-Gating
    bool          valid;
} cache;

// ─── Host API ─────────────────────────────────────────────────────────────────

static void load_colors(void) {
    if (eeconfig_is_kb_datablock_valid()) {
        eeconfig_read_kb_datablock(key_colors, 0, sizeof(key_colors));
        eeconfig_read_kb_datablock(key_lock_flags, sizeof(key_colors), sizeof(key_lock_flags));
    } else {
        memset(key_colors, 0, sizeof(key_colors));
        memset(key_lock_flags, 0, sizeof(key_lock_flags));
    }
}

void dynamic_lights_set_colors(uint8_t layer, uint8_t led_offset, uint8_t count, const uint8_t *rgb_bytes) {
    if (layer >= DYNAMIC_KEYMAP_LAYER_COUNT) return;
    for (uint8_t i = 0; i < count && (uint16_t)led_offset + i < KEY_LED_COUNT; i++) {
        rgb_color_t *c = &key_colors[layer][led_offset + i];
        c->r = rgb_bytes[i * 3 + 0];
        c->g = rgb_bytes[i * 3 + 1];
        c->b = rgb_bytes[i * 3 + 2];
    }
    colors_dirty = true;
}

void dynamic_lights_get_colors(uint8_t layer, uint8_t led_offset, uint8_t count, uint8_t *out_rgb_bytes) {
    memset(out_rgb_bytes, 0, (size_t)count * 3);
    if (layer >= DYNAMIC_KEYMAP_LAYER_COUNT) return;
    for (uint8_t i = 0; i < count && (uint16_t)led_offset + i < KEY_LED_COUNT; i++) {
        rgb_color_t c              = key_colors[layer][led_offset + i];
        out_rgb_bytes[i * 3 + 0]   = c.r;
        out_rgb_bytes[i * 3 + 1]   = c.g;
        out_rgb_bytes[i * 3 + 2]   = c.b;
    }
}

void dynamic_lights_commit_colors(void) {
    eeconfig_update_kb_datablock(key_colors, 0, sizeof(key_colors));
    eeconfig_update_kb_datablock(key_lock_flags, sizeof(key_colors), sizeof(key_lock_flags));
}

void dynamic_lights_set_lock_flags(uint8_t layer, uint8_t led, uint8_t flags) {
    if (layer >= DYNAMIC_KEYMAP_LAYER_COUNT || led >= KEY_LED_COUNT) return;
    key_lock_flags[layer][led] = flags;
    colors_dirty = true;
}

void dynamic_lights_get_lock_flags(uint8_t layer, uint8_t led_offset, uint8_t count, uint8_t *out_flags) {
    memset(out_flags, 0, count);
    if (layer >= DYNAMIC_KEYMAP_LAYER_COUNT) return;
    for (uint8_t i = 0; i < count && (uint16_t)led_offset + i < KEY_LED_COUNT; i++) {
        out_flags[i] = key_lock_flags[layer][led_offset + i];
    }
}

// ─── Split sync ──────────────────────────────────────────────────────────────

static void light_sync_a_handler(uint8_t in_buflen, const void *in_data,
                                  uint8_t out_buflen, void *out_data) {
    if (in_buflen != SYNC_CHUNK_SIZE || !in_data) return;
    memcpy(&synced_colors[0], in_data, SYNC_CHUNK_SIZE);
}

static void light_sync_b_handler(uint8_t in_buflen, const void *in_data,
                                  uint8_t out_buflen, void *out_data) {
    if (in_buflen != SYNC_CHUNK_SIZE || !in_data) return;
    memcpy(&synced_colors[SYNC_CHUNK_LEDS], in_data, SYNC_CHUNK_SIZE);
}

static void light_sync_c_handler(uint8_t in_buflen, const void *in_data,
                                  uint8_t out_buflen, void *out_data) {
    if (in_buflen != SYNC_CHUNK_SIZE || !in_data) return;
    memcpy(&synced_colors[SYNC_CHUNK_LEDS * 2], in_data, SYNC_CHUNK_SIZE);
    synced_colors_valid = true;
}

// transaction_rpc_send() silently drops the packet if the split link isn't
// up yet (is_transport_connected() check inside transactions.c) — right
// after boot this can race the startup-comet timing, so the very first sync
// attempt sometimes fails. Track success and keep retrying every tick until
// it goes through once; a real layer/color change always re-triggers anyway.
static bool link_synced = false;

static void send_light_sync(void) {
    bool ok = transaction_rpc_send(USER_SYNC_LIGHTS_A, SYNC_CHUNK_SIZE, &cache.colors[0]);
    ok      = transaction_rpc_send(USER_SYNC_LIGHTS_B, SYNC_CHUNK_SIZE, &cache.colors[SYNC_CHUNK_LEDS]) && ok;
    ok      = transaction_rpc_send(USER_SYNC_LIGHTS_C, SYNC_CHUNK_SIZE, &cache.colors[SYNC_CHUNK_LEDS * 2]) && ok;
    if (ok) link_synced = true;
}

// ─── Cache ───────────────────────────────────────────────────────────────────
// Repaints from key_colors[] only when the active layer changes, new color/
// lock-flag data arrives over Raw HID (colors_dirty), or the live lock state
// changes (needed for lock-gated LEDs to react instantly) — not on every
// rgb_matrix tick otherwise. Lock-gating is resolved into cache.colors[] here
// (not per-tick in cache_flush_range) so the split-sync path (send_light_sync)
// ships already-resolved colors to the slave half, same as before — the slave
// never needs its own host_keyboard_led_state().

static void cache_rebuild(void) {
    uint8_t layer     = get_highest_layer(layer_state);
    led_t   led_state = host_keyboard_led_state();

    for (uint8_t led = 0; led < KEY_LED_COUNT; led++) {
        uint8_t flags   = key_lock_flags[layer][led];
        bool    visible = !(((flags & LOCK_FLAG_NUM)  && !led_state.num_lock) ||
                             ((flags & LOCK_FLAG_CAPS) && !led_state.caps_lock) ||
                             ((flags & LOCK_FLAG_SCRL) && !led_state.scroll_lock));
        cache.colors[led] = visible ? key_colors[layer][led] : (rgb_color_t){0, 0, 0};
    }

    cache.layer_state   = layer_state;
    cache.led_state_raw = led_state.raw;
    cache.valid          = true;
}

static void cache_flush_range(uint8_t led_min, uint8_t led_max) {
    uint8_t v = rgb_matrix_config.hsv.v;
    for (uint8_t led = led_min; led < led_max && led < KEY_LED_COUNT; led++) {
        rgb_color_t c = cache.colors[led];
        rgb_matrix_set_color(led, (uint16_t)c.r * v / 255, (uint16_t)c.g * v / 255, (uint16_t)c.b * v / 255);
    }
}

static void synced_cache_flush_range(uint8_t led_min, uint8_t led_max) {
    if (!synced_colors_valid) return;
    uint8_t v = rgb_matrix_config.hsv.v;
    for (uint8_t led = led_min; led < led_max && led < KEY_LED_COUNT; led++) {
        rgb_color_t c = synced_colors[led];
        rgb_matrix_set_color(led, (uint16_t)c.r * v / 255, (uint16_t)c.g * v / 255, (uint16_t)c.b * v / 255);
    }
}

// ─── Render dispatch ─────────────────────────────────────────────────────────

static void render_lighting_range(uint8_t led_min, uint8_t led_max) {
    if (!is_keyboard_master()) {
        synced_cache_flush_range(led_min, led_max);
        return;
    }

    bool content_stale = !cache.valid || cache.layer_state != layer_state || colors_dirty ||
                          cache.led_state_raw != host_keyboard_led_state().raw;

    if (content_stale) {
        cache_rebuild();
        colors_dirty = false;
    }
    if (content_stale || !link_synced) {
        send_light_sync();
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

// ─── Direct-mode (viz_frame) split sync ─────────────────────────────────────
// g_direct_mode_colors is filled via Raw HID FASTSET, which only reaches the
// master half over USB — the slave half needs its portion pushed over the
// split link explicitly, same mechanism as m57's dynamic_lights.c.
//
// LED indices 0..SYNC_HALF_SIZE-1 are always the physical left half and
// SYNC_HALF_SIZE..KEY_LED_COUNT-1 the right half (fixed by g_led_config,
// independent of which side is plugged in as master) — is_keyboard_left()
// picks the right offset regardless of which physical side is master.

#define SYNC_HALF_SIZE (KEY_LED_COUNT / 2)

#ifdef RGB_MATRIX_EFFECT_VIALRGB_DIRECT
static void rgb_direct_sync_handler(uint8_t in_buflen, const void *in_data,
                                     uint8_t out_buflen, void *out_data) {
    (void)out_buflen;
    (void)out_data;
    if (in_buflen != SYNC_HALF_SIZE * sizeof(HSV) || in_data == NULL) return;
    uint8_t local_offset = is_keyboard_left() ? 0 : SYNC_HALF_SIZE;
    memcpy(&g_direct_mode_colors[local_offset], in_data, in_buflen);
}

void housekeeping_task_user(void) {
    if (!is_keyboard_master()) return;
    if (rgb_matrix_get_mode() != RGB_MATRIX_CUSTOM_viz_frame) return;

    static uint32_t last_sync = 0;
    if (timer_elapsed32(last_sync) < 20) return;
    last_sync = timer_read32();

    uint8_t remote_offset = is_keyboard_left() ? SYNC_HALF_SIZE : 0;
    transaction_rpc_send(USER_SYNC_RGB_DIRECT,
                          SYNC_HALF_SIZE * sizeof(HSV),
                          &g_direct_mode_colors[remote_offset]);
}
#endif

// ─── Public API ──────────────────────────────────────────────────────────────

void keyboard_post_init_user(void) {
    load_colors();
    transaction_register_rpc(USER_SYNC_LIGHTS_A,          light_sync_a_handler);
    transaction_register_rpc(USER_SYNC_LIGHTS_B,          light_sync_b_handler);
    transaction_register_rpc(USER_SYNC_LIGHTS_C,          light_sync_c_handler);
    transaction_register_rpc(USER_DYNAMIC_LIGHTS_STARTUP, startup_sync_handler);
#ifdef RGB_MATRIX_EFFECT_VIALRGB_DIRECT
    transaction_register_rpc(USER_SYNC_RGB_DIRECT, rgb_direct_sync_handler);
#endif
    entry_wave_register_rpc();
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
