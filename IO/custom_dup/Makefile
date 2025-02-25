CC := gcc
CFLAGS := -Wall -Werror
DEBUG ?= no

CURRENT_DIR := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR := $(CURRENT_DIR)/build
TARGET := $(BUILD_DIR)/custom_dup

TEST_FILE_NAME := file.txt
CFLAGS += -DTEST_FILE_NAME="\"$(TEST_FILE_NAME)\""

ifeq ($(DEBUG),y)
CFLAGS += -DDEBUG
endif

$(TARGET): main.c $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir -p $@

.PHONY: clean run
clean:
	rm -rf $(BUILD_DIR) $(TEST_FILE_NAME)

run: $(TARGET)
	@$<
	