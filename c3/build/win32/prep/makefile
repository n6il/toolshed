include ../rules.mak

vpath %.c ../../../prep
vpath %.h ../../../prep

BINARY	= c3prep
OBJS	= scan.o dues.o eval.o history.o lex.o macros.o main.o misc.o parse.o

$(BINARY):	$(OBJS)
	cc $(OBJS) -o $@

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
