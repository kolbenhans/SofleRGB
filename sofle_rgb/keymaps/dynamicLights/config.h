/* Copyright 2020 Josef Adamcik
 * Modification for VIA support and RGB underglow by Jens Bonk-Wiltfang
 * Modification for Vial support by Drew Petersen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// clang-format off

#pragma once

// Vial Support
#define DYNAMIC_KEYMAP_LAYER_COUNT 8
#define VIAL_COMBO_ENTRIES 64
#define VIAL_KEY_OVERRIDE_ENTRIES 64
#define VIAL_TAP_DANCE_ENTRIES 64
#define DYNAMIC_KEYMAP_MACRO_COUNT 32

// #define VIALRGB_NO_DIRECT

// The four corners
#define VIAL_KEYBOARD_UID { 0x05, 0xCD, 0x9F, 0x8A, 0xF4, 0xDF, 0xDE, 0xB2 }
#define VIAL_UNLOCK_COMBO_ROWS { 0, 5, 3, 8 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 0, 0, 0 }

/* oled effect to display, one of 
    * OLED_EFFECT_BONGOCAT
    * OLED_EFFECT_SOUNDMONSTER
    * OLED_EFFECT_LUNA
    * OLED_EFFECT_LUNABONGO
    * OLED_EFFECT_SNAKEY
    * OLED_EFFECT_OLED
    * OLED_EFFECT_PANDA
    * OLED_EFFECT_MECHABOARDS
*/
// set the actual OLED Display
#define OLED_EFFECT_LUNABONGO

#ifdef RGB_MATRIX_ENABLE
#    define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_CUSTOM_dynamic_lights
#endif

#define PERMISSIVE_HOLD

#define SPLIT_TRANSACTION_IDS_USER USER_SYNC_LIGHTS_A, USER_SYNC_LIGHTS_B, USER_SYNC_LIGHTS_C, USER_DYNAMIC_LIGHTS_STARTUP, USER_SYNC_RGB_DIRECT, USER_ENTRY_WAVE_STARTUP

// --- keypeek (srwi/keypeek module) ---
// Its own raw_hid_receive_kb calls raw_hid_receive_user(), which doesn't
// exist on vial-qmk. Disable it, we chain keypeek_handle_command()
// from our own raw_hid_receive_kb in entry_wave.c instead.
#define KEYPEEK_DISABLE_RAW_HID_HANDLER
// --- end keypeek ---
