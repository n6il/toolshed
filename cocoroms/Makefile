all: bas13.rom extbas11.rom disk11.rom coco3.rom

bas13.rom: equates.asm bas.asm
	lwasm equates.asm bas.asm -o$@ --raw

cb_equates.asm: equates.asm bas.asm
	$(shell lwasm equates.asm bas.asm --symbol-dump > cb_equates.asm)

extbas11.rom: bas.asm cb_equates.asm extbas.asm equates.asm
	lwasm cb_equates.asm extbas.asm -o$@ --raw

ecb_equates.asm: equates.asm bas.asm extbas.asm
	$(shell lwasm equates.asm bas.asm extbas.asm --symbol-dump > ecb_equates.asm)
   
disk11.rom: ecb_equates.asm disk.asm
	lwasm ecb_equates.asm disk.asm -o$@ --raw

coco3.rom: equates.asm coco3.asm
	lwasm equates.asm coco3.asm -o$@ --raw
   
clean:
	-rm -f bas13.rom cb_equates.asm ecb_equates.asm extbas11.rom disk11.rom coco3.rom
	
