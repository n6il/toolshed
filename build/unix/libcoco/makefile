# Makefile for libcoco.a

vpath %.c ../../../libcoco

CFLAGS	+= -g -I../../../include -Wall

%.a:
	ar -r $@ $^
	ranlib $@

libcoco.a:	cocogs.o cocodelete.o cocoopen.o cocoread.o cocorename.o cocoseek.o cocoss.o cocoread.o cocowrite.o \
	cocoreadln.o

clean:
	rm *.o *.a
