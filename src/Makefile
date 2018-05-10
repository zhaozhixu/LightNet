.SUFFIXES:
TARGET = lightnet
CC = gcc
CXX = g++
CUCC = nvcc

CFLAGS = -Wall
CXXFLAGS = -std=c++11 -Wall
CUFLAGS = -m64 -arch=sm_35 -ccbin $(CXX)
LDFLAGS = $(CFLAGS)

ifdef DEBUG
CFLAGS += -g -O0 -DLN_DEBUG
CXXFLAGS += -g -O0 -DLN_DEBUG
CUFLAGS += -lineinfo
LDFLAGS += -g -O0
else
# CFLAGS += -O3 -DNDEBUG
# CXXFLAGS += -O3 -DNDEBUG
CFLAGS += -O3
CXXFLAGS += -O3
CUFLAGS +=
LDFLAGS += -O3
endif

ifeq "$(MAKECMDGOALS)" "libso"
CFLAGS += -fPIC
CUFLAGS += --compiler-options '-fPIC' --linker-options '-Wl,--no-undefined -shared' -shared
LDFLAGS += -shared -Wl,--no-undefined
LIBTARGET = lib$(TARGET).so
endif

ifdef VERBOSE
AT =
else
AT = @
endif

AR = ar cr
ECHO = @echo
SHELL = /bin/sh

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

SRCS = *.cpp *.cc *.c *.cu
OUTDIR = .
OBJDIR = $(call concat,$(OUTDIR),/obj)
OBJS   = $(patsubst %.c, $(OBJDIR)/%.o, $(wildcard *.c))
OBJS  += $(patsubst %.cpp, $(OBJDIR)/%.o, $(wildcard *.cpp))
OBJS  += $(patsubst %.cc, $(OBJDIR)/%.o, $(wildcard *.cc))
CUOBJS = $(patsubst %.cu, $(OBJDIR)/%.o, $(wildcard *.cu))

TRIPLE?=x86_64-linux
CUDA_INSTALL_DIR = /usr/local/cuda-8.0
CUDA_LIBDIR = lib
INCPATHS    =-I"$(CUDA_INSTALL_DIR)/include" -I"/usr/local/include"
LIBPATHS    =-L"$(CUDA_INSTALL_DIR)/targets/$(TRIPLE)/$(CUDA_LIBDIR)" -L"/usr/local/lib" -L"/usr/local/cuda/lib64" -L"$(CUDA_INSTALL_DIR)/$(CUDA_LIBDIR)"
LIBS = $(LIBPATHS) -lcudart -lcudart_static -lm
CFLAGS += $(INCPATHS)
CXXFLAGS += $(INCPATHS)
CUFLAGS += $(INCPATHS)
LDFLAGS += $(LIBS)

.PHONY: all libso
all: $(OUTDIR)/$(TARGET)
libso: $(OUTDIR)/$(LIBTARGET)

$(OUTDIR)/$(LIBTARGET): $(OBJS) $(CUOBJS)
	$(ECHO) Linking: $^
	$(AT)$(CC) -o $@ $^ $(LDFLAGS)

$(OUTDIR)/$(TARGET): $(OBJS) $(CUOBJS)
	$(ECHO) Linking: $^
	$(AT)$(CC) -o $@ $^ $(LDFLAGS)

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
