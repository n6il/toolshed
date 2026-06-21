include ../rules.mak

vpath %.c ../../../prep
vpath %.h ../../../prep
INCS	= -v=../../../prep -v=../../../include

BINARY	= c3prep
OBJS	= scan.r dues.r eval.r lex.r macros.r main.r misc.r parse.r

$(BINARY):	$(OBJS)
	$(OS9LINK)

clean:
	rm -f $(BINARY) *.r

install: $(BINARY)
	$(os9install)
