include ../config.mk
BUILDTOOLS_DIR := ../$(BUILDTOOLS_DIR)
include $(BUILDTOOLS_DIR)/common.mk
BUILD_DIR := ../$(BUILD_DIR)

SRC = $(TEST_FILES)
DEP = $(TEST_DEP_FILES)
REQUIRES = $(TEST_REQUIRES) $(SRC_REQUIRES)
CFLAGS += $(TEST_EXTRA_CFLAGS)

TARGET_TEST := test_$(TARGET)

include $(BUILDTOOLS_DIR)/common_recipe.mk

.PHONY: all bin run

all: run

run: bin
	$(ECHO) Running tests...
	$(AT)$(OBJDIR)/$(TARGET_TEST) $(TEST_ARGS)

bin: $(OBJDIR)/$(TARGET_TEST)

$(OBJDIR)/$(TARGET_TEST): $(OBJS) $(BUILD_DIR)/$(SRC_DIR)/$(LIBTARGET_A)
	$(call ld-bin)
