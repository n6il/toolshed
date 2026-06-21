# Cross-build the c3 tools to NitrOS-9 (OS-9 / MC6809) with an installed c3
# toolchain.  Sibling of build/unix, but the outputs are OS-9 program modules
# rather than host executables.  See rules.mak for the toolchain assumptions
# and the OS-9-idiom install (CMDS staging / disk image; never the host bin).
#
# The set below is the K&R, self-host-relevant subset.  Excluded, as in
# build/unix: "opt" (incomplete - missing inst.c) and "compx" (the host-only
# DEBUG variant of the compiler).  "optx" is the working optimiser.
#
# NOTE: all six tools now build and link to valid OS-9 modules.  What remains is
# runtime correctness of the generated code at this scale (FP especially, via the
# MC6839 shim) - i.e. getting the modules to actually RUN under OS-9.
# The disk uses the two-pass compiler (comp1+comp2); single-pass c3comp is too
# big to F$Fork on an OS-9 Level-2 task, so it is not built or shipped here.
# make is cross-built like the c3 tools (it drives on-target builds, e.g. the
# library build); the r-utils rma/rlink come prebuilt from CoCoC (see mkdsk.sh).
dirs	= exec prep comp1 comp2 optx lorder lsplit make

# dsk target knobs (override on the command line):
#   DSK     output image (in this directory)
#   DSKNAME OS-9 volume name
#   LSECTORS flat OS-9 logical sectors (256B each; default 16384 = 4MB IDE image)
#   COCOC   CoCo C checkout that supplies the r-utils (rma rlink mold rdump);
#           required, e.g. a clone of https://github.com/DrPitre/CoCoC.git
DSK	?= c3tools.dsk
DSKNAME	?= C3TOOLS
LSECTORS ?= 16384
COCOC	?=

.PHONY: all clean install dsk

all:
	@echo "**************************************************"
	@echo "*                                                *"
	@echo "*        MW C Compiler  ->  NitrOS-9 (OS-9)       *"
	@echo "*                                                *"
	@echo "**************************************************"
	$(foreach dir, $(dirs), (cd $(dir); $(MAKE));)

clean:
	$(foreach dir, $(dirs), (cd $(dir); $(MAKE) clean);)
	rm -rf dist
	rm -f "$(DSK)"

install:
	$(foreach dir, $(dirs), (cd $(dir); $(MAKE) install);)

# ---- dsk: one command -> a complete c3-on-NitrOS-9 distribution disk ---------
# Builds the tools, then mkdsk.sh lays out an OS-9 image (mounted as the driver's
# default base /dd):  CMDS/ (c3 tools + the Microware r-utils), defs/ (c3 v2-ABI
# headers), src/ (compiler + library sources + the mklibs build procedure) and an
# empty lib/ that mklibs populates on first boot.  See mkdsk.sh for the knobs.
dsk: all
	DSK="$(CURDIR)/$(DSK)" DSKNAME="$(DSKNAME)" LSECTORS="$(LSECTORS)" COCOC="$(COCOC)" \
	    sh mkdsk.sh
