CFLAGS := $(call uniq,$(CFLAGS))
CXXFLAGS := $(call uniq,$(CXXFLAGS))
CUFLAGS := $(call uniq,$(CUFLAGS))

DEP = $(patsubst %.o,%.d,$(OBJS))

ifeq ($(GEN_CMD_FILE), yes)
make-cmd-file = $(AT)[ -e $@ ] || echo "[]" > $@
endif

.PHONY: cmd clean

$(CMD_FILE):
	$(call make-cmd-file)

$(OBJS): | $(CMD_FILE)

cmd: $(OBJS)

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

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP)
endif
