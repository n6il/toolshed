include ../rules.mak

vpath %.c ../../../comp
vpath %.h ../../../comp

BINARY	= c3comp
OBJS	= bool.o build.o cmain.o codgen.o declare.o floats.o get.o inits.o lex.o local.o \
	longs.o misc.o optim.o printf.o regscont.o stats.o tb.o tranexp.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm -f $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
