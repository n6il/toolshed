# Makefile for CoCoTools

vpath %.c ../../mamou:../../os9

PACKAGENAME	= cocotools.tgz
INSTALLDIR	= /usr/local/bin
DIRS		= libnative librbf libcoco libdecb libmisc libsys libdecb os9 mamou decb
#		rma rdump as6809 aslink

all:
	$(foreach dir, $(DIRS), (cd $(dir); make);)

install: all
	cp os9/os9 mamou/mamou decb/decb rma/rma rdump/rdump \
	as6809/as6809 aslink/aslink $(INSTALLDIR)

package: all
	rm -rf cocotools
	mkdir cocotools
	cp os9/os9 mamou/mamou cocotools
	cp os9/os9 mamou/mamou cocotools
	cp decb/decb cocotools
	tar czvf $(PACKAGENAME) cocotools
	rm -rf cocotools

clean:
	$(foreach dir, $(DIRS), (cd $(dir); make clean);)
	rm cocotools.tgz

