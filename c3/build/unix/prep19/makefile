include ../rules.mak

vpath %.c ../../../prep19
vpath %.h ../../../prep19

BINARY	= c.prep
OBJS	= cp.o cp1.o cp2.o cp3.o cp4.o findstr.o solve.o max.o findchar.o \
	strcmp2.o lnread.o itoa.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm -f $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
