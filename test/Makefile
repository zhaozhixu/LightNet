include ../config.mk
BUILDTOOLS_DIR := ../$(BUILDTOOLS_DIR)
include $(BUILDTOOLS_DIR)/common.mk
BUILD_DIR := ../$(BUILD_DIR)

SRC = $(TEST_FILES)
REQUIRES = $(TEST_REQUIRES)
CFLAGS += $(TEST_EXTRA_CFLAGS)

TARGET_TEST := test_$(TARGET)
TEST_LIB_DIR := lightnettest
TEST_LIB := $(TEST_LIB_DIR)/liblightnettest.a
BEFORE_LDFLAGS += $(abspath $(BUILD_DIR))/$(SRC_DIR)/$(LIBTARGET_A) $(abspath $(OBJDIR))/$(TEST_LIB)

ifeq ("$(origin P)", "command line")
export LDFLAGS BEFORE_LDFLAGS
export CFLAGS CXXFLAGS CUFLAGS LDFLAGS_SO
endif

.PHONY: all bin run test-lib

all: run

run: bin
	$(ECHO) Running tests...
	$(AT)$(OBJDIR)/$(TARGET_TEST) -o $(OBJDIR)/test_output.xml $(TEST_ARGS)

bin: $(OBJDIR)/$(TARGET_TEST)

test-lib:
	$(AT)$(MAKE) -C $(TEST_LIB_DIR)

$(OBJDIR)/$(TEST_LIB): test-lib

$(OBJDIR)/$(TARGET_TEST): $(OBJS) $(OBJDIR)/$(TEST_LIB)
	$(call ld-bin)

cmd:
	$(AT)$(MAKE) -C $(TEST_LIB_DIR) cmd

ext-plugin-test:
	$(AT)$(MAKE) -C $(P)

ext-plugin-cmd:
	$(AT)$(MAKE) -C $(P) cmd

include $(BUILDTOOLS_DIR)/common_recipe.mk
