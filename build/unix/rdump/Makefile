include ../rules.mak

vpath %.c ../../../rdump
vpath %.h ../../../rdump

BINARY	= rdump
OBJS	= rdump_main.o 6309dasm.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@ $(DEBUG)

$(OBJS):	hd6309.h ../include/rof.h Makefile

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
