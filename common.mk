CC ?= gcc
CXX ?= g++
CUCC ?= nvcc
AR = ar cr
ECHO = @echo
SHELL = /bin/sh

ifdef VERBOSE
AT =
else
AT = @
endif

TOOLS_DIR ?= tools

CFLAGS = -Wall -std=gnu99 -D_GNU_SOURCE
CXXFLAGS = -std=c++11 -Wall
CUFLAGS = -m64 -arch=sm_30 -use_fast_math -ccbin $(CXX)
LDFLAGS = $(CFLAGS)

ifeq ($(DEBUG), yes)
CFLAGS += -g -O0 -DLN_DEBUG
CXXFLAGS += -g -O0 -DLN_DEBUG
CUFLAGS += -lineinfo
LDFLAGS += -g -O0
else
CFLAGS += -O2
CXXFLAGS += -O2
CUFLAGS += -O2
LDFLAGS += -O2
endif

INCPATHS = -I/usr/local/include -I. `pkg-config --cflags tensorlight`
LDFLAGS += -L/usr/local/lib -lm `pkg-config --libs tensorlight`

ifeq ($(WITH_CUDA), yes)
CFLAGS += -DLN_CUDA -DTL_CUDA
CXXFLAGS += -DLN_CUDA -DTL_CUDA
CUFLAGS += -DLN_CUDA -DTL_CUDA
CUDA_INSTALL_DIR ?= /usr/local/cuda
INCPATHS += -I$(CUDA_INSTALL_DIR)/include
LDFLAGS += -L$(CUDA_INSTALL_DIR)/lib64 -lcuda -lcudart -lcublas -lcurand -lstdc++
ifeq ($(WITH_CUDNN), yes)
CFLAGS += -DLN_CUDNN
CXXFLAGS += -DLN_CUDNN
CUFLAGS += -DLN_CUDNN
CUDNN_INSTALL_DIR ?= /usr/local/cuda
INCPATHS += -I$(CUDNN_INSTALL_DIR)/include
LDFLAGS += -L$(CUDNN_INSTALL_DIR)/lib -lcudnn
endif
ifeq ($(WITH_TENSORRT), yes)
CFLAGS += -DLN_TENSORRT
CXXFLAGS += -DLN_TENSORRT -Wno-deprecated-declarations
CUFLAGS += -DLN_TENSORRT
TENSORRT_INSTALL_DIR ?= /usr
INCPATHS += -I$(TENSORRT_INSTALL_DIR)/include
LDFLAGS += -L$(TENSORRT_INSTALL_DIR)/lib -lnvinfer -lnvinfer_plugin
endif
endif

ifeq ($(WITH_DPU), yes)
CFLAGS += -DLN_DPU
CXXFLAGS += -DLN_DPU
CUFLAGS += -DLN_DPU
endif

CFLAGS += $(INCPATHS)
CXXFLAGS += $(INCPATHS)
CUFLAGS += $(INCPATHS)

define concat
$1$2$3$4$5$6$7$8
endef

# $(call make-depend,source-file,object-file,depend-file)
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
$(AT)$(TOOLS_DIR)/gen_compile_commands.pl -f $(CMD_FILE) `pwd` $< "$(CC) $(CFLAGS) -c -o $@ $<"
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
$(AT)$(TOOLS_DIR)/gen_compile_commands.pl -f $(CMD_FILE) `pwd` $< "$(CXX) $(CXXFLAGS) -c -o $@ $<"
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
$(AT)$(TOOLS_DIR)/gen_compile_commands.pl -f $(CMD_FILE) `pwd` $< "$(CUCC) $(CUFLAGS) -c -o $@ $<"
endef
endif

define ld-bin
$(ECHO) "  LD\t" $@
$(AT)$(CC) -o $@ $^ $(LDFLAGS)
endef

define ld-so
$(ECHO) "  LD\t" $@
$(AT)$(CC) -o $@ $^ $(LDFLAGS_SO)
endef

define ar-a
$(ECHO) "  AR\t" $@
$(AT)$(AR) $@ $^
endef
