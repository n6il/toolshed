# Makefile for libcoco.a
include ../rules.mak

vpath %.c ../../../libcoco

CFLAGS	+= -g -I../../../include -Wall

%.a:
	$(AR) -r $@ $^
	$(RANLIB) $@

libcoco.a:	libcocogs.o libcocodelete.o libcocoopen.o libcocoread.o libcocorename.o libcocoseek.o libcocoss.o libcocoread.o libcocowrite.o \
	libcocoreadln.o libcocomakdir.o

clean:
	$(RM) *.o *.a
