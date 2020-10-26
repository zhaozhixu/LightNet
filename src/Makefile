include ../config.mk
include ../common.mk

TOOLS_DIR := ../$(TOOLS_DIR)
BUILD_DIR := ../$(BUILD_DIR)

INCPATHS += -I$(BUILD_DIR)/include/$(TARGET)
CFLAGS += -fPIC -fvisibility=hidden
CXXFLAGS += -fPIC -fvisibility=hidden
LDFLAGS_SO += $(LDFLAGS) -shared
CUFLAGS += --compiler-options '-fPIC -fvisibility=hidden' -shared
ifeq ($(UNAME_S),Linux)
LDFLAGS_SO += -Wl,--no-undefined
CUFLAGS += --linker-options '-Wl,--no-undefined -shared'
else ifeq ($(UNAME_S),Darwin)
LDFLAGS_SO += -Wl,-undefined,error
CUFLAGS += --linker-options '-Wl,-undefined,error'
endif
CFLAGS += $(SRC_EXTRA_FLAGS)

SRC = $(SRC_FILES)
DEP = $(SRC_DEP_FILES)

.PHONY: all bin liba libso cmd clean

all: bin liba libso

bin: $(OBJDIR)/$(BIN)

liba: $(OBJDIR)/$(LIBTARGET_A)

libso: $(OBJDIR)/$(LIBTARGET_SO)

cmd: $(OBJS)

$(OBJDIR)/$(BIN): $(OBJS)
	$(call ld-bin)

$(OBJDIR)/$(LIBTARGET_SO): $(OBJS)
	$(call ld-so)

$(OBJDIR)/$(LIBTARGET_A): $(OBJS)
	$(call ar-a)

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
-include $(DEP)
endif
