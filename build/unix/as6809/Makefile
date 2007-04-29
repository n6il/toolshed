# Makefile for as6809
include ../rules.mak

vpath %.c ../../../attic/as6809

CFLAGS	+=
LDFLAGS	+=

as6809:	asdata.o aslex.o asmain.o asout.o assym.o m09adr.o m09mch.o \
	asexpr.o  aslist.o assubr.o m09ext.o m09pst.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o as6809
