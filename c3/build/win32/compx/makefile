include ../rules.mak

vpath %.c ../../../compexp
vpath %.h ../../../compexp

BINARY	= c3compx
OBJS	= bool.o build.o cmain.o codgen.o declare.o floats.o get.o inits.o lex.o local.o \
	longs.o misc.o optim.o printf.o regscont.o stats.o tb.o tranexp.o

# Enable to print debugging info during compile
#CFLAGS += -DDEBUG
#OBJS += prtree.o

$(BINARY):	$(OBJS)
	cc $(OBJS) -o $@

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
