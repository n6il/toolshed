include ../rules.mak

vpath %.c ../../../tocgen
vpath %.h ../../../tocgen

CFLAGS  += -I../../../include -Wall -g
LDFLAGS += -L../libcoco -L../libnative -L../libdecb -L../libmisc -L../librbf -L../libsys -lcoco -lnative -ldecb -lmisc -lrbf -lsys -lm
BINARY	= tocgen
OBJS	= tocgen.o


$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(DEBUG)

$(OBJS):	Makefile

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
