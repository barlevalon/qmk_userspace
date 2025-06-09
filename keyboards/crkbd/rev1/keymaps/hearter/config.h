/*
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
#pragma once

#ifdef VIA_ENABLE
/* VIA configuration. */
#    define DYNAMIC_KEYMAP_LAYER_COUNT 7  // Include GAMING layer
#endif // VIA_ENABLE

/* Disable unused features. */
#define NO_ACTION_ONESHOT

/* For OLED or other display if present */
#ifdef OLED_ENABLE
#    define OLED_TIMEOUT 0  // Never timeout
#    define OLED_BRIGHTNESS 255  // Maximum brightness
#endif


/* RGB configuration */
#ifdef RGBLIGHT_ENABLE
#    define RGBLIGHT_ANIMATIONS             // Enable all animations
#    define RGBLIGHT_SLEEP                  // Turn off LEDs when computer goes to sleep
#endif

/* Improved key press behavior for fast typing */
#define TAPPING_TERM 150  // Reduced from 200ms for faster response
#define QUICK_TAP_TERM 120  // Quick tap optimization for fast typing
#define RETRO_TAPPING  // Send tap even if held longer than tapping term
#define HOLD_ON_OTHER_KEY_PRESS  // Better for fast typing with home row mods

/* Fast typing optimizations */
#define DEBOUNCE 3  // Reduce from default 5ms for faster response
#define USB_POLLING_INTERVAL_MS 1  // 1000Hz polling for gaming/fast typing

/* Tap dance configuration */
#define TAPPING_TERM_TAP_DANCE 200   // Time window for tap dance (ms)

/* Split keyboard specific */
#define SPLIT_TRANSPORT_MIRROR