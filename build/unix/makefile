# Makefile for CoCoTools

vpath %.c ../../os9asm:../../os9

PACKAGENAME	= cocotools.tgz
INSTALLDIR	= /usr/local/bin
DIRS		= libnative librbf libmisc libsys libdecb os9 os9asm decb \
		rma rdump as6809 aslink

all:
	$(foreach dir, $(DIRS), (cd $(dir); make);)

install: all
	cp os9/os9 os9asm/os9asm decb/decb rma/rma rdump/rdump \
	as6809/as6809 aslink/aslink $(INSTALLDIR)

package: all
	rm -rf cocotools
	mkdir cocotools
	cp os9/os9 os9asm/os9asm cocotools
	cp os9/os9 os9asm/os9asm cocotools
	cp decb/decb cocotools
	tar czvf $(PACKAGENAME) cocotools
	rm -rf cocotools

clean:
	$(foreach dir, $(DIRS), (cd $(dir); make clean);)
	rm cocotools.tgz

