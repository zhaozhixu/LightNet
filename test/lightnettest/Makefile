include ../../config.mk
BUILDTOOLS_DIR := ../../$(BUILDTOOLS_DIR)
include $(BUILDTOOLS_DIR)/common.mk
BUILD_DIR := ../../$(BUILD_DIR)

SRC = $(wildcard *.c)
REQUIRES = check

LIB_TEST := liblightnettest.a

.PHONY: all lib

all: lib

lib: $(OBJDIR)/$(LIB_TEST)

$(OBJDIR)/$(LIB_TEST): $(OBJS)
	$(call ar-a)

include $(BUILDTOOLS_DIR)/common_recipe.mk
