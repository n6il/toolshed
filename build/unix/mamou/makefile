# Makefile for mamou

vpath %.c ../../../mamou

CFLAGS	+= -DLINUX -I../../../include -I../../../dal/include -I../../../kal/include
LDFLAGS	+= -L../libbp_kal -L../libcoco -L../libnative -L../libdecb -L../librbf -L../libmisc -L../libsys -lcoco -lnative -ldecb -lrbf -lmisc -lsys -lbp_kal

mamou:		mamou.o evaluator.o pseudo.o h6309.o ffwd.o \
		print.o util.o symbol_bucket.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o mamou
