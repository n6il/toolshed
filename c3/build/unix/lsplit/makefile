include ../rules.mak

vpath %.c ../../../lsplit
vpath %.h ../../../lsplit

BINARY	= lsplit
OBJS	= lsplit.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm -f $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
