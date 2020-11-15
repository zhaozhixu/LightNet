include ../config.mk
BUILDTOOLS_DIR := ../$(BUILDTOOLS_DIR)
include $(BUILDTOOLS_DIR)/common.mk
BUILD_DIR := ../$(BUILD_DIR)

SRC = $(SRC_FILES)
REQUIRES = $(SRC_REQUIRES)
CFLAGS += $(SRC_EXTRA_CFLAGS)

ifeq ("$(origin P)", "command line")
export LDFLAGS += $(abspath $(OBJDIR))/$(LIBTARGET_A)
export CFLAGS CXXFLAGS CUFLAGS LDFLAGS_SO
endif

.PHONY: all bin lib liba libso ext-plugin

all: bin lib

bin: $(OBJDIR)/$(BIN)

lib: liba libso

liba: $(OBJDIR)/$(LIBTARGET_A)

libso: $(OBJDIR)/$(LIBTARGET_SO)

$(OBJDIR)/$(BIN): $(OBJS)
	$(call ld-bin)

$(OBJDIR)/$(LIBTARGET_SO): $(OBJS)
	$(call ld-so)

$(OBJDIR)/$(LIBTARGET_A): $(OBJS)
	$(call ar-a)

ext-plugin:
	$(AT)$(MAKE) -C $(P)

ext-plugin-cmd:
	$(AT)$(MAKE) -C $(P) cmd

include $(BUILDTOOLS_DIR)/common_recipe.mk
