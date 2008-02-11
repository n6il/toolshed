include ../rules.mak

vpath %.c ../../../rma
vpath %.h ../../../rma

BINARY	= rma
OBJS	= chcodes.o part2.o part3.o part4.o part5.o part6.o part7.o part8.o part9.o rma_main.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
