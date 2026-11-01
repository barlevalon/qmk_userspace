#pragma once

#include "test_common.h"
#undef MATRIX_ROWS
#undef MATRIX_COLS
#define MATRIX_ROWS 8
#define MATRIX_COLS 6

#include "../../keyboards/bastardkb/charybdis/3x6/keymaps/hearter/config.h"
// Match the RP2040 build, where __arm__ keeps oneshot actions enabled.
#undef NO_ACTION_ONESHOT
