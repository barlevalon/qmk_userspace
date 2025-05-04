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
#    define DYNAMIC_KEYMAP_LAYER_COUNT 6
#endif // VIA_ENABLE

/* Disable unused features. */
#define NO_ACTION_ONESHOT

/* For OLED or other display if present */
#ifdef OLED_ENABLE
#    define OLED_DISPLAY_128X64
#    define OLED_TIMEOUT 0  // Never timeout
#endif

/* Improved key press behavior */
#define TAPPING_TERM 175
#define PERMISSIVE_HOLD

/* Split keyboard specific */
#define SPLIT_TRANSPORT_MIRROR