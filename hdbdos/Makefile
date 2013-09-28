HDBSCSI	= hdbtc3.rom hdbkenton.rom hdblrtech.rom hdbd4n1.rom hdbdhdii.rom
HDBIDE  = hdblba.rom hdbchs.rom
HDBDW   = hdbdw3cc1.rom hdbdw3cc2.rom hdbdw3cc3.rom \
	hdbdw4cc3.rom hdbdw4cc2.rom \
	hdbdw3bck.rom hdbdw3bc3.rom hdbdw3jc3.rom hdbdw3jc2.rom \
	hdbdw3arduino.rom
HDBROM  = $(HDBSCSI) $(HDBIDE) $(HDBDW)
HDBBIN	= hdbtc3.bin hdbkenton.bin hdblrtech.bin hdbd4n1.bin hdbdhdii.bin \
	hdblba.bin hdbchs.bin hdbdw3cc1.bin hdbdw3cc2.bin hdbdw3cc3.bin \
	hdbdw4cc2.bin hdbdw4cc3.bin \
	hdbdw3bck.bin hdbdw3bc3.bin hdbdw3jc3.bin hdbdw3jc2.bin \
	hdbdw3arduino.bin
HDBCAS  = hdbtc3.wav hdbkenton.wav hdblrtech.wav hdbd4n1.wav hdbdhdii.wav \
	hdblba.wav hdbchs.wav hdbdw3cc1.wav hdbdw3cc2.wav hdbdw3cc3.wav \
	hdbdw3bck.wav hdbdw3bc3.wav hdbdw3jc3.wav hdbdw3jc2.wav hdbdw4cc3.wav \
	hdbdw4cc2.wav hdbdw3arduino.wav \
	hdbdw3bck.cas hdbdw3bc3.cas

ALLOBJS         = preload postload $(HDBROM) $(HDBBIN) $(HDBCAS)

#AS		= mamou -r -q
AS		= lwasm -r --pragma=condundefzero,nodollarlocal,noindex0tonone
MERGE		= cat
MAKEWAV		= makewav -r -c
CECB		= cecb

all: $(ALLOBJS)

preload: preload.asm
	$(AS) $< -opreload

postload: postload.asm
	$(AS) $< -opostload

%.bin : preload %.rom postload
	$(MERGE) $+ > $@

# TC^3 SCSI
hdbtc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DSCSI -DTC3 $+ -o$@

hdbtc3.wav: hdbtc3.bin
	$(MAKEWAV) -o$@ -nHDBTC3 $<

# Ken-Ton SCSI
hdbkenton.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DSCSI -DKENTON $+ -o$@

hdbkenton.wav: hdbkenton.bin
	$(MAKEWAV) -o$@ -nHDBKENTN $<

# LRTech SCSI
hdblrtech.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DSCSI -DLRTECH $+ -o$@

hdblrtech.wav: hdblrtech.bin
	$(MAKEWAV) -o$@ -nHDBLRTCH $<

# Disto 4-N-1 SCSI
hdbd4n1.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DSCSI -DD4N1 $+ -o$@

hdbd4n1.wav: hdbd4n1.bin
	$(MAKEWAV) -o$@ -nHDBD4N1 $<

# Disto HD-II
hdbdhdii.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DSCSI -DDHDII $+ -o$@

hdbdhdii.wav: hdbdhdii.bin
	$(MAKEWAV) -o$@ -nHDBDHDII $<

# IDE LBA
hdblba.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DIDE -DUSELBA $+ -o$@

hdblba.wav: hdblba.bin
	$(MAKEWAV) -o$@ -nHDBLBA $<

# IDE CHS
hdbchs.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DIDE -DUSECHS $+ -o$@

hdbchs.wav: hdbchs.bin
	$(MAKEWAV) -o$@ -nHDBCHS $<

# DW4 CoCo 3
hdbdw4cc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DDW -DLevel=2 -DDW4 $+ -o$@

hdbdw4cc3.wav: hdbdw4cc3.bin
	$(MAKEWAV) -o$@ -nHDBDW4C3 $<

# DW4 CoCo 2
hdbdw4cc2.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DDW -DLevel=1 -DDW4 $+ -o$@

hdbdw4cc2.wav: hdbdw4cc2.bin
	$(MAKEWAV) -o$@ -nHDBDW4C2 $<

# DW3 CoCo 1
hdbdw3cc1.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DDW -DLevel=1 -DBAUD38400 $+ -o$@

hdbdw3cc1.wav: hdbdw3cc1.bin
	$(MAKEWAV) -o$@ -nHDBDW3C1 $<

# DW3 CoCo 2
hdbdw3cc2.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DDW -DLevel=1 $+ -o$@

hdbdw3cc2.wav: hdbdw3cc2.bin
	$(MAKEWAV) -o$@ -nHDBDW3C2 $<

# DW3 CoCo 3
hdbdw3cc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DDW -DLevel=2 $+ -o$@

hdbdw3cc3.wav: hdbdw3cc3.bin
	$(MAKEWAV) -o$@ -nHDBDW3C3 $<

# DW3 CoCo Becker
hdbdw3bck.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DDW -DLevel=1 -DBECKER $+ -o$@

hdbdw3bck.wav: hdbdw3bck.bin
	$(MAKEWAV) -o$@ -nHDBDW3BK $<

hdbdw3bck.cas: hdbdw3bck.bin
	$(CECB) bulkerase $@
	$(CECB) copy -2 -b -g $< $@,HDBDW3BK

# DW3 CoCo 3 Becker
hdbdw3bc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DDW -DLevel=2 -DBECKER $+ -o$@

hdbdw3bc3.wav: hdbdw3bc3.bin
	$(MAKEWAV) -o$@ -nHDBDW3B3 $<

hdbdw3bc3.cas: hdbdw3bc3.bin
	$(CECB) bulkerase $@
	$(CECB) copy -2 -b -g $< $@,HDBDW3B3

# DW3 CoCo 3 J&M CP FDC Becker
hdbdw3jc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DDW -DLevel=2 -DJMCPBCK $+ -o$@

hdbdw3jc3.wav: hdbdw3jc3.bin
	$(MAKEWAV) -o$@ -nHDBDW3J3 $<

# DW3 CoCo J&M CP FDC Becker
hdbdw3jc2.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DDW -DLevel=1 -DJMCPBCK $+ -o$@

hdbdw3jc2.wav: hdbdw3jc2.bin
	$(MAKEWAV) -o$@ -nHDBDW3J2 $<

# DW3 Arduino (Boisy)
hdbdw3arduino.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	$(AS) -DDW -DLevel=1 -DARDUINO $+ -o$@

hdbdw3arduino.wav: hdbdw3arduino.bin
	$(MAKEWAV) -o$@ -nHDBDW3ARD $<

# Dragon example, DW3 run from lower RAM
hdbdw3dgn32ram.bin: ../cocoroms/ecb_equates.asm ../cocoroms/dragon_equivs.asm hdbdos.asm
	$(AS) -b -DDW -DORG=0x6000 -DLevel=1 -DDRAGON $+ -o$@
hdbdw3dgn32ram.wav: hdbdw3dgn32ram.bin
	$(MAKEWAV) -o$@ -nHDBDW3DR $<

# Dragon example, DW4 run from lower RAM
hdbdw4dgn32ram.bin: ../cocoroms/ecb_equates.asm ../cocoroms/dragon_equivs.asm hdbdos.asm
	$(AS) -b -DDW -DDW4 -DORG=0x6000 -DLevel=1 -DDRAGON $+ -o$@
hdbdw4dgn32ram.wav: hdbdw4dgn32ram.bin
	$(MAKEWAV) -o$@ -nHDBDW4DR $<

# Dragon example, IDE LBA for ROM cartridge
hdblbadgn.rom: ../cocoroms/ecb_equates.asm ../cocoroms/dragon_equivs.asm hdbdos.asm
	$(AS) -DIDE -DUSELBA -DLevel=1 -DDRAGON $+ -o$@

clean:
	-rm -f preload postload $(HDBROM) $(HDBBIN) $(HDBCAS)
