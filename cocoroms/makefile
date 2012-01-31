all: bas13.rom extbas11.rom disk11.rom coco3.rom

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
   
clean:
	-rm -f bas13.rom cb_equates.asm ecb_equates.asm extbas11.rom disk11.rom coco3.rom
	
