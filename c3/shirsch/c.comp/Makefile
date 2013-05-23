CFLAGS = -m32 -g -Wall
LFLAGS = -m32 -g -Wall -o co_comp

RELS = comp_01.o comp_02.o comp_03.o comp_04.o comp_05.o comp_06.o\
	   comp_07.o comp_08.o comp_09.o comp_10.o comp_11.o comp_12.o\
	   comp_13.o comp_14.o comp_15.o cocodbl.o

protos = comp_01.c comp_02.c comp_03.c comp_04.c comp_05.c\
		 comp_06.c comp_07.c comp_08.c comp_09.c comp_10.c\
		 comp_11.c comp_12.c comp_13.c comp_14.c comp_15.c

co_comp: $(RELS)
	$(CC) $(RELS) $(LFLAGS)

comp_01.o: ccomp.h

$(RELS): defines.h

proto.h: $(protos)
	echo >proto.h
	cproto $(protos) -o proto.z
	mv proto.z proto.h

clean:
	rm -f *.o *.s co_comp

install: co_comp
	install co_comp /usr/local/bin
