include ../rules.mak

vpath %.c ../../../lsplit
vpath %.h ../../../lsplit
INCS	= -v=../../../lsplit -v=../../../include

BINARY	= lsplit
OBJS	= lsplit.r

$(BINARY):	$(OBJS)
	$(OS9LINK)

clean:
	rm -f $(BINARY) *.r

install: $(BINARY)
	$(os9install)
