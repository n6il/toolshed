# Makefile for ToolShed

vpath %.c ../../mamou:../../os9


RELEASE		= _2.0
PACKAGENAME	= toolshed$(RELEASE)_win32.zip
INSTALLDIR	= $(HOME)/bin
DIRS		= libnative librbf libcoco libdecb libmisc libcecb libsys  \
		libtoolshed os9 mamou decb cecb rma rlink makewav tocgen
# ar2

all:
	$(foreach dir, $(DIRS), (echo $(dir); cd $(dir); make);)

install: all
	cp os9/os9.exe mamou/mamou.exe decb/decb.exe cecb/cecb.exe \
	rma/rma.exe rlink/rlink.exe makewav/makewav.exe tocgen/tocgen.exe \
	$(INSTALLDIR)
#	ar2/ar2.exe

package: all
	rm -f $(PACKAGENAME)
	zip -9j $(PACKAGENAME) os9/os9.exe decb/decb.exe cecb/cecb.exe mamou/mamou.exe \
	rma/rma.exe rlink/rlink.exe makewav/makewav.exe \
	tocgen/tocgen.exe readme.rtf
# ar2/ar2.exe

clean:
	$(foreach dir, $(DIRS), (cd $(dir); make clean);)
	rm -f $(PACKAGENAME)
