# Makefile for rsdos

vpath %.c ../../../rsdos

CFLAGS	+= -I../../../include
LDFLAGS	+= -L../librsdos -L../libmisc -lrsdos -lmisc -lm

rsdos:	rsdos.o rsdosdir.o rsdosfree.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o rsdos
