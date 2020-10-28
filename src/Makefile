include ../config.mk
BUILDTOOLS_DIR := ../$(BUILDTOOLS_DIR)
include $(BUILDTOOLS_DIR)/common.mk
BUILD_DIR := ../$(BUILD_DIR)

SRC = $(SRC_FILES)
DEP = $(SRC_DEP_FILES)
REQUIRES = $(SRC_REQUIRES)
CFLAGS += $(SRC_EXTRA_CFLAGS)

include $(BUILDTOOLS_DIR)/common_recipe.mk

.PHONY: all bin lib liba libso

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
