include ../rules.mak

vpath %.c ../../../compexp
vpath %.h ../../../compexp

BINARY	= c3compx
NOBJS   = declare.o printf.o
OBJS	= bool.o build.o cmain.o codgen.o declare.o floats.o get.o inits.o lex.o local.o \
	longs.o misc.o optim.o regscont.o stats.o tb.o tranexp.o fadjust.o

# Enable to print debugging info during compile
CFLAGS += -DDEBUG -std=c89 -g
OBJS += prtree.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
