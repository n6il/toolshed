include ../rules.mak

vpath %.c ../../../ar2
vpath %.h ../../../ar2

BINARY	= ar2
OBJS	= ar.o arsup.o lz1.o o2u.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@ $(DEBUG) -DSYSV

$(OBJS):

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
