/**
 * Copyright 2023 Hearter
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
#include QMK_KEYBOARD_H

enum corne_keymap_layers {
    LAYER_BASE = 0,
    LAYER_NUM,
    LAYER_SYM,
    LAYER_NAV,
    LAYER_MEDIA,
    LAYER_FN,
    LAYER_GAMING,
};

#define NUM_BSPC LT(LAYER_NUM, KC_BSPC)
#define SYM_ENT LT(LAYER_SYM, KC_ENT)
#define NAV_SPC LT(LAYER_NAV, KC_SPC)
#define MED_ESC LT(LAYER_MEDIA, KC_ESC)
#define FN_TAB LT(LAYER_FN, KC_TAB)
#define FN_DEL LT(LAYER_FN, KC_DEL)

#define ONE_PASS G(KC_BSLS)
#define TABS LCAG(KC_T)
#define RAYC G(KC_SPC)
#define LEADER TD(TD_GAMING_TOGGLE) // Tap dance: single tap for HYPR(KC_SPACE), double tap to toggle gaming layer
#define LCRLY S(KC_LBRC)
#define RCRLY S(KC_RBRC)
#define CLN S(KC_SCLN)
#define TLD S(KC_GRV)
#define PIPE S(KC_BSLS)
#define LPAREN S(KC_9)
#define RPAREN S(KC_0)
#define UNDSCR S(KC_MINS)

// Left-hand home row mods
#define HOME_A LCTL_T(KC_A)
#define HOME_S LALT_T(KC_S)
#define HOME_D LGUI_T(KC_D)
#define HOME_F LSFT_T(KC_F)

// Right-hand home row mods
#define HOME_J RSFT_T(KC_J)
#define HOME_K RGUI_T(KC_K)
#define HOME_L LALT_T(KC_L)
#define HOME_QUO RCTL_T(KC_QUOT)

// User config structure to store in EEPROM
typedef union {
    uint32_t raw;
    struct {
        bool rgb_enabled : 1;
        // You can add more settings here if needed
    };
} user_config_t;

user_config_t user_config;

enum custom_keycodes {
    TMUX = SAFE_RANGE,
    RGB_TOG_EE, // Custom RGB toggle with EEPROM persistence
};

// Tap Dance definitions
enum {
    TD_GAMING_TOGGLE, // Tap dance for gaming layer toggle
    TD_TO_BASE,       // Tap dance to return to base layer
};

// Forward declarations
void set_rgb_for_layer(uint8_t layer);

// Tap dance functions
void gaming_toggle_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count >= 2) {
        // Double-tap or more: toggle the gaming layer
        layer_invert(LAYER_GAMING);
    } else {
        // Single tap: perform the original key function (HYPR+Space)
        if (state->pressed) {
            register_mods(MOD_BIT(KC_LCTL) | MOD_BIT(KC_LALT) | MOD_BIT(KC_LGUI) | MOD_BIT(KC_LSFT)); // HYPR
        } else {
            register_mods(MOD_BIT(KC_LCTL) | MOD_BIT(KC_LALT) | MOD_BIT(KC_LGUI) | MOD_BIT(KC_LSFT)); // HYPR
            tap_code(KC_SPACE);
            unregister_mods(MOD_BIT(KC_LCTL) | MOD_BIT(KC_LALT) | MOD_BIT(KC_LGUI) | MOD_BIT(KC_LSFT)); // HYPR
        }
    }
}

void gaming_toggle_reset(tap_dance_state_t *state, void *user_data) {
    // If the key was just tapped and not held, do nothing here
    // If it was held and then released, we already sent the key in the finished function
    if (state->count == 1 && state->pressed) {
        unregister_mods(MOD_BIT(KC_LCTL) | MOD_BIT(KC_LALT) | MOD_BIT(KC_LGUI) | MOD_BIT(KC_LSFT)); // HYPR
    }
}

// Tap dance function to go back to base layer
void to_base_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count >= 2) {
        // Double-tap or more: go back to base layer
        layer_clear(); // Clear all layers and go back to base
    }
    // Single tap: Do nothing (matches ZMK behavior)
}

// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {[TD_GAMING_TOGGLE] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, gaming_toggle_finished, gaming_toggle_reset), [TD_TO_BASE] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, to_base_finished, NULL)};

// Initialize user EEPROM with default values
void eeconfig_init_user(void) {
    // Initialize the user EEPROM with default values
    user_config.raw         = 0;
    user_config.rgb_enabled = true; // RGB enabled by default
    eeconfig_update_user(user_config.raw);
}

// Read the user config from EEPROM and apply settings
void keyboard_post_init_user(void) {
    // Read the user config from EEPROM
    user_config.raw = eeconfig_read_user();

#ifdef RGBLIGHT_ENABLE
    // Apply the RGB enabled state from EEPROM
    if (user_config.rgb_enabled) {
        // Enable RGB and set initial color for base layer
        rgblight_enable();
        rgblight_mode(RGBLIGHT_MODE_STATIC_LIGHT);
        set_rgb_for_layer(LAYER_BASE);
    } else {
        rgblight_disable();
    }
#endif
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case TMUX:
            if (record->event.pressed) {
                // on press
                SEND_STRING(SS_LALT("t") SS_DELAY(200) "`t");
            }
            break;

        case RGB_TOG_EE:
            if (record->event.pressed) {
                // Toggle the RGB enabled state
                user_config.rgb_enabled = !user_config.rgb_enabled;

                // Update EEPROM with the new setting
                eeconfig_update_user(user_config.raw);

                // Actually toggle the RGB and update the layer color if enabled
                if (user_config.rgb_enabled) {
                    rgblight_enable();
                    set_rgb_for_layer(get_highest_layer(layer_state));
                } else {
                    rgblight_disable();
                }
            }
            return false; // Skip all further processing of this key
    }
    return true;
}

#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return rotation;
}

// Custom bootmagic handling for RGB indicators
void bootmagic_lite_reset_handler(void) {
#    ifdef RGBLIGHT_ENABLE
    // Flash all LEDs red as bootloader indication
    rgblight_enable_noeeprom();

    // Several bright flashes to make it very obvious
    for (int i = 0; i < 5; i++) {
        rgblight_setrgb(RGB_RED);
        wait_ms(50);
        rgblight_setrgb(RGB_OFF);
        wait_ms(50);
    }

    // Solid red for a moment before bootloader
    rgblight_setrgb(RGB_RED);
    wait_ms(100);
#    endif
}

// Define RGB colors for each layer
void set_rgb_for_layer(uint8_t layer) {
#    ifdef RGBLIGHT_ENABLE
    switch (layer) {
        case LAYER_BASE:
            // More purple shade for base layer
            rgblight_sethsv_noeeprom(190, 255, 255); // Purple-blue
            break;
        case LAYER_NUM:
            // Teal/Cyan for number layer
            rgblight_sethsv_noeeprom(128, 255, 255); // Teal
            break;
        case LAYER_SYM:
            // Green for symbol layer
            rgblight_sethsv_noeeprom(85, 255, 255); // Green
            break;
        case LAYER_NAV:
            // Red-orange for navigation layer
            rgblight_sethsv_noeeprom(10, 255, 255); // Red-orange
            break;
        case LAYER_MEDIA:
            // Bright yellow for media layer
            rgblight_sethsv_noeeprom(43, 255, 255); // Bright yellow
            break;
        case LAYER_FN:
            // Purple for function layer
            rgblight_sethsv_noeeprom(213, 255, 255); // Purple
            break;
        case LAYER_GAMING:
            // Bright red for gaming layer
            rgblight_sethsv_noeeprom(0, 255, 255); // Red
            break;
        default:
            // Magenta for unknown layers
            rgblight_sethsv_noeeprom(234, 255, 255); // Magenta
            break;
    }
#    endif
}

// Layer state change callback
layer_state_t layer_state_set_user(layer_state_t state) {
#    ifdef RGBLIGHT_ENABLE
    // Only update RGB if it should be enabled (based on EEPROM setting)
    if (user_config.rgb_enabled) {
        // Enable RGB and set color based on active layer
        rgblight_enable_noeeprom();
        set_rgb_for_layer(get_highest_layer(state));
    }
#    endif
    return state;
}

bool shutdown_user(bool jump_to_bootloader) {
#    ifdef RGBLIGHT_ENABLE
    rgblight_enable_noeeprom();

    // Multiple flashes for visibility
    for (int i = 0; i < 3; i++) {
        rgblight_setrgb(RGB_RED);
        wait_ms(50);
        rgblight_setrgb(RGB_OFF);
        wait_ms(50);
    }

    // Final red for a moment
    rgblight_setrgb(RGB_RED);
    wait_ms(100);
#    endif

    return false;
}

// Per-key tapping term for homerow mods optimization
uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        // Pinkies need more time (weaker fingers)
        case HOME_A:
        case HOME_QUO:
            return TAPPING_TERM + 25;
        // Ring fingers
        case HOME_S:
        case HOME_L:
            return TAPPING_TERM + 10;
        // Index fingers are faster
        case HOME_F:
        case HOME_J:
            return TAPPING_TERM - 10;
        default:
            return TAPPING_TERM;
    }
}

// Print current layer with a visual indicator
void render_layer_state(void) {
    oled_write_P(PSTR("LAYER"), false);
    oled_write_P(PSTR("\n"), false);

    switch (get_highest_layer(layer_state)) {
        case LAYER_BASE:
            oled_write_P(PSTR(" BASE "), false);
            break;
        case LAYER_NUM:
            oled_write_P(PSTR(" NUM  "), false);
            break;
        case LAYER_SYM:
            oled_write_P(PSTR(" SYM  "), false);
            break;
        case LAYER_NAV:
            oled_write_P(PSTR(" NAV  "), false);
            break;
        case LAYER_MEDIA:
            oled_write_P(PSTR(" MEDIA"), false);
            break;
        case LAYER_FN:
            oled_write_P(PSTR(" FUNC "), false);
            break;
        case LAYER_GAMING:
            oled_write_P(PSTR(" GAME "), false);
            break;
        default:
            oled_write_P(PSTR("????  "), false);
    }
}

// Print current modifier state
void render_mod_status(uint8_t modifiers) {
    oled_write_P(PSTR("\nMODS\n"), false);
    oled_write_P(PSTR(" "), false);
    oled_write_P((modifiers & MOD_MASK_SHIFT) ? PSTR("S") : PSTR(" "), false);
    oled_write_P((modifiers & MOD_MASK_CTRL) ? PSTR("C") : PSTR(" "), false);
    oled_write_P((modifiers & MOD_MASK_ALT) ? PSTR("A") : PSTR(" "), false);
    oled_write_P((modifiers & MOD_MASK_GUI) ? PSTR("G") : PSTR(" "), false);
}

// Print caps lock indicator
void render_caps_lock(bool caps_on) {
    oled_write_P(PSTR("\nCAPS"), false);
    oled_write_P(PSTR("\n "), false);
    oled_write_P(caps_on ? PSTR("[ON]") : PSTR("    "), false);
}

// Render keylogger (last pressed key)
void render_keylogger(void) {
    oled_write_P(PSTR("\nLAST"), false);
    oled_write_P(PSTR("\n "), false);

    // This would need a key logger implementation
    // For now just display a placeholder
    oled_write_P(PSTR("KEY"), false);
}

// Main OLED task function
bool oled_task_user(void) {
    // Clear the display
    oled_clear();

    if (is_keyboard_master()) {
        // Left OLED - Show centered layer name and mods

        // Layer name - centered
        uint8_t layer = get_highest_layer(layer_state);
        uint8_t layer_pos;

        // First determine the position for proper centering
        switch (layer) {
            case LAYER_MEDIA:
                layer_pos = 7; // Adjust for longer name (5 chars)
                break;
            case LAYER_FN:
                layer_pos = 9; // Adjust for shorter name (2 chars)
                break;
            case LAYER_GAMING:
                layer_pos = 8; // "GAME" is 4 chars
                break;
            default:
                layer_pos = 8; // Default for 4-char layer names
                break;
        }

        // Now position cursor and display the text
        oled_set_cursor(layer_pos, 0);
        switch (layer) {
            case LAYER_BASE:
                oled_write_P(PSTR("BASE"), false);
                break;
            case LAYER_NUM:
                oled_write_P(PSTR("NUM"), false);
                break;
            case LAYER_SYM:
                oled_write_P(PSTR("SYM"), false);
                break;
            case LAYER_NAV:
                oled_write_P(PSTR("NAV"), false);
                break;
            case LAYER_MEDIA:
                oled_write_P(PSTR("MEDIA"), false);
                break;
            case LAYER_FN:
                oled_write_P(PSTR("FN"), false);
                break;
            case LAYER_GAMING:
                oled_write_P(PSTR("GAME"), false);
                break;
            default:
                oled_write_P(PSTR("???"), false);
        }

        // Show modifier status - centered
        uint8_t mod_state = get_mods();

        char mods[5] = "    ";
        if (mod_state & MOD_MASK_SHIFT) mods[0] = 'S';
        if (mod_state & MOD_MASK_CTRL) mods[1] = 'C';
        if (mod_state & MOD_MASK_ALT) mods[2] = 'A';
        if (mod_state & MOD_MASK_GUI) mods[3] = 'G';

        oled_set_cursor(8, 2);
        oled_write(mods, false);
    } else {
        // Right OLED - Display "HEARTER" text

        // Place HEARTER centered on line 2
        // The OLED is 21 characters wide (128 pixels / 6 pixels per char)
        // "HEARTER" is 7 characters, so start at position (21-7)/2 = 7
        oled_set_cursor(7, 1);
        oled_write_P(PSTR("HEARTER"), false);
        oled_set_cursor(8, 2);
        oled_write_P(PSTR("crkbd"), false);
    }

    return false;
}
#endif

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [LAYER_BASE] = LAYOUT_split_3x6_3(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
       TMUX,    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,       KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    TABS,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       KC_CAPS, HOME_A,  HOME_S,  HOME_D,  HOME_F,  KC_G,       KC_H,    HOME_J,  HOME_K,  HOME_L,  HOME_QUO, RAYC,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       ONE_PASS, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,       KC_N,    KC_M,   KC_COMM, KC_DOT,  KC_SLSH, LEADER,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                  MED_ESC, NAV_SPC, FN_TAB,     SYM_ENT, NUM_BSPC, FN_DEL
  //                            ╰───────────────────────────╯ ╰──────────────────────────────╯
  ),

  [LAYER_NUM] = LAYOUT_split_3x6_3(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
       XXXXXXX, KC_LBRC, KC_7,    KC_8,    KC_9,    KC_RBRC,    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       XXXXXXX, KC_SCLN, KC_4,    KC_5,    KC_6,    KC_EQL,     XXXXXXX, KC_RSFT, KC_RGUI, KC_RALT, KC_RCTL, XXXXXXX,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       XXXXXXX, KC_GRV,  KC_1,    KC_2,    KC_3,    KC_BSLS,    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                  KC_DOT,  KC_0,    KC_MINS,    XXXXXXX, _______, XXXXXXX
  //                            ╰───────────────────────────╯ ╰──────────────────────────────╯
  ),

  [LAYER_SYM] = LAYOUT_split_3x6_3(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
       XXXXXXX, LCRLY,   S(KC_7), S(KC_8), S(KC_9), RCRLY,      XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       XXXXXXX, CLN,     S(KC_4), S(KC_5), S(KC_6), KC_PPLS,    XXXXXXX, KC_RSFT, KC_RGUI, KC_RALT, KC_RCTL, XXXXXXX,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       XXXXXXX, TLD,     S(KC_1), S(KC_2), S(KC_3), PIPE,       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                  LPAREN,  RPAREN,  UNDSCR,     _______, XXXXXXX, XXXXXXX
  //                            ╰───────────────────────────╯ ╰──────────────────────────────╯
  ),

  [LAYER_NAV] = LAYOUT_split_3x6_3(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       XXXXXXX, KC_LCTL, KC_LALT, KC_LGUI, KC_LSFT, XXXXXXX,    KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, CW_TOGG, XXXXXXX,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,    KC_END,  KC_PGDN, KC_PGUP, KC_HOME, XXXXXXX, XXXXXXX,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                  XXXXXXX, _______, XXXXXXX,    KC_ENT,  KC_BSPC, KC_DEL
  //                            ╰───────────────────────────╯ ╰──────────────────────────────╯
  ),

  [LAYER_MEDIA] = LAYOUT_split_3x6_3(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
       XXXXXXX, XXXXXXX, KC_PSCR, XXXXXXX, XXXXXXX, XXXXXXX,    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       XXXXXXX, KC_LCTL, KC_LALT, KC_LGUI, KC_LSFT, XXXXXXX,    KC_MPRV, KC_VOLD, KC_VOLU, KC_MNXT, XXXXXXX, XXXXXXX,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,    XXXXXXX, KC_MUTE, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                  _______, XXXXXXX, XXXXXXX,    KC_MSTP, KC_MPLY, XXXXXXX
  //                            ╰───────────────────────────╯ ╰──────────────────────────────╯
  ),

  [LAYER_FN] = LAYOUT_split_3x6_3(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
       QK_BOOT,  EE_CLR, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,    XXXXXXX, KC_F7,   KC_F8,   KC_F9,   KC_F12,   QK_BOOT,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       XXXXXXX, KC_LCTL, KC_LALT, KC_LGUI, KC_LSFT, XXXXXXX,    XXXXXXX, KC_F4,   KC_F5,   KC_F6,   KC_F11,   XXXXXXX,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       XXXXXXX, _______, RGB_TOG_EE, RGB_HUI, RGB_SAI, RGB_VAI, XXXXXXX, KC_F1, KC_F2, KC_F3, KC_F10, XXXXXXX,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                  RGB_MOD, RGB_MOD, RGB_RMOD,   XXXXXXX, XXXXXXX, XXXXXXX
  //                            ╰───────────────────────────╯ ╰──────────────────────────────╯
  ),

  [LAYER_GAMING] = LAYOUT_split_3x6_3(
  // ╭──────────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────────────╮
       KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,       KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_F12,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,       KC_H,    KC_J,    KC_K,    KC_L,    KC_QUOT, XXXXXXX,
  // ├──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────┤
       KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,       KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, TD(TD_TO_BASE),
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                  KC_SPC,  KC_LALT, LT(LAYER_NUM, KC_ESC), KC_ENT, KC_BSPC, KC_DEL
  //                            ╰───────────────────────────╯ ╰──────────────────────────────╯
  ),
};
// clang-format on
