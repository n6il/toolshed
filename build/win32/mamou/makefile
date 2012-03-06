# Makefile for mamou

vpath %.c ../../../mamou

CFLAGS	+= -I../../../include
LDFLAGS	+= -L../libtoolshed -L../libcoco -L../libnative -L../libmisc -L../librbf \
-L../libdecb -L../libcecb -L../libsys -ltoolshed -lcoco -lnative -lmisc -lrbf \
-ldecb -lcecb -lsys

mamou:	evaluator.o ffwd.o h6309.o mamou_main.o pseudo.o print.o symbol_bucket.o \
	util.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o mamou.exe
