include ../rules.mak

vpath %.c ../../../make
vpath %.h ../../../make

BINARY	= make
OBJS	= check.o files.o input.o macro.o main.o make.o reader.o rules.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm -f $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
