include ../rules.mak

vpath %.c ../../../exec
vpath %.h ../../../exec
INCS	= -v=../../../exec -v=../../../include

BINARY	= c3
OBJS	= c3.r

$(BINARY):	$(OBJS)
	$(OS9LINK)

clean:
	rm -f $(BINARY) *.r

install: $(BINARY)
	$(os9install)
