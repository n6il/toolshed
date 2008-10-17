# Makefile for tocgen

vpath %.c ../../../tocgen
vpath %.h ../../../tocgen

CFLAGS += -I../../../include
LDFLAGS	+= -L../libtoolshed -L../libcoco -L../libnative -L../librbf -L../libdecb -L../libmisc -L../libsys -ltoolshed -lcoco -lnative -lrbf -ldecb -lmisc -lsys -lm 

tocgen:	tocgen_main.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o tocgen.exe
