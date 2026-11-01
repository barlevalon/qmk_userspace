TAP_DANCE_ENABLE = yes
CAPS_WORD_ENABLE = yes
POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = custom

OPT_DEFS += -DQMK_KEYBOARD_H=\"default_keyboard.h\"
VPATH += .build/charybdis_test keyboards/bastardkb/charybdis
SRC += .build/charybdis_test/default_keyboard.c
