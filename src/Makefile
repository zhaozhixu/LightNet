include ../config.mk
include ../common.mk

TARGET ?= lightnet
TARGET_BIN = $(TARGET)

SRC = $(wildcard *.c)
SRC += $(wildcard *.cpp)
SRC += $(wildcard *.cc)
SRC += $(wildcard *.cu)
SRC += $(wildcard op/*.c)
NORMAL_SRC = $(filter-out %cuda.c %cuda.cu %cudnn.c %cudnn.cu %tensorrt.c %tensorrt.cc,$(SRC))
CUDA_SRC = $(filter %cuda.c %cuda.cu,$(SRC))
CUDNN_SRC = $(filter %cudnn.c %cudnn.cu,$(SRC))
TENSORRT_SRC = $(filter %tensorrt.c %tensorrt.cc,$(SRC))

OBJDIR = obj
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
endif

.PHONY: all bin clean
all: bin

bin: $(OBJDIR)/$(TARGET_BIN)

$(OBJDIR)/$(TARGET_BIN): $(OBJS)
	$(ECHO) Linking: $@
	$(AT)$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR)/op; fi
	$(call make-depend-c,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling: $<
	$(AT)$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp
	$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR)/op; fi
	$(call make-depend-cxx,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling: $<
	$(AT)$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cc
	$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR)/op; fi
	$(call make-depend-cxx,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling: $<
	$(AT)$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cu
	$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR)/op; fi
	$(call make-depend-cu,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling CUDA: $<
	$(AT)$(CUCC) $(CUFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJDIR)

ifneq "$(MAKECMDGOALS)" "clean"
-include $(OBJDIR)/*.d
-include $(OBJDIR)/op/*.d
endif
