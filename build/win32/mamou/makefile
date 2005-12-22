# Makefile for mamou

vpath %.c ../../../mamou

CFLAGS	+= -I../../../include
LDFLAGS	+= -L ../libcoco -L ../libnative -L../libmisc -L../librbf -L ../libdecb -L../libsys -lcoco -lnative -lmisc -lrbf -ldecb -lsys -lm

mamou:	evaluator.o ffwd.o h6309.o mamou.o pseudo.o print.o symbol_bucket.o \
	util.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o mamou.exe
