# Makefile for libcecb.a
include ../rules.mak

vpath %.c ../../../libcecb

CFLAGS	+= -g -I../../../include -Wall

%.a:
	ar -r $@ $^
	ranlib $@

libcecb.a:	libcebcopen.o libcecbgs.o libcecbwav.o \
                libcecbcas.o libcecbread.o libcecbwrite.o

clean:
	rm *.o *.a
