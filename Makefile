include config.mk

TARGET ?= lightnet
TARGET_VERSION = $(TARGET).$(MAJOR).$(MINOR).$(MICRO)

SRC_DIR = src
OBJ_DIR = $(SRC_DIR)/obj
TEST_DIR = test
BUILD_DIR ?= build
BUILD_BIN_DIR = $(BUILD_DIR)/bin
INSTALL_DIR ?= /usr/local
INSTALL_BIN_DIR = $(INSTALL_DIR)/bin

OBJ_BIN = $(OBJ_DIR)/$(TARGET)
BUILD_BIN_VERSION = $(BUILD_BIN_DIR)/$(TARGET_VERSION)
INSTALL_BIN = $(INSTALL_BIN_DIR)/$(TARGET)
INSTALL_BIN_VERSION = $(INSTALL_BIN_DIR)/$(TARGET_VERSION)

ifdef VERBOSE
AT =
else
AT = @
endif

define make-build-dir
$(AT)if [ ! -d $(BUILD_DIR) ]; then mkdir -p $(BUILD_DIR); fi
$(AT)if [ ! -d $(BUILD_BIN_DIR) ]; then mkdir -p $(BUILD_BIN_DIR); fi
cp $(OBJ_BIN) $(BUILD_BIN_VERSION)
endef

define make-install-dir
$(AT)if [ ! -d $(INSTALL_DIR) ]; then mkdir -p $(INSTALL_DIR); fi
$(AT)if [ ! -d $(INSTALL_BIN_DIR) ]; then mkdir -p $(INSTALL_BIN_DIR); fi
cp $(BUILD_BIN_VERSION) $(INSTALL_BIN_VERSION)
ln -s $(INSTALL_BIN_VERSION) $(INSTALL_BIN)
endef

.PHONY: all bin test clean info install uninstall

all: bin test

install:
	$(call make-install-dir)

test: bin
	$(AT)(cd $(TEST_DIR) && make)

bin:
	$(AT)(cd $(SRC_DIR) && make)
	$(call make-build-dir)

clean:
	$(AT)(cd $(SRC_DIR) && make clean);\
	(cd $(TEST_DIR) && make clean)
	rm -rf $(BUILD_DIR)

uninstall:
	rm $(INSTALL_BIN)
	rm $(INSTALL_BIN_VERSION)

info:
	@echo "Available make targets:"
	@echo "  all: make executables and tests"
	@echo "  bin: make executables"
	@echo "  test: make tests"
	@echo "  install: install executables"
	@echo "  clean: clean up all object files"
	@echo "  uninstall: uninstall executables"
	@echo "  info: show this infomation"
