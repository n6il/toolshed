# Makefile for libdecb.a

vpath %.c ../../../libdecb

CFLAGS	+= -g -I../../../include -Wall

%.a:
	ar -r $@ $^
	ranlib $@

libdecb.a:	decbgs.o decbkill.o decbopen.o decbread.o decbrename.o decbseek.o decbss.o decbread.o decbwrite.o decbtokenize.o

clean:
	rm *.o *.a
