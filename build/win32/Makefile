# Makefile for ToolShed

vpath %.c ../../mamou:../../os9


RELEASE		= _1.0
PACKAGENAME	= toolshed$(RELEASE).zip
INSTALLDIR	= $(HOME)/bin
DIRS		= libnative librbf libcoco libdecb libmisc libsys libdecb \
		libtoolshed os9 mamou decb ar2 rma rlink makewav tocgen
# rma rlink rdump

all:
	$(foreach dir, $(DIRS), (cd $(dir); make);)

install: all
	cp os9/os9.exe mamou/mamou.exe decb/decb.exe ar2/ar2.exe \
	rma/rma.exe rlink/rlink.exe makewav/makewav.exe tocgen/tocgen.exe \
	$(INSTALLDIR)
#	as6809/as6809 aslink/aslink $(INSTALLDIR)

package: all
	rm -f $(PACKAGENAME)
	zip -9j $(PACKAGENAME) os9/os9.exe decb/decb.exe mamou/mamou.exe \
	rma/rma.exe ar2/ar2.exe rlink/rlink.exe makewav/makewav.exe \
	tocgen/tocgen.exe readme.rtf

clean:
	$(foreach dir, $(DIRS), (cd $(dir); make clean);)
	rm $(PACKAGENAME)
