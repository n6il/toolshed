# Makefile for ToolShed

vpath %.c ../../mamou:../../os9


RELEASE		= _2.0
PACKAGENAME	= toolshed$(RELEASE)_win32.zip
INSTALLDIR	= $(HOME)/bin
DIRS		= libnative librbf libcoco libdecb libmisc libcecb libsys  \
		libtoolshed os9 mamou decb cecb ar2 rma rlink makewav tocgen
# rma rlink rdump

all:
	$(foreach dir, $(DIRS), (echo $(dir); cd $(dir); make);)

install: all
	cp os9/os9.exe mamou/mamou.exe decb/decb.exe cecb/cecb.exe ar2/ar2.exe \
	rma/rma.exe rlink/rlink.exe makewav/makewav.exe tocgen/tocgen.exe \
	$(INSTALLDIR)
#	as6809/as6809 aslink/aslink $(INSTALLDIR)

package: all
	rm -f $(PACKAGENAME)
	zip -9j $(PACKAGENAME) os9/os9.exe decb/decb.exe cecb/cecb.exe mamou/mamou.exe \
	rma/rma.exe ar2/ar2.exe rlink/rlink.exe makewav/makewav.exe \
	tocgen/tocgen.exe readme.rtf

clean:
	$(foreach dir, $(DIRS), (cd $(dir); make clean);)
	rm -f $(PACKAGENAME)
