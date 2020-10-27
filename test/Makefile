include ../config.mk
include ../common.mk

TARGET_TEST = test_$(TARGET)

TOOLS_DIR := ../$(TOOLS_DIR)
BUILD_DIR := ../$(BUILD_DIR)
SRCOBJ := $(BUILD_DIR)/$(SRC_DIR)/$(LIBTARGET_A)
SRC_DIR := ../$(SRC_DIR)

INCPATHS += -I$(SRC_DIR)
CFLAGS += $(TEST_EXTRA_FLAGS)

SRC = $(TEST_FILES)
DEP = $(TEST_DEP_FILES)
REQUIRES = $(TEST_REQUIRES) $(SRC_REQUIRES)

.PHONY: all cmd clean

all: $(OBJDIR)/$(TARGET_TEST)
	$(ECHO) Running tests...
	$(AT)$(OBJDIR)/$(TARGET_TEST) $(TEST_ARGS)

cmd: $(OBJS)

$(OBJDIR)/$(TARGET_TEST): $(OBJS) $(SRCOBJ)
	$(call ld-bin)

$(OBJDIR)/%.o: %.c
	$(call compile-c)

$(OBJDIR)/%.o: %.cpp
	$(call compile-cxx)

$(OBJDIR)/%.o: %.cc
	$(call compile-cxx)

$(OBJDIR)/%.o: %.cu
	$(call compile-cu)

clean:
	rm -rf $(OBJDIR)

ifneq "$(MAKECMDGOALS)" "clean"
-include DEP
endif
