# Makefile for libsys.a
include ../rules.mak

vpath %.c ../../../libsys

CFLAGS	+= -I../../../include -Wall

%.a:
	$(AR) -r $@ $^
	$(RANLIB) $@

libsys.a:	crc.o prsnam.o

clean:
	$(RM) *.o *.a
