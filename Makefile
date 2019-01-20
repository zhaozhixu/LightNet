include config.mk
include common.mk

TARGET ?= lightnet
TARGET_VERSION = $(TARGET).$(MAJOR).$(MINOR).$(MICRO)

SRC_DIR = src
OBJ_DIR = $(SRC_DIR)/obj
TEST_DIR = test
BUILD_DIR ?= build
BUILD_BIN_DIR = $(BUILD_DIR)/bin
BUILD_DOC_DIR = $(BUILD_DIR)/doc

INSTALL_DIR ?= /usr/local
INSTALL_BIN_DIR = $(INSTALL_DIR)/bin
INSTALL_DOC_DIR = $(INSTALL_DIR)/doc

OBJ_BIN = $(OBJ_DIR)/$(TARGET)
BUILD_BIN_VERSION = $(BUILD_BIN_DIR)/$(TARGET_VERSION)
BUILD_DOC = $(BUILD_DOC_DIR)/$(TARGET)

INSTALL_BIN = $(INSTALL_BIN_DIR)/$(TARGET)
INSTALL_BIN_VERSION = $(INSTALL_BIN_DIR)/$(TARGET_VERSION)
INSTALL_DOC = $(INSTALL_DOC_DIR)/$(TARGET)

ifdef VERBOSE
AT =
else
AT = @
endif

define make-build-dir
$(AT)if [ ! -d $(BUILD_DIR) ]; then mkdir -p $(BUILD_DIR); fi
$(AT)if [ ! -d $(BUILD_BIN_DIR) ]; then mkdir -p $(BUILD_BIN_DIR); fi
$(AT)if [ ! -d $(BUILD_DOC_DIR) ]; then mkdir -p $(BUILD_DOC_DIR); fi
endef

define make-install-dir
$(AT)if [ ! -d $(INSTALL_DIR) ]; then mkdir -p $(INSTALL_DIR); fi
$(AT)if [ ! -d $(INSTALL_BIN_DIR) ]; then mkdir -p $(INSTALL_BIN_DIR); fi
$(AT)if [ ! -d $(INSTALL_DOC_DIR) ]; then mkdir -p $(INSTALL_DOC_DIR); fi
endef

define make-bin
$(AT)cp $(OBJ_BIN) $(BUILD_BIN_VERSION)
endef

define make-doc
$(AT)mkdocs build -c
endef

define make-install
cp $(BUILD_BIN_VERSION) $(INSTALL_BIN_VERSION)
cp $(INSTALL_BIN_VERSION) $(INSTALL_BIN)
cp -r $(BUILD_DOC) $(INSTALL_DOC)
endef

define make-uninstall
rm -f $(INSTALL_BIN)
rm -f $(INSTALL_BIN_VERSION)
rm -rf $(INSTALL_DOC)
endef

define make-clean
$(AT)(cd $(SRC_DIR) && make clean);\
(cd $(TEST_DIR) && make clean)
rm -rf $(BUILD_DIR)
endef

.PHONY: all bin test doc clean info install uninstall

all: bin test doc

install:
	$(call make-install-dir)
	$(call make-install)

test: bin
	$(AT)(cd $(TEST_DIR) && make)

bin:
	$(AT)(cd $(SRC_DIR) && make)
	$(call make-build-dir)
	$(call make-bin)

doc:
	$(call make-build-dir)
	$(call make-doc)

clean:
	$(call make-clean)

uninstall:
	$(call make-uninstall)

info:
	@echo "Available make targets:"
	@echo "  all: make bin, test, doc"
	@echo "  bin: make binaries"
	@echo "  test: make and run tests"
	@echo "  doc: make documents"
	@echo "  install: install build directory"
	@echo "  clean: clean up all object files"
	@echo "  uninstall: uninstall binaries"
	@echo "  info: show this infomation"
