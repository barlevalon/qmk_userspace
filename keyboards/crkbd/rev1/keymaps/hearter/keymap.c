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
#include <stdio.h>   // For sprintf
#include <string.h>  // For strlen

enum corne_keymap_layers {
    LAYER_BASE = 0,
    LAYER_NUM,
    LAYER_SYM,
    LAYER_NAV,
    LAYER_MEDIA,
    LAYER_FN,
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
#define LEADER HYPR(KC_SPACE)
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

enum custom_keycodes {
    TMUX = SAFE_RANGE,
    RGB_TOG_LAYER, // Toggle RGB and remember to restore layer color when turned back on
};

// Forward declaration for RGB layer function
void set_rgb_for_layer(uint8_t layer);

// Global state to track if RGB should be enabled
bool rgb_enabled = true;

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    switch(keycode) {
        case TMUX:
            if (record->event.pressed) {
                // on press
                SEND_STRING(SS_LALT("t") SS_DELAY(200) "`t");
            }
            break;
            
        case RGB_TOG_LAYER:
            if (record->event.pressed) {
                // Toggle RGB on/off with state tracking
                rgb_enabled = !rgb_enabled;
                
                if (rgb_enabled) {
                    // If enabling RGB, turn it on and set color for current layer
                    rgblight_enable_noeeprom();
                    set_rgb_for_layer(get_highest_layer(layer_state));
                } else {
                    // If disabling RGB, turn it off
                    rgblight_disable_noeeprom();
                }
            }
            return false; // Skip all further processing of this key
    }
    return true;
}

// Keyboard initialization
void keyboard_post_init_user(void) {
#ifdef RGBLIGHT_ENABLE
    // Initialize RGB state
    rgb_enabled = true;
    
    // Enable RGB and set initial color for base layer
    rgblight_enable();
    rgblight_mode(RGBLIGHT_MODE_STATIC_LIGHT);
    set_rgb_for_layer(LAYER_BASE);
#endif
}

#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return rotation;
}

// Custom bootmagic handling for RGB indicators
void bootmagic_lite_reset_handler(void) {
#ifdef RGBLIGHT_ENABLE
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
#endif
}

// Define RGB colors for each layer
void set_rgb_for_layer(uint8_t layer) {
#ifdef RGBLIGHT_ENABLE
    switch (layer) {
        case LAYER_BASE:
            // Cool blue/violet for base layer
            rgblight_sethsv_noeeprom(170, 255, 255); // Blue-violet
            break;
        case LAYER_NUM:
            // Teal/Cyan for number layer
            rgblight_sethsv_noeeprom(128, 255, 255); // Teal
            break;
        case LAYER_SYM:
            // Green for symbol layer
            rgblight_sethsv_noeeprom(85, 255, 255);  // Green
            break;
        case LAYER_NAV:
            // Orange for navigation layer
            rgblight_sethsv_noeeprom(28, 255, 255);  // Orange
            break;
        case LAYER_MEDIA:
            // Yellow for media layer
            rgblight_sethsv_noeeprom(43, 255, 255);  // Yellow
            break;
        case LAYER_FN:
            // Purple for function layer
            rgblight_sethsv_noeeprom(213, 255, 255); // Purple
            break;
        default:
            // Magenta for unknown layers
            rgblight_sethsv_noeeprom(234, 255, 255); // Magenta
            break;
    }
#endif
}

// Layer state change callback
layer_state_t layer_state_set_user(layer_state_t state) {
#ifdef RGBLIGHT_ENABLE
    // Only update RGB if it should be enabled
    if (rgb_enabled) {
        // Enable RGB and set color based on active layer
        rgblight_enable_noeeprom();
        set_rgb_for_layer(get_highest_layer(state));
    }
#endif
    return state;
}

bool shutdown_user(bool jump_to_bootloader) {
#ifdef RGBLIGHT_ENABLE
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
#endif
    
    return false;
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

// Render WPM counter
void render_wpm(void) {
#ifdef WPM_ENABLE
    oled_write_P(PSTR("\nWPM\n "), false);
    
    uint8_t n = get_current_wpm();
    char wpm_str[4];
    wpm_str[3] = '\0';
    wpm_str[2] = '0' + n % 10;
    wpm_str[1] = '0' + (n /= 10) % 10;
    wpm_str[0] = '0' + n / 10;
    oled_write(wpm_str, false);
    
#else
    oled_write_P(PSTR("\nWPM\n ---"), false);
#endif
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
            default:
                oled_write_P(PSTR("???"), false);
        }
        
        // Show modifier status - centered
        uint8_t mod_state = get_mods();
        
        char mods[5] = "    ";
        if (mod_state & MOD_MASK_SHIFT) mods[0] = 'S';
        if (mod_state & MOD_MASK_CTRL)  mods[1] = 'C';
        if (mod_state & MOD_MASK_ALT)   mods[2] = 'A';
        if (mod_state & MOD_MASK_GUI)   mods[3] = 'G';
        
        oled_set_cursor(8, 2);
        oled_write(mods, false);
    } else {
        // Right OLED - Display "HEARTER" text
        
        // Place HEARTER centered on line 2
        // The OLED is 21 characters wide (128 pixels / 6 pixels per char)
        // "HEARTER" is 7 characters, so start at position (21-7)/2 = 7
        oled_set_cursor(7, 1);
        oled_write_P(PSTR("HEARTER"), false);
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
       XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,    XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
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
       XXXXXXX, _______, RGB_TOG_LAYER, RGB_HUI, RGB_SAI, RGB_VAI, XXXXXXX, KC_F1, KC_F2, KC_F3, KC_F10, XXXXXXX,
  // ╰──────────────────────────────────────────────────────┤ ├──────────────────────────────────────────────────────╯
                                  RGB_MOD, RGB_MOD, RGB_RMOD,   XXXXXXX, XXXXXXX, XXXXXXX
  //                            ╰───────────────────────────╯ ╰──────────────────────────────╯
  ),
};
// clang-format on