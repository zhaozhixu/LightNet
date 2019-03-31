include config.mk
include common.mk

TARGET ?= lightnet
ABBR ?= ln

SRC_DIR = src
OBJ_DIR = $(SRC_DIR)/obj
TEST_DIR = test

LIBTARGET_A = lib$(TARGET).a
LIBTARGET_SO = lib$(TARGET).so
LIBTARGET_SO_MM = $(LIBTARGET_SO).$(MAJOR).$(MINOR)
LIBTARGET_SO_MMM = $(LIBTARGET_SO).$(MAJOR).$(MINOR).$(MICRO)
BIN = $(TARGET)
BIN_MMM = $(BIN).$(MAJOR).$(MINOR).$(MICRO)

OBJ_A = $(OBJ_DIR)/$(LIBTARGET_A)
OBJ_SO = $(OBJ_DIR)/$(LIBTARGET_SO)
OBJ_BIN = $(OBJ_DIR)/$(BIN)
SRC_HEADERS = $(wildcard $(SRC_DIR)/ln_*.h)

BUILD_DIR ?= build
BUILD_INCLUDE_DIR = $(BUILD_DIR)/include/$(TARGET)
BUILD_LIB_DIR = $(BUILD_DIR)/lib
BUILD_BIN_DIR = $(BUILD_DIR)/bin
BUILD_DOC_DIR = $(BUILD_DIR)/doc

BUILD_A = $(BUILD_LIB_DIR)/$(LIBTARGET_A)
BUILD_SO = $(BUILD_LIB_DIR)/$(LIBTARGET_SO)
BUILD_SO_MM = $(BUILD_LIB_DIR)/$(LIBTARGET_SO_MM)
BUILD_SO_MMM = $(BUILD_LIB_DIR)/$(LIBTARGET_SO_MMM)
BUILD_BIN = $(BUILD_BIN_DIR)/$(BIN)
BUILD_BIN_MMM = $(BUILD_BIN_DIR)/$(BIN_MMM)
BUILD_DOC = $(BUILD_DOC_DIR)/$(TARGET)

INSTALL_DIR ?= /usr/local
INSTALL_INCLUDE_DIR = $(INSTALL_DIR)/include/$(TARGET)
INSTALL_LIB_DIR = $(INSTALL_DIR)/lib
INSTALL_BIN_DIR = $(INSTALL_DIR)/bin
INSTALL_DOC_DIR = $(INSTALL_DIR)/doc

INSTALL_A = $(INSTALL_LIB_DIR)/$(LIBTARGET_A)
INSTALL_SO = $(INSTALL_LIB_DIR)/$(LIBTARGET_SO)
INSTALL_SO_MM = $(INSTALL_LIB_DIR)/$(LIBTARGET_SO_MM)
INSTALL_SO_MMM = $(INSTALL_LIB_DIR)/$(LIBTARGET_SO_MMM)
INSTALL_BIN = $(INSTALL_BIN_DIR)/$(BIN)
INSTALL_BIN_MMM = $(INSTALL_BIN_DIR)/$(BIN_MMM)
INSTALL_DOC = $(INSTALL_DOC_DIR)/$(TARGET)

PKGCONFIG_DIR ?= /usr/local/lib/pkgconfig

CONFIG_SRC = $(SRC_DIR)/$(TARGET).h.in
CONFIG_DST = $(BUILD_INCLUDE_DIR)/$(TARGET).h
CONFIG_DEFINES =
CONFIG_DEFINES += "LN_MAJOR_VERSION ($(MAJOR))"
CONFIG_DEFINES += "LN_MINOR_VERSION ($(MINOR))"
CONFIG_DEFINES += "LN_MICRO_VERSION ($(MICRO))"
ifeq ($(WITH_CUDA), yes)
CONFIG_DEFINES += "LN_CUDA"
endif
ifeq ($(WITH_CUDNN), yes)
CONFIG_DEFINES += "LN_CUDNN"
endif
ifeq ($(WITH_TENSORRT), yes)
CONFIG_DEFINES += "LN_TENSORRT"
endif

ifeq ($(WITH_PYTHON), yes)
PYTHON_TARGET = py$(TARGET)
PYTHON_DIR = tools/py$(TARGET)
INSTALL_PYTHON = (cd $(PYTHON_DIR) && $(PYTHON_CMD) setup.py install --prefix $(PYTHON_PREFIX) --record .install-log)
UNINSTALL_PYTHON = perl tools/uninstallpy.pl $(PYTHON_DIR)/.install-log $(PYTHON_TARGET) $(PYTHON_VERSION) $(PYTHON_PREFIX)
else
INSTALL_PYTHON =
UNINSTALL_PYTHON =
endif

ifeq ($(DOC), yes)
MAKE_DOC = mkdocs build -c -d $(BUILD_DOC)
INSTALL_DOC_CMD = cp -r $(BUILD_DOC) $(INSTALL_DOC)
else
MAKE_DOC =
INSTALL_DOC_CMD =
endif

ifdef VERBOSE
AT =
else
AT = @
endif

define make-build-dir
$(AT)if [ ! -d $(BUILD_DIR) ]; then mkdir -p $(BUILD_DIR); fi
$(AT)if [ ! -d $(BUILD_INCLUDE_DIR) ]; then mkdir -p $(BUILD_INCLUDE_DIR); fi
$(AT)if [ ! -d $(BUILD_LIB_DIR) ]; then mkdir -p $(BUILD_LIB_DIR); fi
$(AT)if [ ! -d $(BUILD_BIN_DIR) ]; then mkdir -p $(BUILD_BIN_DIR); fi
$(AT)if [ ! -d $(BUILD_DOC_DIR) ]; then mkdir -p $(BUILD_DOC_DIR); fi
endef

define make-install-dir
$(AT)if [ ! -d $(INSTALL_DIR) ]; then mkdir -p $(INSTALL_DIR); fi
$(AT)if [ ! -d $(INSTALL_INCLUDE_DIR) ]; then mkdir -p $(INSTALL_INCLUDE_DIR); fi
$(AT)if [ ! -d $(INSTALL_LIB_DIR) ]; then mkdir -p $(INSTALL_LIB_DIR); fi
$(AT)if [ ! -d $(INSTALL_BIN_DIR) ]; then mkdir -p $(INSTALL_BIN_DIR); fi
$(AT)if [ ! -d $(INSTALL_DOC_DIR) ]; then mkdir -p $(INSTALL_DOC_DIR); fi
$(AT)if [ ! -d $(PKGCONFIG_DIR) ]; then mkdir -p $(PKGCONFIG_DIR); fi
endef

define pre-make-lib
$(AT)perl tools/addconfig.pl $(CONFIG_SRC) $(CONFIG_DST) -d $(CONFIG_DEFINES) -i
endef

define make-lib
$(AT)cp $(SRC_HEADERS) $(BUILD_INCLUDE_DIR)
$(AT)cp $(OBJ_A) $(BUILD_A)
$(AT)cp $(OBJ_SO) $(BUILD_SO_MMM)
$(AT)ln -sf $(BUILD_SO_MMM) $(BUILD_SO_MM)
$(AT)ln -sf $(BUILD_SO_MMM) $(BUILD_SO)
endef

define make-bin
$(AT)cp $(OBJ_BIN) $(BUILD_BIN_MMM)
$(AT)ln -sf $(BUILD_BIN_MMM) $(BUILD_BIN)
# TODO: orgnize tools
$(AT)cp tools/ir2json.pl $(BUILD_BIN_DIR)/ir2json.pl
endef

define make-doc
$(AT)$(MAKE_DOC)
endef

define make-install
cp -r $(BUILD_INCLUDE_DIR)/* $(INSTALL_INCLUDE_DIR)
cp $(BUILD_A) $(INSTALL_A)
cp $(BUILD_SO_MMM) $(INSTALL_SO_MMM)
ln -sf $(INSTALL_SO_MMM) $(INSTALL_SO_MM)
ln -sf $(INSTALL_SO_MMM) $(INSTALL_SO)
cp $(BUILD_BIN_MMM) $(INSTALL_BIN_MMM)
ln -sf $(INSTALL_BIN_MMM) $(INSTALL_BIN)
cp $(BUILD_BIN_DIR)/ir2json.pl $(INSTALL_BIN_DIR)/ir2json.pl
$(INSTALL_DOC_CMD)
perl tools/gen_pkgconfig.pl $(TARGET) $(INSTALL_DIR) $(MAJOR).$(MINOR).$(MICRO) $(PKGCONFIG_DIR) "$(REQUIRES)" "A light-weight neural network compiler for different software/hardware backends."
$(INSTALL_PYTHON)
endef

define make-uninstall
rm -rf $(INSTALL_INCLUDE_DIR)
rm -f $(INSTALL_A)
rm -f $(INSTALL_SO)
rm -f $(INSTALL_SO_MM)
rm -f $(INSTALL_SO_MMM)
rm -f $(INSTALL_BIN)
rm -f $(INSTALL_BIN_MMM)
rm -rf $(INSTALL_DOC)
rm -f $(PKGCONFIG_DIR)/$(TARGET).pc
$(UNINSTALL_PYTHON)
endef

define make-clean
$(AT)(cd $(SRC_DIR) && make clean);\
(cd $(TEST_DIR) && make clean)
rm -rf $(BUILD_DIR)
endef

.PHONY: all lib bin test doc clean info install uninstall

all: lib bin test doc

install:
	$(call make-install-dir)
	$(call make-install)

test: bin
	$(AT)(cd $(TEST_DIR) && make)

bin: lib
	$(call make-build-dir)
	$(call make-bin)

lib:
	$(call make-build-dir)
	$(call pre-make-lib)
	$(AT)(cd $(SRC_DIR) && make)
	$(call make-lib)

doc:
	$(call make-build-dir)
	$(call make-doc)

clean:
	$(call make-clean)

uninstall:
	$(call make-uninstall)

info:
	@echo "Available make targets:"
	@echo "  all: make lib, bin, test, doc"
	@echo "  lib: make libraries"
	@echo "  bin: make executables"
	@echo "  test: make and run tests"
	@echo "  doc: make documents"
	@echo "  install: install build directory"
	@echo "  clean: remove all files generated by the build"
	@echo "  uninstall: uninstall installed files"
	@echo "  info: show this infomation"
