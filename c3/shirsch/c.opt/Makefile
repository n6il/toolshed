CFLAGS = -m32 -g -Wall
#CFLAGS = -g

RELS = opt01.o opt02.o opt03.o opt04.o opt05.o opt06.o

protos = opt01.c opt02.c opt03.c opt04.c opt05.c opt06.c

co_opt: $(RELS)
	$(CC) $(CFLAGS) $(RELS) -o $@

proto.h: $(protos)
	echo > proto.h
	cproto $(protos) -o proto.z
	mv proto.z proto.h

clean:
	rm -f *.o *.s proto.h co_opt

