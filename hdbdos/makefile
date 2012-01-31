HDBSCSI	= hdbtc3.rom hdbkenton.rom hdblrtech.rom hdbd4n1.rom hdbdhdii.rom
HDBIDE  = hdblba.rom hdbchs.rom
HDBDW   = hdbdw3cc2.rom hdbdw3cc3.rom hdbdw4cc3.rom
HDB     = $(HDBSCSI) $(HDBIDE) $(HDBDW)

ALLOBJS         = $(HDB)

all: $(ALLOBJS)

hdbtc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aTC3 -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbkenton.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aKENTON -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdblrtech.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aLRTECH -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbd4n1.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aD4N1 -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdhdii.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aDHDII -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdblba.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aIDE -aUSELBA -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbchs.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aIDE -aUSECHS -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdw3cc2.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=1 -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdw3cc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=2 -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdw4cc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aDW4 -aLevel=2 -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

clean:
	-rm -f $(HDB)
