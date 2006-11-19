# Makefile for ToolShed

vpath %.c ../../mamou:../../os9

PACKAGENAME	= toolshed$(RELEASE).tgz
INSTALLDIR	= $(HOME)/bin
DIRS		= libtoolshed libnative librbf libcoco libdecb libmisc libsys libdecb \
		ar2 os9 mamou decb rma rlink rdump tocgen

all:
	$(foreach dir, $(DIRS), (cd $(dir); make);)

install: all
	cp ar2/ar2 os9/os9 mamou/mamou decb/decb rma/rma rlink/rlink rdump/rdump tocgen/tocgen \
	$(INSTALLDIR)
#	as6809/as6809 aslink/aslink $(INSTALLDIR)

package: all
	rm -rf toolshed$(RELEASE)
	mkdir toolshed$(RELEASE)
	mkdir toolshed$(RELEASE)/bin
	mkdir toolshed$(RELEASE)/lib
	cp os9/os9 decb/decb mamou/mamou rma/rma rlink/rlink rdump/rdump tocgen/tocgen toolshed$(RELEASE)/bin
	cp ../../alib/alib.l toolshed$(RELEASE)/lib
	tar czvf $(PACKAGENAME) toolshed$(RELEASE)
	rm -rf toolshed$(RELEASE)

clean:
	$(foreach dir, $(DIRS), (cd $(dir); make clean);)
	rm $(PACKAGENAME)
