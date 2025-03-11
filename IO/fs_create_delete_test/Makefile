CC := gcc
CFLAGS := -Wall -g

BUILD_DIR := build
TARGET := $(BUILD_DIR)/test_fs
MOUNT_DIR := /tmp/test_fs_mount
IMG := $(BUILD_DIR)/$(FS_TYPE).img

$(TARGET): main.c $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir -p $@


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
