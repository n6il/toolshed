# Makefile for rma

vpath %.c ../../../rma

CFLAGS	+=
LDFLAGS	+=

rma:	rma.o part2.o part3.o part4.o part5.o part6.o part7.o \
	part8.o  part9.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o rma
