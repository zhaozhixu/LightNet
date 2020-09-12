include ../config.mk
include ../common.mk

TARGET ?= lightnet
TARGET_TEST = test_$(TARGET)
LIBTARGET_A = lib$(TARGET).a

TOOLS_DIR := ../$(TOOLS_DIR)
BUILD_DIR := ../$(BUILD_DIR)
SRCOBJ = $(BUILD_DIR)/src/$(LIBTARGET_A)
TEST_RESULT_DIR = `realpath $(OBJDIR)/result`
TEST_DATA_DIR = `realpath data`
TEST_UTIL_DIR = lightnettest

INCPATHS += -I../src -I../src/arch -I../src/op `pkg-config --cflags check`
LDFLAGS += `pkg-config --libs check` -lpthread

CFLAGS += -DLN_TEST_RESULT_DIR="\"$(TEST_RESULT_DIR)\"" -DLN_TEST_DATA_DIR="\"$(TEST_DATA_DIR)\""
CFLAGS += $(INCPATHS)
CXXFLAGS += $(INCPATHS)
CUFLAGS += $(INCPATHS)

SRC = $(wildcard *.c)
SRC += $(wildcard *.cpp)
SRC += $(wildcard *.cc)
SRC += $(wildcard *.cu)
SRC += $(wildcard $(TEST_UTIL_DIR)/*.c)
NORMAL_SRC = $(filter-out %cuda.c %cuda.cu %cudnn.c %cudnn.cu,$(SRC))
CUDA_SRC = $(filter %cuda.c %cuda.cu,$(SRC))
CUDNN_SRC = $(filter %cudnn.c %cudnn.cu,$(SRC))

OBJDIR = $(BUILD_DIR)/test
OBJS   = $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(NORMAL_SRC)))
OBJS  += $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(NORMAL_SRC)))
OBJS  += $(patsubst %.cc,$(OBJDIR)/%.o,$(filter %.cc,$(NORMAL_SRC)))
ifeq ($(WITH_CUDA), yes)
OBJS  += $(patsubst %.cu,$(OBJDIR)/%.o,$(filter %.cu,$(CUDA_SRC)))
OBJS  += $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(CUDA_SRC)))
ifeq ($(WITH_CUDNN), yes)
OBJS  += $(patsubst %.cu,$(OBJDIR)/%.o,$(filter %.cu,$(CUDNN_SRC)))
OBJS  += $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(CUDNN_SRC)))
endif
endif

define make-obj-dir
$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR)/$(TEST_UTIL_DIR); fi
endef

.PHONY: all cmd clean

all: $(OBJDIR)/$(TARGET_TEST)
	$(ECHO) Running tests...
	$(AT)if [ ! -d $(TEST_RESULT_DIR) ]; then mkdir -p $(TEST_RESULT_DIR); fi
	$(AT)$(OBJDIR)/$(TARGET_TEST) $(TEST_ARGS)

cmd: $(OBJS)

$(OBJDIR)/$(TARGET_TEST): $(OBJS) $(SRCOBJ)
	$(call ld-bin)

$(OBJDIR)/%.o: %.c
	$(call make-obj-dir)
	$(call compile-c)

$(OBJDIR)/%.o: %.cpp
	$(call make-obj-dir)
	$(call compile-cxx)

$(OBJDIR)/%.o: %.cc
	$(call make-obj-dir)
	$(call compile-cxx)

$(OBJDIR)/%.o: %.cu
	$(call make-obj-dir)
	$(call compile-cu)

clean:
	rm -rf $(OBJDIR)

ifneq "$(MAKECMDGOALS)" "clean"
-include $(OBJDIR)/*.d
-include $(OBJDIR)/$(TEST_UTIL_DIR)/*.d
endif
