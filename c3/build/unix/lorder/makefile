include ../rules.mak

vpath %.c ../../../lorder
vpath %.h ../../../lorder

BINARY	= lorder
OBJS	= lorder.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm -f $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
