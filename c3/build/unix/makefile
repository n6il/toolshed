# NOTE: "opt" (the original optimiser) is intentionally excluded from the
# default build.  Its source directory is incomplete: inst.c -- which defines
# the instruction-list routines (remove, insins, insref, newins, insinit, ...)
# -- is absent from the tree, so opt cannot link (its own makefile already
# comments inst.o out of OBJS).  It also #includes the non-existent
# ../rma/ctype.h.  The complete, working optimiser is "optx" (built from the
# optexp/ sources), which is included below.  To work on opt, build it directly
# with "make -C build/unix/opt" once inst.c has been restored.
dirs	= exec prep comp comp1 comp2 compx optx lorder lsplit make

# Make all components
all:
	@echo "**************************************************"
	@echo "*                                                *"
	@echo "*               MW C Compiler                    *"
	@echo "*                                                *"
	@echo "**************************************************"
	$(foreach dir, $(dirs), (cd $(dir); make);)

# Clean all components
clean:
	$(foreach dir, $(dirs), (cd $(dir); make clean);)

# Install all compontents
install:
	$(foreach dir, $(dirs), (cd $(dir); make install);)
