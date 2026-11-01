#include "../../keyboards/bastardkb/charybdis/3x6/keymaps/hearter/keymap.c"

bool test_sniping_enabled;

void charybdis_set_pointer_sniping_enabled(bool enabled) {
    test_sniping_enabled = enabled;
}
