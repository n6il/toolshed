# Makefile for libnative.a
include ../rules.mak

vpath %.c ../../../libnative

CFLAGS	+= -g -I../../../include -Wall

%.a:
	$(AR) -r $@ $^
	$(RANLIB) $@

libnative.a:	libnativegs.o libnativedelete.o libnativeopen.o libnativeread.o libnativerename.o libnativeseek.o libnativess.o libnativeread.o libnativewrite.o libnativereadln.o libnativemakdir.o
clean:
	$(RM) *.o *.a
