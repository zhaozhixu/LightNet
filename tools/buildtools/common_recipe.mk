.PHONY: cmd clean

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
