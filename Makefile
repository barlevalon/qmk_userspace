.SILENT:

MAKEFLAGS += --no-print-directory

QMK_USERSPACE := $(patsubst %/,%,$(dir $(shell realpath "$(lastword $(MAKEFILE_LIST))")))
ifeq ($(QMK_USERSPACE),)
    QMK_USERSPACE := $(shell pwd)
endif

QMK_FIRMWARE_ROOT = $(shell qmk env QMK_HOME)
ifeq ($(QMK_FIRMWARE_ROOT),)
    $(error Cannot determine qmk_firmware location. `qmk config -ro user.qmk_home` is not set)
endif

.PHONY: test-charybdis
test-charybdis:
	qmk generate-keyboard-h -kb bastardkb/charybdis/3x6/elitec -i charybdis.h -o "$(QMK_FIRMWARE_ROOT)/.build/charybdis_test/default_keyboard.h" -q
	qmk generate-keyboard-c -kb bastardkb/charybdis/3x6/elitec -o "$(QMK_FIRMWARE_ROOT)/.build/charybdis_test/default_keyboard.c" -q
	ln -sfn "$(QMK_USERSPACE)/tests/charybdis" "$(QMK_FIRMWARE_ROOT)/.build/charybdis_test/tests"
	+$(MAKE) -C $(QMK_FIRMWARE_ROOT) -f builddefs/build_test.mk SILENT=false QMK_BIN=qmk TEST=charybdis FULL_TESTS=charybdis TEST_OUTPUT=charybdis TEST_PATH=.build/charybdis_test/tests
	"$(QMK_FIRMWARE_ROOT)/.build/test/charybdis.elf"

%:
	+$(MAKE) -C $(QMK_FIRMWARE_ROOT) $(MAKECMDGOALS) QMK_USERSPACE=$(QMK_USERSPACE)
