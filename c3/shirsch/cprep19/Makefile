RELS = cp.o cp1.o cp2.o cp3.o cp4.o solve.o

CFLAGS = -m32 -g -I. -DXTARGET

coprep: $(RELS)
	gcc -m32 $(RELS) -o $@

$(RELS): cp.h

clean:
	rm -f $(RELS) coprep

install: coprep
	install coprep /usr/local/bin
