include ../rules.mak

vpath %.c ../../../prep19
vpath %.h ../../../prep19

BINARY	= c.prep
OBJS	= cp.o cp1.o cp2.o cp3.o cp4.o findstr.o

$(BINARY):	$(OBJS)
	cc $(OBJS) -o $@

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
