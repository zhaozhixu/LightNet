CC ?= gcc
CXX ?= g++
CUCC ?= nvcc
AR = ar cr
ECHO = @echo
SHELL = /bin/sh

ifndef VERBOSE
AT = @
endif

CFLAGS += -Wall -std=gnu99 -D_GNU_SOURCE
CXXFLAGS += -std=c++11 -Wall
CUFLAGS += -m64 -arch=sm_30 -use_fast_math -ccbin $(CXX)

ifeq ($(DEBUG), yes)
CFLAGS += -g -O0 -D$(ABBR)_DEBUG
CXXFLAGS += -g -O0 -D$(ABBR)_DEBUG
CUFLAGS += -lineinfo
else
CFLAGS += -O2
CXXFLAGS += -O2
CUFLAGS += -O2
endif

INCPATHS += -I/usr/local/include -I. -I$(CURDIR)
LDFLAGS += -L/usr/local/lib -lm
# cannot use ifeq/ifneq because they expand immediately
INCPATHS += $(if $(REQUIRES),$(shell pkg-config --cflags '$(REQUIRES)'))
LDFLAGS += $(if $(REQUIRES),$(shell pkg-config --libs '$(REQUIRES)'))

NORMAL_SRC = $(filter-out %cuda.c %cuda.cc %cuda.cpp %cudnn.c %cudnn.cc %cudnn.cpp %tensorrt.c %tensorrt.cc %tensorrt.cpp %dpu.c %dpu.cc %dpu.cpp %.cu,$(SRC))
CUDA_SRC = $(filter %cuda.c %cuda.cc %cuda.cpp %.cu,$(SRC))
CUDNN_SRC = $(filter %cudnn.c %cudnn.cc %cudnn.cpp,$(SRC))
TENSORRT_SRC = $(filter %tensorrt.c %tensorrt.cc %tensorrt.cpp,$(SRC))
DPU_SRC = $(filter %dpu.c %dpu.cc %dpu.cpp,$(SRC))

# OBJDIR is BUILD_DIR/{directories from project root to current makefile}
# project cannot contain subdirectories that have spaces in names to make this code work
cur_dirs = $(subst /, ,$(abspath .))
build_dirs = $(subst /, ,$(abspath $(BUILD_DIR)))
common_dirs = $(foreach dir,$(cur_dirs),$(if $(findstring $(dir),$(build_dirs)),$(dir)))
space := $(subst ,, )
OBJDIR = $(BUILD_DIR)/$(subst $(space),/,$(wordlist $(words $(common_dirs) 1),$(words $(cur_dirs)),$(cur_dirs)))
OBJS   = $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(NORMAL_SRC)))
OBJS  += $(patsubst %.cc,$(OBJDIR)/%.o,$(filter %.cc,$(NORMAL_SRC)))
OBJS  += $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(NORMAL_SRC)))

ifeq ($(WITH_CUDA), yes)
CFLAGS += -D$(ABBR)_CUDA
CXXFLAGS += -D$(ABBR)_CUDA
CUFLAGS += -D$(ABBR)_CUDA
CUDA_INSTALL_DIR ?= /usr/local/cuda
INCPATHS += -I$(CUDA_INSTALL_DIR)/include
LDFLAGS += -L$(CUDA_INSTALL_DIR)/lib64 -lcudart -lcublas -lcurand -lstdc++
OBJS  += $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(CUDA_SRC)))
OBJS  += $(patsubst %.cc,$(OBJDIR)/%.o,$(filter %.cc,$(CUDA_SRC)))
OBJS  += $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(CUDA_SRC)))
OBJS  += $(patsubst %.cu,$(OBJDIR)/%.o,$(filter %.cu,$(CUDA_SRC)))
ifeq ($(WITH_CUDNN), yes)
CFLAGS += -D$(ABBR)_CUDNN
CXXFLAGS += -D$(ABBR)_CUDNN
CUFLAGS += -D$(ABBR)_CUDNN
CUDNN_INSTALL_DIR ?= /usr/local/cuda
INCPATHS += -I$(CUDNN_INSTALL_DIR)/include
LDFLAGS += -L$(CUDNN_INSTALL_DIR)/lib -lcudnn
OBJS  += $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(CUDNN_SRC)))
OBJS  += $(patsubst %.cc,$(OBJDIR)/%.o,$(filter %.cc,$(CUDNN_SRC)))
OBJS  += $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(CUDNN_SRC)))
endif
ifeq ($(WITH_TENSORRT), yes)
CFLAGS += -D$(ABBR)_TENSORRT
CXXFLAGS += -D$(ABBR)_TENSORRT -Wno-deprecated-declarations
CUFLAGS += -D$(ABBR)_TENSORRT
TENSORRT_INSTALL_DIR ?= /usr
INCPATHS += -I$(TENSORRT_INSTALL_DIR)/include
LDFLAGS += -L$(TENSORRT_INSTALL_DIR)/lib -lnvinfer -lnvinfer_plugin
OBJS  += $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(TENSORRT_SRC)))
OBJS  += $(patsubst %.cc,$(OBJDIR)/%.o,$(filter %.cc,$(TENSORRT_SRC)))
OBJS  += $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(TENSORRT_SRC)))
endif
endif

ifeq ($(WITH_DPU), yes)
CFLAGS += -D$(ABBR)_DPU
CXXFLAGS += -D$(ABBR)_DPU
CUFLAGS += -D$(ABBR)_DPU
OBJS  += $(patsubst %.c,$(OBJDIR)/%.o,$(filter %.c,$(DPU_SRC)))
OBJS  += $(patsubst %.cc,$(OBJDIR)/%.o,$(filter %.cc,$(DPU_SRC)))
OBJS  += $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(DPU_SRC)))
endif

ifeq ($(WITH_PLUGIN), yes)
CFLAGS += -D$(ABBR)_PLUGIN
CXXFLAGS += -D$(ABBR)_PLUGIN
CUFLAGS += -D$(ABBR)_PLUGIN
endif

CFLAGS += $(INCPATHS)
CXXFLAGS += $(INCPATHS)
CUFLAGS += $(INCPATHS)

VISIBLE := -fvisibility=hidden
CFLAGS += -fPIC $(VISIBLE)
CXXFLAGS += -fPIC $(VISIBLE)
CUFLAGS += --compiler-options '-fPIC $(VISIBLE)' -shared
LDFLAGS_SO += $(LDFLAGS) -shared
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
LDFLAGS_SO += -Wl,--no-undefined
CUFLAGS += --linker-options '-Wl,--no-undefined -shared'
else ifeq ($(UNAME_S),Darwin)
LDFLAGS_SO += -Wl,-undefined,error
CUFLAGS += --linker-options '-Wl,-undefined,error'
endif

uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

define concat
$1$2$3$4$5$6$7$8
endef

define make-depend-c
$(AT)$(CC) -MM -MF $(subst .o,.d,$@) -MP -MT $@ $(CFLAGS) $<
endef

define make-depend-cxx
$(AT)$(CXX) -MM -MF $(subst .o,.d,$@) -MP -MT $@ $(CXXFLAGS) $<
endef

define make-depend-cu
$(AT)$(CUCC) -M $(CUFLAGS) $< > $(subst .o,.d,$@).$$$$; \
sed 's,.*\.o[ :]*,$@ : ,g' < $(subst .o,.d,$@).$$$$ > $(subst .o,.d,$@); \
rm -f $(subst .o,.d,$@).$$$$
endef

GEN_CMD_FILE := no
ifeq ($(MAKECMDGOALS), cmd)
GEN_CMD_FILE := yes
endif
CMD_FILE ?= $(BUILD_DIR)/compile_commands.json

ifeq ($(GEN_CMD_FILE), no)
define compile-c
$(ECHO) "  CC\t" $@
$(call make-depend-c)
$(AT)$(CC) $(CFLAGS) -c -o $@ $<
endef
else
define compile-c
$(ECHO) "  GEN\t" $(CMD_FILE) for $@
$(AT)$(BUILDTOOLS_DIR)/gen_compile_commands.pl -f $(CMD_FILE) `pwd` $< "$(CC) $(CFLAGS) -c -o $@ $<"
endef
endif

ifeq ($(GEN_CMD_FILE), no)
define compile-cxx
$(ECHO) "  CXX\t" $@
$(call make-depend-cxx)
$(AT)$(CXX) $(CXXFLAGS) -c -o $@ $<
endef
else
define compile-cxx
$(ECHO) "  GEN\t" $(CMD_FILE) for $@
$(AT)$(BUILDTOOLS_DIR)/gen_compile_commands.pl -f $(CMD_FILE) `pwd` $< "$(CXX) $(CXXFLAGS) -c -o $@ $<"
endef
endif

ifeq ($(GEN_CMD_FILE), no)
define compile-cu
$(ECHO) "  CUCC\t" $@
$(call make-depend-cu)
$(AT)$(CUCC) $(CUFLAGS) -c -o $@ $<
endef
else
define compile-cu
$(ECHO) "  GEN\t" $(CMD_FILE) for $@
$(AT)$(BUILDTOOLS_DIR)/gen_compile_commands.pl -f $(CMD_FILE) `pwd` $< "$(CUCC) $(CUFLAGS) -c -o $@ $<"
endef
endif

define ld-bin
$(ECHO) "  LD\t" $@
$(AT)$(CC) -o $@ $^ $(BEFORE_LDFLAGS) $(LDFLAGS)
endef

define ld-so
$(ECHO) "  LD\t" $@
$(AT)$(CC) -o $@ $^ $(BEFORE_LDFLAGS) $(LDFLAGS_SO)
endef

define ar-a
$(ECHO) "  AR\t" $@
$(AT)$(AR) $@ $^
endef
