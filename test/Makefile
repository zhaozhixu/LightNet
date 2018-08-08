include ../config.mk

TARGET ?= lightnet
TARGET_TEST = test_$(TARGET)

CC = gcc
CXX = g++
CUCC = nvcc
AR = ar cr
ECHO = @echo
SHELL = /bin/sh

ifdef VERBOSE
AT =
else
AT = @
endif

CFLAGS = -Wall
CXXFLAGS = -std=c++11 -Wall
CUFLAGS = -m64 -arch=sm_30 -ccbin $(CC)
LDFLAGS = $(CFLAGS)

ifeq ($(DEBUG), yes)
CFLAGS += -g -O0 -DLN_DEBUG
CXXFLAGS += -g -O0 -DLN_DEBUG
CUFLAGS += -lineinfo -O0
LDFLAGS += -g -O0
else
CFLAGS += -O3
CXXFLAGS += -O3
CUFLAGS += -O3
LDFLAGS += -O3
endif

INCPATHS = -I/usr/local/include `pkg-config --cflags check tensorlight`
LDFLAGS += -L/usr/local/lib -lm `pkg-config --libs check tensorlight` -lpthread

ifeq ($(WITH_CUDA), yes)
CFLAGS += -DLN_CUDA
CXXFLAGS += -DLN_CUDA
CUFLAGS += -DLN_CUDA
CUDA_INSTALL_DIR ?= /usr/local/cuda
INCPATHS += -I$(CUDA_INSTALL_DIR)/include
LDFLAGS += -L$(CUDA_INSTALL_DIR)/lib64 -lcuda -lcudart -lcublas -lcurand
ifeq ($(WITH_CUDNN), yes)
CFLAGS += -DLN_CUDNN
CXXFLAGS += -DLN_CUDNN
CUFLAGS += -DLN_CUDNN
LDFLAGS += -lcudnn
endif
endif

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
endif

SRCOBJS = $(filter-out %$(TARGET).o,$(wildcard ../src/obj/*.o))

define concat
  $1$2$3$4$5$6$7$8
endef

#$(call make-depend,source-file,object-file,depend-file)
define make-depend-c
  $(AT)$(CC) -MM -MF $3 -MP -MT $2 $(CFLAGS) $1
endef

define make-depend-cxx
  $(AT)$(CXX) -MM -MF $3 -MP -MT $2 $(CFLAGS) $1
endef

define make-depend-cu
  $(AT)$(CUCC) -M $(CUFLAGS) $1 > $3.$$$$; \
  sed 's,.*\.o[ :]*,$2 : ,g' < $3.$$$$ > $3; \
  rm -f $3.$$$$
endef

.PHONY: all clean
all: $(TARGET_TEST)
	$(ECHO) Running tests...
	$(AT)./$(TARGET_TEST)

$(TARGET_TEST): $(OBJS)
	$(ECHO) Linking: $^ $(SRCOBJS)
	$(AT)$(CC) -o $@ $^ $(SRCOBJS) $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi
	$(call make-depend-c,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling: $<
	$(AT)$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp
	$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi
	$(call make-depend-cxx,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling: $<
	$(AT)$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cc
	$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi
	$(call make-depend-cxx,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling: $<
	$(AT)$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cu
	$(AT)if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi
	$(call make-depend-cu,$<,$@,$(subst .o,.d,$@))
	$(ECHO) Compiling CUDA: $<
	$(AT)$(CUCC) $(CUFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJDIR)

ifneq "$(MAKECMDGOALS)" "clean"
-include $(OBJDIR)/*.d
endif
