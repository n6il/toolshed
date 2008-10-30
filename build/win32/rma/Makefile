# Makefile for rma

vpath %.c ../../../rma
vpath %.h ../../../rma

rma:    chcodes.o part2.o part3.o part4.o part5.o part6.o \
part7.o part8.o part9.o rma_main.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f rma.exe *.o
