include ../rules.mak

vpath %.c ../../../makewav
vpath %.h ../../../makewav

# CFLAGS += -DDEBUG

BINARY	= makewav
OBJS	= makewav.o
LIBS	= -lm

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $@

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
