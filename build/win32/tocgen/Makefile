# Makefile for tocgen

vpath %.c ../../../tocgen
vpath %.h ../../../tocgen

CFLAGS += -I../../../include
LDFLAGS += -L../libtoolshed -L../libcoco -L../libnative -L../librbf -L../libdecb \
-L../libcecb -L../libmisc -L../libsys -ltoolshed -lcoco -lnative -lrbf -ldecb -lcecb \
-lmisc -lsys -lm 

tocgen: tocgen_main.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o tocgen.exe
