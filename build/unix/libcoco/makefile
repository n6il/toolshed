# Makefile for libcoco.a

vpath %.c ../../../libcoco

CFLAGS	+= -g -I../../../include -Wall

%.a:
	ar -r $@ $^
	ranlib $@

libcoco.a:	libcocogs.o libcocodelete.o libcocoopen.o libcocoread.o libcocorename.o libcocoseek.o libcocoss.o libcocoread.o libcocowrite.o \
	libcocoreadln.o

clean:
	rm *.o *.a
