# Makefile for os9/decb suite

vpath %.c ../../os9asm:../../os9

PACKAGENAME	= os9tools.tgz
INSTALLDIR	= /usr/local/bin
DIRS		= libcoco libnative librbf libmisc libsys libdecb os9 os9asm decb

all:
	$(foreach dir, $(DIRS), (cd $(dir); make);)

install: all
	cp os9/os9 os9asm/os9asm decb/decb $(INSTALLDIR)

package: all
	rm -rf os9tools
	mkdir os9tools
	mkdir os9tools/bin
	cp os9/os9 os9asm/os9asm os9tools/bin
	cp ../../README os9tools
	tar czvf $(PACKAGENAME) os9tools
	rm -rf os9tools

clean:
	$(foreach dir, $(DIRS), (cd $(dir); make clean);)
	rm os9tools.tgz

