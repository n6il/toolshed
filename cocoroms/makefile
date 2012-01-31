HDBSCSI	= hdbtc3.rom hdbkenton.rom hdblrtech.rom hdbd4n1.rom hdbdhdii.rom
HDBIDE  = hdblba.rom hdbchs.rom
HDBDW   = hdbdw3cc2.rom hdbdw3cc3.rom hdbdw4cc3.rom
HDB     = $(HDBSCSI) $(HDBIDE) $(HDBDW)

all: bas13.rom extbas11.rom disk11.rom coco3.rom \
	$(HDB)

bas13.rom: equates.asm bas.asm
	mamou -q -mr equates.asm bas.asm -o$@

cb_equates.asm: equates.asm bas.asm
	$(shell mamou -q -mr equates.asm bas.asm -sa > cb_equates.asm)

extbas11.rom: bas.asm cb_equates.asm extbas.asm equates.asm
	mamou -q -mr cb_equates.asm extbas.asm -o$@

ecb_equates.asm: equates.asm bas.asm extbas.asm
	$(shell mamou -q -mr equates.asm bas.asm extbas.asm -sa > ecb_equates.asm)
   
disk11.rom: ecb_equates.asm disk.asm
	mamou -q -mr ecb_equates.asm disk.asm -o$@

coco3.rom: equates.asm coco3.asm
	mamou -q -mr equates.asm coco3.asm -o$@
   
hdbtc3.rom: ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aTC3 -q -mr ecb_equates.asm hdbdos.asm -o$@

hdbkenton.rom: ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aKENTON -q -mr ecb_equates.asm hdbdos.asm -o$@

hdblrtech.rom: ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aLRTECH -q -mr ecb_equates.asm hdbdos.asm -o$@

hdbd4n1.rom: ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aD4N1 -q -mr ecb_equates.asm hdbdos.asm -o$@

hdbdhdii.rom: ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aDHDII -q -mr ecb_equates.asm hdbdos.asm -o$@

hdblba.rom: ecb_equates.asm hdbdos.asm
	mamou -aIDE -aUSELBA -q -mr ecb_equates.asm hdbdos.asm -o$@

hdbchs.rom: ecb_equates.asm hdbdos.asm
	mamou -aIDE -aUSECHS -q -mr ecb_equates.asm hdbdos.asm -o$@

hdbdw3cc2.rom: ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=1 -q -mr ecb_equates.asm hdbdos.asm -o$@

hdbdw3cc3.rom: ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=2 -q -mr ecb_equates.asm hdbdos.asm -o$@

hdbdw4cc3.rom: ecb_equates.asm hdbdos.asm
	mamou -aDW -aDW4 -aLevel=2 -q -mr ecb_equates.asm hdbdos.asm -o$@

clean:
	-rm -f bas13.rom cb_equates.asm ecb_equates.asm extbas11.rom disk11.rom coco3.rom \
	$(HDB)
