include ../rules.mak

vpath %.c ../../../lorder
vpath %.h ../../../lorder
INCS	= -v=../../../lorder -v=../../../include

BINARY	= lorder
OBJS	= lorder.r

$(BINARY):	$(OBJS)
	$(OS9LINK)

clean:
	rm -f $(BINARY) *.r

install: $(BINARY)
	$(os9install)
