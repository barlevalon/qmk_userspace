#include "test_common.hpp"

extern "C" {
#include "action_tapping.h"
#include "keymap_introspection.h"
extern bool test_sniping_enabled;
}

using testing::_;
using testing::InSequence;

class Charybdis : public TestFixture {
   public:
    Charybdis() {
        for (uint8_t layer = 0; layer < keymap_layer_count_raw(); ++layer) {
            for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
                for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
                    add_key(KeymapKey(layer, col, row, keycode_at_keymap_location_raw(layer, row, col)));
                }
            }
        }
    }

    KeymapKey key(uint8_t row, uint8_t col) {
        return KeymapKey(0, col, row, keycode_at_keymap_location_raw(0, row, col));
    }
};

TEST_F(Charybdis, same_hand_home_row_roll_stays_letters) {
    TestDriver driver;
    InSequence sequence;
    auto       f = key(1, 4);
    auto       r = key(0, 4);

    EXPECT_REPORT(driver, (KC_F));
    EXPECT_REPORT(driver, (KC_F, KC_R));
    EXPECT_REPORT(driver, (KC_R));
    EXPECT_EMPTY_REPORT(driver);
    f.press();
    run_one_scan_loop();
    r.press();
    run_one_scan_loop();
    f.release();
    run_one_scan_loop();
    r.release();
    run_one_scan_loop();
}

TEST_F(Charybdis, opposite_hand_nested_chord_uses_shift) {
    TestDriver driver;
    InSequence sequence;
    auto       f = key(1, 4);
    auto       u = key(4, 4);

    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_REPORT(driver, (KC_LSFT, KC_U));
    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
    f.press();
    run_one_scan_loop();
    tap_key(u);
    f.release();
    run_one_scan_loop();
}

TEST_F(Charybdis, fast_typing_flow_does_not_hold_shift) {
    TestDriver driver;
    InSequence sequence;
    auto       q = key(0, 1);
    auto       f = key(1, 4);

    EXPECT_REPORT(driver, (KC_Q));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_F));
    EXPECT_EMPTY_REPORT(driver);
    tap_key(q);
    f.press();
    idle_for(TAPPING_TERM + 10);
    f.release();
    run_one_scan_loop();
    EXPECT_EQ(get_mods(), 0);
}

TEST_F(Charybdis, isolated_long_hold_retro_taps_letter) {
    TestDriver driver;
    InSequence sequence;
    auto       f = key(1, 4);

    EXPECT_REPORT(driver, (KC_LSFT));
    EXPECT_EMPTY_REPORT(driver);
    EXPECT_REPORT(driver, (KC_F));
    EXPECT_EMPTY_REPORT(driver);
    f.press();
    idle_for(TAPPING_TERM + 10);
    f.release();
    run_one_scan_loop();
}

TEST_F(Charybdis, pointer_hold_survives_typing_flow) {
    TestDriver driver;
    InSequence sequence;
    auto       q = key(0, 1);
    auto       z = key(2, 1);

    EXPECT_REPORT(driver, (KC_Q));
    EXPECT_EMPTY_REPORT(driver);
    tap_key(q);
    z.press();
    idle_for(TAPPING_TERM + 10);
    EXPECT_TRUE(layer_state_is(7));
    EXPECT_TRUE(test_sniping_enabled);
    z.release();
    run_one_scan_loop();
    EXPECT_FALSE(layer_state_is(7));
    EXPECT_FALSE(test_sniping_enabled);
}

TEST_F(Charybdis, right_pointer_hold_does_not_type_slash) {
    TestDriver driver;
    InSequence sequence;
    auto       p     = key(4, 1);
    auto       slash = key(6, 1);

    EXPECT_REPORT(driver, (KC_P));
    EXPECT_EMPTY_REPORT(driver);
    tap_key(p);
    slash.press();
    idle_for(TAPPING_TERM + 10);
    EXPECT_TRUE(layer_state_is(7));
    slash.release();
    run_one_scan_loop();
    EXPECT_FALSE(layer_state_is(7));
}

TEST_F(Charybdis, same_hand_pointer_chord_activates_layer) {
    TestDriver driver;
    auto       z = key(2, 1);
    auto       x = key(2, 2);

    EXPECT_NO_REPORT(driver);
    z.press();
    run_one_scan_loop();
    tap_key(x);
    EXPECT_TRUE(layer_state_is(7));
    z.release();
    run_one_scan_loop();
    EXPECT_FALSE(layer_state_is(7));
}

TEST_F(Charybdis, function_layer_has_function_keys) {
    TestDriver driver;
    InSequence sequence;
    auto       tab = key(3, 1);
    auto       u   = key(4, 4);

    EXPECT_REPORT(driver, (KC_F7));
    EXPECT_EMPTY_REPORT(driver);
    tab.press();
    idle_for(TAPPING_TERM + 10);
    EXPECT_TRUE(layer_state_is(6));
    tap_key(u);
    tab.release();
    run_one_scan_loop();
    EXPECT_FALSE(layer_state_is(6));
}

TEST_F(Charybdis, navigation_tab_position_supplies_delete) {
    TestDriver driver;
    InSequence sequence;
    auto       space = key(3, 4);
    auto       tab   = key(3, 1);

    EXPECT_REPORT(driver, (KC_DEL));
    EXPECT_EMPTY_REPORT(driver);
    space.press();
    idle_for(TAPPING_TERM + 10);
    tap_key(tab);
    space.release();
    run_one_scan_loop();
}

TEST_F(Charybdis, leader_single_tap_releases_all_modifiers) {
    TestDriver driver;
    InSequence sequence;
    auto       leader = key(6, 0);

    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT, KC_LALT, KC_LGUI));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT, KC_LALT, KC_LGUI, KC_SPC));
    EXPECT_REPORT(driver, (KC_LCTL, KC_LSFT, KC_LALT, KC_LGUI));
    EXPECT_EMPTY_REPORT(driver);
    tap_key(leader);
    idle_for(TAPPING_TERM + 10);
    EXPECT_EQ(get_mods(), 0);
    EXPECT_EQ(get_weak_mods(), 0);
    EXPECT_FALSE(layer_state_is(1));
}

TEST_F(Charybdis, double_tap_enters_and_exits_gaming) {
    TestDriver driver;
    auto       leader = key(6, 0);

    EXPECT_NO_REPORT(driver);
    tap_key(leader);
    idle_for(20);
    tap_key(leader);
    idle_for(TAPPING_TERM + 10);
    EXPECT_TRUE(layer_state_is(1));
    EXPECT_EQ(keycode_at_keymap_location_raw(1, 1, 4), KC_F);
    tap_key(leader);
    idle_for(20);
    tap_key(leader);
    idle_for(TAPPING_TERM + 10);
    EXPECT_FALSE(layer_state_is(1));
    EXPECT_EQ(get_mods(), 0);
    EXPECT_EQ(get_weak_mods(), 0);
}
