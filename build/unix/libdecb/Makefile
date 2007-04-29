# Makefile for libdecb.a
include ../rules.mak

vpath %.c ../../../libdecb

CFLAGS	+= -g -I../../../include -Wall

%.a:
	ar -r $@ $^
	ranlib $@

libdecb.a:	libdecbgs.o libdecbkill.o libdecbopen.o libdecbread.o libdecbrename.o libdecbseek.o libdecbss.o libdecbread.o libdecbwrite.o libdecbtokenize.o

clean:
	rm *.o *.a
