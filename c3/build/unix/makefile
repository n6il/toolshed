# NOTE: "opt" (the original optimiser) is intentionally excluded from the
# default build.  Its source directory is incomplete: inst.c -- which defines
# the instruction-list routines (remove, insins, insref, newins, insinit, ...)
# -- is absent from the tree, so opt cannot link (its own makefile already
# comments inst.o out of OBJS).  It also #includes the non-existent
# ../rma/ctype.h.  The complete, working optimiser is "optx" (built from the
# optexp/ sources), which is included below.  To work on opt, build it directly
# with "make -C build/unix/opt" once inst.c has been restored.
dirs	= exec prep comp comp1 comp2 compx optx lorder lsplit make

# This is the c3-only BUILD tree (all / clean).  Installation is owned by the
# top-level ToolShed installer in ../../../build/unix, which installs c3 along
# with the rest of ToolShed (binaries + lwasm/lwlink back end to /usr/local/bin,
# the preprocessor headers to $(C3BASE)/defs) and provides DESTDIR staging and a
# `package` tarball.  Keeping one installer avoids the two diverging.
TOOLSHED = ../../../build/unix

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

# Installation lives in the top-level ToolShed makefile (see note above).
install dist package uninstall:
	@echo "c3 is installed by the top-level ToolShed makefile, not here."
	@echo "Run:  make -C $(TOOLSHED) install      (installs c3 + the rest of ToolShed)"
	@echo "  or:  make -C $(TOOLSHED) package      (build a distribution tarball)"
	@echo "(honours PREFIX/DESTDIR; headers go to PREFIX/share/toolshed/defs.)"
	@false
