# Makefile for libcoco.a

vpath %.c ../../../libcoco

CFLAGS	+= -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

libcoco.a:	libcocodelete.o libcocogs.o libcocomakdir.o libcocoopen.o libcocoread.o \
libcocoreadln.o libcocorename.o libcocoseek.o libcocoss.o libcocowrite.o

clean:
	rm *.o *.a
