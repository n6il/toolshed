# Makefile for ToolShed
include rules.mak

vpath %.c ../../mamou:../../os9

PACKAGENAME	= toolshed$(RELEASE).tgz
INSTALLDIR	= $(HOME)/bin
DIRS		= libtoolshed libnative libcecb librbf libcoco libdecb libmisc libsys \
		ar2 os9 mamou decb tocgen cocofuse makewav libcecb cecb

all:
	$(foreach dir, $(DIRS), (cd $(dir); $(MAKE));)

install: all
	cp ar2/ar2 os9/os9 mamou/mamou cecb/cecb decb/decb tocgen/tocgen \
           cocofuse/cocofuse makewav/makewav $(INSTALLDIR)

package: all
	rm -rf toolshed$(RELEASE)
	mkdir toolshed$(RELEASE)
	mkdir toolshed$(RELEASE)/bin
	mkdir toolshed$(RELEASE)/lib
	cp os9/os9 cecb/cecb decb/decb mamou/mamou tocgen/tocgen makewav/makewav toolshed$(RELEASE)/bin
	cp ../../alib/alib.l toolshed$(RELEASE)/lib
	tar czvf $(PACKAGENAME) toolshed$(RELEASE)
	rm -rf toolshed$(RELEASE)

clean:
	$(foreach dir, $(DIRS), (cd $(dir); $(MAKE) clean);)
	rm -f $(PACKAGENAME)
