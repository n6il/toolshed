# Makefile for aslink
include ../rules.mak

vpath %.c ../../../aslink

CFLAGS	+=
LDFLAGS	+=

aslink:	lkarea.o lkdata.o lkhead.o lklex.o lkmain.o lkos9.o lks19.o \
	lkbin.o lkeval.o lkihx.o lklist.o lkmod.o lkrloc.o  lksym.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o aslink
