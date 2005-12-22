# Makefile for ToolShed

vpath %.c ../../mamou:../../os9


RELEASE		= 0.9
PACKAGENAME	= toolshed$(RELEASE).tgz
INSTALLDIR	= $(HOME)/bin
DIRS		= libnative librbf libcoco libdecb libmisc libsys libdecb \
		os9 mamou decb
# rma rlink rdump

all:
	$(foreach dir, $(DIRS), (cd $(dir); make);)

install: all
	cp os9/os9 mamou/mamou decb/decb $(INSTALLDIR)
#	as6809/as6809 aslink/aslink $(INSTALLDIR)

package: all
	rm -rf toolshed$(RELEASE)
	mkdir toolshed$(RELEASE)
	mkdir toolshed$(RELEASE)/bin
	mkdir toolshed$(RELEASE)/lib
	cp os9/os9 decb/decb mamou/mamou toolshed$(RELEASE)/bin
	cp ../../alib/alib.l toolshed$(RELEASE)/lib
	tar czvf $(PACKAGENAME) toolshed$(RELEASE)
	rm -rf toolshed$(RELEASE)

clean:
	$(foreach dir, $(DIRS), (cd $(dir); make clean);)
	rm $(PACKAGENAME)
