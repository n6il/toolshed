# Makefile for librbf.a

vpath %.c ../../../librbf

CFLAGS	+= -I../../../include -Wall -g

%.a:
	ar -r $@ $^
	ranlib $@

librbf.a:	rbfbitmap.o rbfmakdir.o rbfread.o rbfrename.o rbfss.o rbfdelete.o \
		rbfgs.o rbfopen.o rbfreadln.o rbfseek.o rbfwrite.o

clean:
	rm *.o *.a
