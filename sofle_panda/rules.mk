# MCU and Bootloader configuration
# MCU = RP2040                # Microcontroller Unit: Raspberry Pi RP2040
# BOOTLOADER = rp2040         # Bootloader for RP2040 chips

# Core Framework Features
VIA_ENABLE = yes            # Enable VIA protocol for real-time keymap editing
OLED_ENABLE = yes           # Enable support for OLED displays
LTO_ENABLE = yes            # Link Time Optimization: Reduces firmware size
VIALRGB_ENABLE = yes        # Enable Vial-specific RGB lighting control
RGB_MATRIX_ENABLE = yes     # Enable RGB Matrix lighting effects
RGB_MATRIX_KEYPRESSES = yes # Enable reactive lighting effects on keypress

# Special Features
EXTRAKEY_ENABLE = yes       # Audio control (Volume) and System control (Sleep/Power)
MOUSEKEY_ENABLE = yes       # Control mouse cursor and buttons via keyboard
WPM_ENABLE = yes            # Enable Words Per Minute (WPM) calculation and reporting
ENCODER_ENABLE = yes
ENCODER_MAP_ENABLE = yes    # Advanced layer handling for rotary encoders


