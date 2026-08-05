#include "entry_wave.h"
#include "dynamic_lights.h"
#include "transactions.h"
#include "keypeek_layer_notify.h" // keypeek

#ifdef RAW_ENABLE

static bool     entry_wave_active = false;
static uint32_t entry_wave_timer  = 0;

void entry_wave_start(void) {
    entry_wave_active = true;
    entry_wave_timer  = timer_read32();
}

bool entry_wave_running(void) {
    return entry_wave_active;
}

uint32_t entry_wave_elapsed(void) {
    return timer_elapsed32(entry_wave_timer);
}

void entry_wave_stop(void) {
    entry_wave_active = false;
}

static void entry_wave_sync_handler(uint8_t in_buflen, const void *in_data,
                                     uint8_t out_buflen, void *out_data) {
    (void)in_buflen;
    (void)in_data;
    (void)out_buflen;
    (void)out_data;
    entry_wave_start();
}

void entry_wave_register_rpc(void) {
    transaction_register_rpc(USER_ENTRY_WAVE_STARTUP, entry_wave_sync_handler);
}

// Starts the wave locally and pushes it to the other half too — a plain
// entry_wave_start() only fires where it's called, which is always the
// master (raw HID / key events never reach the slave directly).
void entry_wave_trigger(void) {
    entry_wave_start();
    if (is_keyboard_master()) {
        transaction_rpc_send(USER_ENTRY_WAVE_STARTUP, 0, NULL);
    }
}

// WebGUI key-color chunk size — bounded by the 32-byte raw HID report minus
// the 5-byte header (family, subcmd, layer, led_offset, count).
#define KEY_COLOR_CHUNK_MAX 9

static void handle_get_key_colors(const uint8_t *req) {
    uint8_t layer      = req[2];
    uint8_t led_offset = req[3];
    uint8_t count      = req[4];
    if (count > KEY_COLOR_CHUNK_MAX) count = KEY_COLOR_CHUNK_MAX;

    uint8_t resp[32] = {0};
    resp[0] = 0x02;
    resp[1] = 0xA7;
    resp[2] = layer;
    resp[3] = led_offset;
    resp[4] = count;
    dynamic_lights_get_colors(layer, led_offset, count, &resp[5]);
    host_raw_hid_send(resp, sizeof(resp));
}

// Lock-flags chunk size — 1 byte per LED, same 32-byte report.
#define LOCK_FLAGS_CHUNK_MAX 27

static void handle_get_lock_flags(const uint8_t *req) {
    uint8_t layer      = req[2];
    uint8_t led_offset = req[3];
    uint8_t count      = req[4];
    if (count > LOCK_FLAGS_CHUNK_MAX) count = LOCK_FLAGS_CHUNK_MAX;

    uint8_t resp[32] = {0};
    resp[0] = 0x02;
    resp[1] = 0xA9;
    resp[2] = layer;
    resp[3] = led_offset;
    resp[4] = count;
    dynamic_lights_get_lock_flags(layer, led_offset, count, &resp[5]);
    host_raw_hid_send(resp, sizeof(resp));
}

// Mode-switch and WebGUI key-color/lock-flag commands. 0xA3/0xA4 come from
// the Python viz tool (viz_hid.py); 0xA5-0xA9 come from the browser WebGUI (WebHID).
void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    // --- keypeek: claims its own subscribe/keepalive packets, ignores everything else ---
    if (keypeek_handle_command(data, length)) return;
    // --- end keypeek ---

    if (length < 2 || data[0] != 0x02) return;

    switch (data[1]) {
        case 0xA3:
            entry_wave_trigger();
            rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_viz_frame);
            break;

        case 0xA4:
            dynamic_lights_on_mode_enter();
            rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_dynamic_lights);
            break;

        case 0xA5: { // SET_KEY_COLORS_CHUNK: layer, led_offset, count, (r,g,b)×count
            if (length < 5) return;
            uint8_t count = data[4];
            if (count > KEY_COLOR_CHUNK_MAX) count = KEY_COLOR_CHUNK_MAX;
            if (length < (uint16_t)5 + (uint16_t)count * 3) return;
            dynamic_lights_set_colors(data[2], data[3], count, &data[5]);
            break;
        }

        case 0xA6: { // COMMIT_KEY_COLORS: persist current table to EEPROM
            dynamic_lights_commit_colors();
            // Ack sent only after the flash write actually completes — the
            // host previously took "HID report sent" as "safe to reboot",
            // but that's just USB transfer done, not flash-write-done. A
            // reboot mid-write can corrupt the datablock (all colors lost).
            uint8_t resp[32] = {0};
            resp[0] = 0x02;
            resp[1] = 0xA6;
            host_raw_hid_send(resp, sizeof(resp));
            break;
        }

        case 0xA7: // GET_KEY_COLORS_CHUNK: layer, led_offset, count
            if (length < 5) return;
            handle_get_key_colors(data);
            break;

        case 0xA8: // SET_KEY_LOCK_FLAGS: layer, led, flags
            if (length < 5) return;
            dynamic_lights_set_lock_flags(data[2], data[3], data[4]);
            break;

        case 0xA9: // GET_KEY_LOCK_FLAGS_CHUNK: layer, led_offset, count
            if (length < 5) return;
            handle_get_lock_flags(data);
            break;
    }
}

#else

bool     entry_wave_running(void)      { return false; }
uint32_t entry_wave_elapsed(void)      { return 0; }
void     entry_wave_stop(void)         {}
void     entry_wave_start(void)        {}
void     entry_wave_trigger(void)      {}
void     entry_wave_register_rpc(void) {}
void     raw_hid_receive_kb(uint8_t *data, uint8_t length) { (void)data; (void)length; }

#endif
