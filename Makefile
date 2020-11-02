include config.mk

ifdef VERBOSE
AT =
else
AT = @
endif

export BIN = $(TARGET)
export LIBTARGET_A = lib$(TARGET).a
export LIBTARGET_SO = lib$(TARGET).so

LIBTARGET_SO_MM = $(LIBTARGET_SO).$(MAJOR).$(MINOR)
LIBTARGET_SO_MMM = $(LIBTARGET_SO).$(MAJOR).$(MINOR).$(MICRO)
BIN_MMM = $(BIN).$(MAJOR).$(MINOR).$(MICRO)

BUILD_SRC_DIR = $(BUILD_DIR)/$(SRC_DIR)
OBJ_A = $(BUILD_SRC_DIR)/$(LIBTARGET_A)
OBJ_SO = $(BUILD_SRC_DIR)/$(LIBTARGET_SO)
OBJ_BIN = $(BUILD_SRC_DIR)/$(BIN)

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

CONFIG_SRC = $(SRC_DIR)/$(TARGET).h.in
CONFIG_DST = $(BUILD_INCLUDE_DIR)/$(TARGET).h
CONFIG_DEFINES =
CONFIG_DEFINES += "$(ABBR)_MAJOR_VERSION ($(MAJOR))"
CONFIG_DEFINES += "$(ABBR)_MINOR_VERSION ($(MINOR))"
CONFIG_DEFINES += "$(ABBR)_MICRO_VERSION ($(MICRO))"
ifeq ($(WITH_CUDA), yes)
CONFIG_DEFINES += "$(ABBR)_CUDA"
endif
ifeq ($(WITH_CUDNN), yes)
CONFIG_DEFINES += "$(ABBR)_CUDNN"
endif
ifeq ($(WITH_TENSORRT), yes)
CONFIG_DEFINES += "$(ABBR)_TENSORRT"
endif
ifeq ($(WITH_DPU), yes)
CONFIG_DEFINES += "$(ABBR)_DPU"
endif

ifeq ($(WITH_PYTHON), yes)
PYTHON_TARGET = py$(TARGET)
PYTHON_DIR = tools/py$(TARGET)
ifeq ($(PYTHON_USER), yes)
INSTALL_PYTHON = (cd $(PYTHON_DIR) && $(PYTHON_CMD) setup.py install --user --record .install-log)
else
INSTALL_PYTHON = (cd $(PYTHON_DIR) && $(PYTHON_CMD) setup.py install --prefix $(PYTHON_PREFIX) --record .install-log)
endif
UNINSTALL_PYTHON = perl $(BUILDTOOLS_DIR)/uninstallpy.pl $(PYTHON_DIR)/.install-log $(PYTHON_TARGET)
else
INSTALL_PYTHON =
UNINSTALL_PYTHON =
endif

ifeq ($(DOC), yes)
MAKE_DOC_CMD = mkdocs build -c -d $(BUILD_DOC)
INSTALL_DOC_CMD = if [ -d $(BUILD_DOC) ]; then cp -r $(BUILD_DOC) $(INSTALL_DOC); fi
UNINSTALL_DOC_CMD = rm -rf $(INSTALL_DOC)
else
MAKE_DOC_CMD =
INSTALL_DOC_CMD =
UNINSTALL_DOC_CMD =
endif

ifeq ($(HAS_EXTRA_BINS), yes)
MAKE_EXTRA_BINS_CMD = cp -L $(EXTRA_BINS) $(BUILD_BIN_DIR)
INSTALL_EXTRA_BINS_CMD = cp $(BUILD_EXTRA_BINS) $(INSTALL_BIN_DIR)
UNINSTALL_EXTRA_BINS_CMD = rm -f $(INSTALL_EXTRA_BINS)
else
MAKE_EXTRA_BINS_CMD =
INSTALL_EXTRA_BINS_CMD =
UNINSTALL_EXTRA_BINS_CMD =
endif

define make-build-dir
$(AT)if [ ! -d $(BUILD_DIR) ]; then mkdir -p $(BUILD_DIR); fi
$(AT)if [ ! -d $(BUILD_INCLUDE_DIR) ]; then mkdir -p $(BUILD_INCLUDE_DIR); fi
$(AT)if [ ! -d $(BUILD_LIB_DIR) ]; then mkdir -p $(BUILD_LIB_DIR); fi
$(AT)if [ ! -d $(BUILD_BIN_DIR) ]; then mkdir -p $(BUILD_BIN_DIR); fi
$(AT)if [ ! -d $(BUILD_DOC_DIR) ]; then mkdir -p $(BUILD_DOC_DIR); fi
$(AT)find $(SRC_DIR) -type d -print0 | xargs -0 -I{} mkdir -p $(BUILD_DIR)/{}
$(AT)find $(TEST_DIR) -type d -print0 | xargs -0 -I{} mkdir -p $(BUILD_DIR)/{}
endef

define make-install-dir
$(AT)if [ ! -d $(INSTALL_DIR) ]; then mkdir -p $(INSTALL_DIR); fi
$(AT)if [ ! -d $(INSTALL_INCLUDE_DIR) ]; then mkdir -p $(INSTALL_INCLUDE_DIR); fi
$(AT)if [ ! -d $(INSTALL_LIB_DIR) ]; then mkdir -p $(INSTALL_LIB_DIR); fi
$(AT)if [ ! -d $(INSTALL_BIN_DIR) ]; then mkdir -p $(INSTALL_BIN_DIR); fi
$(AT)if [ ! -d $(INSTALL_DOC_DIR) ]; then mkdir -p $(INSTALL_DOC_DIR); fi
$(AT)if [ ! -d $(PKGCONFIG_DIR) ]; then mkdir -p $(PKGCONFIG_DIR); fi
endef

define pre-make-config
$(AT)perl $(BUILDTOOLS_DIR)/add_config.pl $(CONFIG_SRC) $(CONFIG_DST) -d $(CONFIG_DEFINES) -i
endef

define make-lib
$(AT)cp $(EXPORT_HEADERS) $(BUILD_INCLUDE_DIR)
$(AT)cp $(OBJ_A) $(BUILD_A)
$(AT)cp $(OBJ_SO) $(BUILD_SO_MMM)
$(AT)ln -sf $(LIBTARGET_SO_MMM) $(BUILD_SO_MM)
$(AT)ln -sf $(LIBTARGET_SO_MMM) $(BUILD_SO)
endef

define make-bin
$(AT)cp $(OBJ_BIN) $(BUILD_BIN_MMM)
$(AT)ln -sf $(BIN_MMM) $(BUILD_BIN)
$(AT)$(MAKE_EXTRA_BINS_CMD)
endef

define make-doc
$(AT)$(MAKE_DOC_CMD)
endef

define make-install
cp -r $(BUILD_INCLUDE_DIR)/* $(INSTALL_INCLUDE_DIR)
cp $(BUILD_A) $(INSTALL_A)
cp $(BUILD_SO_MMM) $(INSTALL_SO_MMM)
ln -sf $(LIBTARGET_SO_MMM) $(INSTALL_SO_MM)
ln -sf $(LIBTARGET_SO_MMM) $(INSTALL_SO)
cp $(BUILD_BIN_MMM) $(INSTALL_BIN_MMM)
ln -sf $(BIN_MMM) $(INSTALL_BIN)
$(INSTALL_EXTRA_BINS_CMD)
$(INSTALL_DOC_CMD)
perl $(BUILDTOOLS_DIR)/gen_pkgconfig.pl $(TARGET) $(INSTALL_DIR) $(MAJOR).$(MINOR).$(MICRO) $(PKGCONFIG_DIR) "$(REQUIRES)" "A light-weight neural network compiler for different software/hardware backends."
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
$(UNINSTALL_EXTRA_BINS_CMD)
$(UNINSTALL_DOC_CMD)
rm -f $(PKGCONFIG_DIR)/$(TARGET).pc
$(UNINSTALL_PYTHON)
endef

define make-clean
$(AT)$(MAKE) -C $(SRC_DIR) clean; $(MAKE) -C $(TEST_DIR) clean
rm -rf $(BUILD_DIR)
endef

CMD_FILE ?= $(BUILD_DIR)/compile_commands.json

.PHONY: all lib bin test cmd doc clean info help install uninstall

all: lib bin

install:
	$(call make-install-dir)
	$(call make-install)

bin:
	$(call make-build-dir)
	$(call pre-make-config)
	$(AT)$(MAKE) -C $(SRC_DIR) bin
	$(call make-bin)

lib:
	$(call make-build-dir)
	$(call pre-make-config)
	$(AT)$(MAKE) -C $(SRC_DIR) lib
	$(call make-lib)

test: lib
	$(AT)$(MAKE) -C $(TEST_DIR) all

cmd:
	$(call make-build-dir)
	$(call pre-make-config)
	$(AT)[ -e $(CMD_FILE) ] || echo "[]" > $(CMD_FILE)
	$(AT)$(MAKE) -C $(SRC_DIR) cmd
	$(AT)$(MAKE) -C $(TEST_DIR) cmd

doc:
	$(call make-build-dir)
	$(call make-doc)

clean:
	$(call make-clean)

uninstall:
	$(call make-uninstall)

info:
	@echo "Available make targets:"
	@echo "  all: make lib, bin"
	@echo "  lib: make libraries"
	@echo "  bin: make executables"
	@echo "  test: make lib, test and run test"
	@echo "  cmd: generate $(BUILD_DIR)/compile_commands.json for clang tooling;"
	@echo "       use 'cmd' before 'all/lib/bin/test' for the initial generation"
	@echo "  doc: make documents"
	@echo "  install: install to $(INSTALL_DIR)"
	@echo "  clean: remove all the files generated by the build"
	@echo "  uninstall: uninstall installed files"
	@echo "  info/help: show this infomation"

help: info
