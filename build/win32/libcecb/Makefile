# Makefile for libcecb.a

vpath %.c ../../../libcecb

CFLAGS	+= -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

libcecb.a:	libcecbwrite.o libcecbwav.o libcecbread.o libcecbgs.o libcecbcas.o libcebcopen.o

clean:
	rm *.o *.a
