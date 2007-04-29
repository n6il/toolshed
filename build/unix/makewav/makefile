include ../rules.mak

vpath %.c ../../../makewav
vpath %.h ../../../makewav

# CFLAGS += -DDEBUG

BINARY	= makewav
OBJS	= makewav.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
