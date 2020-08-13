include ../config.mk
include ../common.mk

TARGET ?= lightnet
TARGET_BIN = $(TARGET)
LIBTARGET_A = lib$(TARGET).a
LIBTARGET_SO = lib$(TARGET).so

BUILD_DIR := ../$(BUILD_DIR)
INCPATHS += -Iarch -Iop -I$(BUILD_DIR)/include/$(TARGET)
CFLAGS += -fPIC -fvisibility=hidden
CXXFLAGS += -fPIC -fvisibility=hidden
CUFLAGS += --compiler-options '-fPIC -fvisibility=hidden' --linker-options '-Wl,--no-undefined -shared' -shared
LDFLAGS_SO = $(LDFLAGS) -shared -Wl,--no-undefined

SRC = $(wildcard *.c)
SRC += $(wildcard *.cpp)
SRC += $(wildcard *.cc)
SRC += $(wildcard *.cu)
SRC += $(wildcard op/*.c)
SRC += $(wildcard op/*.cc)
SRC += $(wildcard op/*.cu)
SRC += $(wildcard op/auto/*.c)
SRC += $(wildcard op/auto/*.cc)
SRC += $(wildcard op/auto/*.cu)
SRC += $(wildcard arch/*.c)
SRC += $(wildcard arch/*.cc)
SRC += $(wildcard arch/*.cu)
SRC += $(wildcard arch/auto/*.c)
SRC += $(wildcard arch/auto/*.cc)
SRC += $(wildcard arch/auto/*.cu)

NORMAL_SRC = $(filter-out %cuda.c %cuda.cu %cudnn.c %cudnn.cu %tensorrt.c %tensorrt.cc %dpu.c %dpu.cc,$(SRC))
CUDA_SRC = $(filter %cuda.c %cuda.cu,$(SRC))
CUDNN_SRC = $(filter %cudnn.c %cudnn.cu,$(SRC))
TENSORRT_SRC = $(filter %tensorrt.c %tensorrt.cc,$(SRC))
DPU_SRC = $(filter %dpu.c %dpu.cc,$(SRC))

OBJDIR = $(BUILD_DIR)/src
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
ifeq ($(WITH_TENSORRT), yes)
OBJS  += $(patsubst %.cc,$(OBJDIR)/%.o,$(filter %.cc,$(TENSORRT_SRC)))
OBJS  += $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(TENSORRT_SRC)))
endif
ifeq ($(WITH_DPU), yes)
OBJS  += $(patsubst %.cc,$(OBJDIR)/%.o,$(filter %.cc,$(DPU_SRC)))
OBJS  += $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(DPU_SRC)))
endif
endif

define make-obj-dir
$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR)/op/auto; \
mkdir -p $(OBJDIR)/arch/auto; fi
endef

.PHONY: all bin liba libso clean

all: bin liba libso

bin: $(OBJDIR)/$(TARGET_BIN)

liba: $(OBJDIR)/$(LIBTARGET_A)

liba: $(OBJDIR)/$(LIBTARGET_SO)

$(OBJDIR)/$(TARGET_BIN): $(OBJS)
	$(ECHO) Linking: $@
	$(AT)$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/$(LIBTARGET_SO): $(OBJS)
	$(ECHO) Linking: $@
	$(AT)$(CC) -o $@ $^ $(LDFLAGS_SO)

$(OBJDIR)/$(LIBTARGET_A): $(OBJS)
	$(ECHO) Packing: $@
	$(AT)$(AR) $@ $^

$(OBJDIR)/%.o: %.c
	$(call make-obj-dir)
	$(call make-depend-c,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling: $@
	$(AT)$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp
	$(call make-obj-dir)
	$(call make-depend-cxx,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling: $@
	$(AT)$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cc
	$(call make-obj-dir)
	$(call make-depend-cxx,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling: $@
	$(AT)$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cu
	$(call make-obj-dir)
	$(call make-depend-cu,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling CUDA: $@
	$(AT)$(CUCC) $(CUFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJDIR)

ifneq "$(MAKECMDGOALS)" "clean"
-include $(OBJDIR)/*.d
-include $(OBJDIR)/op/*.d
-include $(OBJDIR)/op/auto/*.d
-include $(OBJDIR)/arch/*.d
-include $(OBJDIR)/arch/auto/*.d
endif
