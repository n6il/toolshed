include ../rules.mak

vpath %.c ../../../lsplit
vpath %.h ../../../lsplit

BINARY	= lsplit
OBJS	= lsplit.o

$(BINARY):	$(OBJS)
	cc $(OBJS) -o $@

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
