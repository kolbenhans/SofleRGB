SRC += key_colors.c    # WebGUI-driven per-key/per-layer colors + Raw HID dispatch

# Vial-Features
COMBO_ENABLE = yes          # Enable key combinations (Chords) for special actions
KEY_OVERRIDE_ENABLE = yes   # Allow certain keys to override others (e.g., Shift + Backspace = Delete)
CAPS_WORD_ENABLE = yes      # Auto-disable Caps Lock after the next space or non-word character
QMK_SETTINGS = yes          # Allow changing QMK-specific settings via the Vial GUI
TAP_DANCE_ENABLE = yes      # Enable different actions based on single/double/triple tapping a key
VIAL_INSECURE = yes

# OLED-Features
OS_DETECTION_ENABLE = yes   # Detect the connected OS (Win/Mac/Linux) for OLED/Keymap adaptation

# Debug
CONSOLE_ENABLE = no

RGB_MATRIX_CUSTOM_USER = yes
RAW_ENABLE = yes