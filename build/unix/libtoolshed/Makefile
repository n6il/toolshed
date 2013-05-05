# Makefile for libtoolshed.a
include ../rules.mak

vpath %.c ../../../libtoolshed

CFLAGS	+= -I../../../include -Wall

%.a:
	$(AR) -r lib$@ $^
	$(RANLIB) lib$@

toolshed.a:	toolshed.o

clean:
	$(RM) *.o *.a
