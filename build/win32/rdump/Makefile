# Makefile for rdump

vpath %.c ../../../rdump
vpath %.h ../../../rdump

CFLAGS += -I../../../include

rdump:  rdump_main.o 6309dasm.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o rdump.exe
