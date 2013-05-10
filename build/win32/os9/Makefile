# Makefile for os9

vpath %.c ../../../os9

CFLAGS  += -I../../../include

LDFLAGS += -L../libtoolshed -L../libcoco -L../libnative -L../libmisc -L../librbf \
-L../libdecb -L../libcecb -L../libsys -ltoolshed -lcoco -lnative -lmisc \
-lrbf -ldecb -lcecb -lsys

os9:    os9copy.o os9dsave.o os9gen.o os9modbust.o os9dcheck.o os9dump.o \
    os9id.o os9padrom.o os9_main.o os9del.o os9format.o os9ident.o \
    os9rename.o os9attr.o os9deldir.o os9free.o os9list.o os9cmp.o \
    os9dir.o os9fstat.o os9makdir.o os9rdump.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o os9.exe
