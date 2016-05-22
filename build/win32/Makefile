# Makefile for ToolShed

$(warning ********************************************************)
$(warning This makefile is not maintained, use build/unix/Makefile)
$(warning also for Windows builds)
$(warning ********************************************************)

vpath %.c ../../mamou:../../os9


RELEASE		= 2.2
PACKAGENAME	= toolshed_$(RELEASE)_win32.zip
INSTALLDIR	= /usr/local/bin
DIRS		= libnative librbf libcoco libdecb libmisc libcecb libsys  \
		libtoolshed os9 mamou decb cecb makewav tocgen
# ar2

export CC=gcc
export CFLAGS=-DWIN32 -mno-cygwin

all:
	$(foreach dir, $(DIRS), (echo $(dir); cd $(dir); make);)

install: all
	mkdir -p $(INSTALLDIR)
	cp os9/os9.exe mamou/mamou.exe decb/decb.exe cecb/cecb.exe \
	makewav/makewav.exe tocgen/tocgen.exe \
	$(INSTALLDIR)
#	ar2/ar2.exe

package: all
	rm -f $(PACKAGENAME)
	zip -9j $(PACKAGENAME) os9/os9.exe decb/decb.exe cecb/cecb.exe mamou/mamou.exe \
	makewav/makewav.exe \
	tocgen/tocgen.exe readme.rtf
# ar2/ar2.exe

clean:
	$(foreach dir, $(DIRS), (cd $(dir); make clean);)
	rm -f $(PACKAGENAME)
