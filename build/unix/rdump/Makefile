# Makefile for rdump

vpath %.c ../../../rdump

CFLAGS	+=
LDFLAGS	+=

rdump:	rdump.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o rdump
