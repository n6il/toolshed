# Makefile for mamou

vpath %.c ../../../mamou

DEBUG	= -g
CFLAGS	+= -DLINUX -I../../../include $(DEBUG) -Wall
LDFLAGS	+= -L../libcoco -L../libnative -L../libdecb -L../librbf -L../libmisc -L../libsys -lcoco -lnative -ldecb -lrbf -lmisc -lsys $(DEBUG)

mamou:		mamou.o evaluator.o pseudo.o h6309.o ffwd.o \
		print.o util.o symbol_bucket.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o mamou
