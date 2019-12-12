include ../config.mk
include ../common.mk

TARGET ?= lightnet
TARGET_TEST = test_$(TARGET)
LIBTARGET_A = lib$(TARGET).a

BUILD_DIR := ../$(BUILD_DIR)
SRCOBJ = $(BUILD_DIR)/src/$(LIBTARGET_A)
TEST_RESULT_DIR=$(OBJDIR)/result

INCPATHS += -I../src -I../src/arch -I../src/op `pkg-config --cflags check`
LDFLAGS += `pkg-config --libs check` -lpthread

CFLAGS += -DLN_TEST_RESULT_DIR="\"$(TEST_RESULT_DIR)\""
CFLAGS += $(INCPATHS)
CXXFLAGS += $(INCPATHS)
CUFLAGS += $(INCPATHS)

SRC = $(wildcard *.c)
SRC += $(wildcard *.cpp)
SRC += $(wildcard *.cc)
SRC += $(wildcard *.cu)
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

.PHONY: all clean

all: $(OBJDIR)/$(TARGET_TEST)
	$(ECHO) Running tests...
	$(AT)if [ ! -d $(TEST_RESULT_DIR) ]; then mkdir -p $(TEST_RESULT_DIR); fi
	$(AT)cp result/check_unittest.xslt $(TEST_RESULT_DIR)/check_unittest.xslt
	$(AT)$(OBJDIR)/$(TARGET_TEST)

$(OBJDIR)/$(TARGET_TEST): $(OBJS) $(SRCOBJ)
	$(ECHO) Linking: $@
	$(AT)$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi
	$(call make-depend-c,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling: $@
	$(AT)$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp
	$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi
	$(call make-depend-cxx,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling: $@
	$(AT)$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cc
	$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi
	$(call make-depend-cxx,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling: $@
	$(AT)$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cu
	$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi
	$(call make-depend-cu,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling CUDA: $@
	$(AT)$(CUCC) $(CUFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJDIR)

ifneq "$(MAKECMDGOALS)" "clean"
-include $(OBJDIR)/*.d
endif
