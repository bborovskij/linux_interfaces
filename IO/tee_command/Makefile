CC := gcc
CFLAGS := -Wall

# Main target
BUILD_DIR := build
DEBUG ?= no
TARGET := $(BUILD_DIR)/my_tee

# Test related 
TEST_DIR := test
TEST_VENV_DIR := $(TEST_DIR)/.venv
TEST_APT_PACKAGES := $(TEST_DIR)/packages.txt
TEST_PYTHON_REQUIREMENTS := $(TEST_DIR)/requirements.txt
PYTEST_CACHE := .pytest_cache
PY_CACHE := $(TEST_DIR)/__pycache__


# Enable LOG_DEBUG
ifeq ($(DEBUG),y)
CFLAGS += -DDEBUG
endif

# Main target
$(TARGET): main.c $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Main target
$(BUILD_DIR):
	mkdir -p $@

# Test related. requires sudo
$(TEST_VENV_DIR): $(TEST_APT_PACKAGES) $(TEST_PYTHON_REQUIREMENTS)
	xargs apt-get -y install < $<
	python3 -m venv $@
	$@/bin/pip install -r $(word 2,$^)

.PHONY: clean test
clean:
	rm -rf $(BUILD_DIR) $(TEST_VENV_DIR) $(PYTEST_CACHE) $(PY_CACHE)

# Test related
test: $(TEST_VENV_DIR) $(TARGET) $(TEST_DIR)
	$</bin/pytest $(TEST_DIR) --binary $(word 2,$^)

