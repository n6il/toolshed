# C-Cubed: cross-build to NitrOS-9 (OS-9 / MC6809), project-wide rules.
#
# Mirrors build/unix, but instead of the host `cc` it drives an INSTALLED c3
# cross-toolchain to turn the c3 sources into 6809 relocatable objects (.r) and
# link them into OS-9 program modules.  Nothing here touches the build host's
# native compiler or its $HOME/bin etc.
#
# The compile step is the same pipeline the c3 driver's `c3-as` shim runs
# (c3prep | c3comp | c3optx | c3lw.awk | lwasm); the link step is what `c3-ld`
# runs (lwlink --format=os9 with crt0.o + libc3lw.a).  We invoke the stages
# directly rather than the `c3` driver because the driver writes its .r next to
# the source file, whereas a build/unix-style out-of-tree build keeps objects
# in the build directory.
#
# Assumes an installed toolchain (override these on the make command line to
# relocate, e.g. `make C3BASE=$PREFIX/share/toolshed C3LIBDIR=$PREFIX/bin`):
#   - on PATH:    c3prep  c3comp  c3optx  lwasm  lwlink  awk
#   - C3BASE/defs : the target's system headers (<stdio.h>, <string.h>, ...)
#   - C3LIBDIR    : the lwbackend runtime — c3lw.awk, os9c3.ld, crt0.o, libc3lw.a
#
# The defaults match a stock `make -C build/unix install` (PREFIX=/usr/local):
# headers under $PREFIX/share/toolshed/defs, the runtime beside the binaries in
# $PREFIX/bin.

PREFIX		?= /usr/local
C3BASE		?= $(PREFIX)/share/toolshed
C3LIBDIR	?= $(PREFIX)/bin

PREP		= c3prep
COMP		= c3comp
OPTX		= c3optx
LWASM		= lwasm
LWLINK		= lwlink
FILTER		= $(C3LIBDIR)/c3lw.awk
LDSCRIPT	= $(C3LIBDIR)/os9c3.ld
CRT0		= $(C3LIBDIR)/crt0.o

# Preprocessor flags: cross-target define + the target system headers.  Each
# component adds its own source include directory(ies) via INCS.
CPPFLAGS	= -DOS9 -v=$(C3BASE)/defs $(INCS)

# Catch a failure in any pipeline stage, not just the last.
SHELL		= /bin/bash
.SHELLFLAGS	= -o pipefail -c

# .c -> .r  (a 6809 relocatable object, == what the c3 driver's c3-as produces)
%.r : %.c
	$(PREP) $(CPPFLAGS) $< | $(COMP) | $(OPTX) | awk -f $(FILTER) \
	    | $(LWASM) --format=obj -o $@ -

# Link an OS-9 program module from $(OBJS)  (== what c3-ld produces).  The
# module name embedded in the file is the output basename, so link to a bare
# name (e.g. c3comp), not a path.
OS9LINK		= $(LWLINK) --format=os9 -s $(LDSCRIPT) -o $@ \
		      $(CRT0) $(OBJS) -L$(C3LIBDIR) -lc3 -lc3lw

# ---- install: OS-9 idioms only; never touch the host's native dirs ----------
# OS-9 keeps command modules in a CMDS directory.  Two modes:
#   * OS9DISK set  -> install straight onto that disk image's CMDS with the
#                     toolshed `os9` tool, setting the execute attribute (the
#                     idiomatic, ready-to-run form).
#   * otherwise    -> stage the module file into $(CMDSDIR) (a shared host
#                     directory) for later `os9 copy ... ,CMDS/<name>` +
#                     `os9 attr -e -pe`.
# Either way nothing is written to $HOME/bin, /usr, etc.
OS9DISK		?=
CMDSDIR		?= ../dist/CMDS

define os9install
	@if [ -n "$(OS9DISK)" ]; then \
	    os9 copy -r $(BINARY) "$(OS9DISK),CMDS/$(BINARY)" && \
	    os9 attr -e -pe "$(OS9DISK),CMDS/$(BINARY)" && \
	    echo "installed $(BINARY) -> $(OS9DISK),CMDS/$(BINARY) (e,pe set)"; \
	else \
	    mkdir -p "$(CMDSDIR)" && cp $(BINARY) "$(CMDSDIR)/$(BINARY)" && \
	    echo "staged $(BINARY) -> $(CMDSDIR)/  (then: os9 copy + os9 attr -e -pe onto a disk)"; \
	fi
endef
