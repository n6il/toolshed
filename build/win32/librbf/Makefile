# Makefile for librbf.a

vpath %.c ../../../librbf

CFLAGS	+= -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

librbf.a:	librbfbitmap.o librbfmakdir.o librbfread.o librbfrename.o \
librbfss.o librbfdelete.o librbfgs.o librbfopen.o librbfreadln.o \
librbfseek.o librbfwrite.o

clean:
	rm *.o *.a
