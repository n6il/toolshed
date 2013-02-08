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
	hdbdw4cc2.wav hdbdw3arduino.wav

ALLOBJS         = preload postload $(HDBROM) $(HDBBIN) $(HDBCAS)

all: $(ALLOBJS)

preload: preload.asm
	mamou -q $< -mr -opreload

postload: postload.asm
	mamou -q $< -mr -opostload

# TC^3 SCSI
hdbtc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aTC3 -q -mr $+ -o$@

hdbtc3.bin: preload hdbtc3.rom postload 
	cat $+ > $@

hdbtc3.wav: hdbtc3.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBTC3
	rm hdbtmp

# Ken-Ton SCSI
hdbkenton.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aKENTON -q -mr $+ -o$@

hdbkenton.bin: preload hdbkenton.rom postload 
	cat $+ > $@

hdbkenton.wav: hdbkenton.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBKENTN
	rm hdbtmp

# LRTech SCSI
hdblrtech.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aLRTECH -q -mr $+ -o$@

hdblrtech.bin: preload hdblrtech.rom postload 
	cat $+ > $@

hdblrtech.wav: hdblrtech.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBLRTCH
	rm hdbtmp

# Disto 4-N-1 SCSI
hdbd4n1.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aD4N1 -q -mr $+ -o$@

hdbd4n1.bin: preload hdbd4n1.rom postload 
	cat $+ > $@

hdbd4n1.wav: hdbd4n1.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBD4N1
	rm hdbtmp

# Disto HD-II
hdbdhdii.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aDHDII -q -mr $+ -o$@

hdbdhdii.bin: preload hdbdhdii.rom postload 
	cat $+ > $@

hdbdhdii.wav: hdbdhdii.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDHDII
	rm hdbtmp

# IDE LBA
hdblba.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aIDE -aUSELBA -q -mr $+ -o$@

hdblba.bin: preload hdblba.rom postload 
	cat $+ > $@

hdblba.wav: hdblba.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBLBA
	rm hdbtmp

# IDE CHS
hdbchs.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aIDE -aUSECHS -q -mr $+ -o$@

hdbchs.bin: preload hdbchs.rom postload 
	cat $+ > $@

hdbchs.wav: hdbchs.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBCHS
	rm hdbtmp

# DW4 CoCo 3
hdbdw4cc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=2 -aDW4 -q -mr $+ -o$@

hdbdw4cc3.bin: preload hdbdw4cc3.rom postload 
	cat $+ > $@

hdbdw4cc3.wav: hdbdw4cc3.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3C3
	rm hdbtmp

# DW4 CoCo 2
hdbdw4cc2.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=1 -aDW4 -q -mr $+ -o$@

hdbdw4cc2.bin: preload hdbdw4cc2.rom postload 
	cat $+ > $@

hdbdw4cc2.wav: hdbdw4cc2.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3C3
	rm hdbtmp

# DW3 CoCo 1
hdbdw3cc1.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=1 -aBAUD38400 -q -mr $+ -o$@

hdbdw3cc1.bin: preload hdbdw3cc1.rom postload 
	cat $+ > $@

hdbdw3cc1.wav: hdbdw3cc1.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3C1
	rm hdbtmp

# DW3 CoCo 2
hdbdw3cc2.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=1 -q -mr $+ -o$@

hdbdw3cc2.bin: preload hdbdw3cc2.rom postload 
	cat $+ > $@

hdbdw3cc2.wav: hdbdw3cc2.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3C2
	rm hdbtmp

# DW3 CoCo 3
hdbdw3cc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=2 -q -mr $+ -o$@

hdbdw3cc3.bin: preload hdbdw3cc3.rom postload 
	cat $+ > $@

hdbdw3cc3.wav: hdbdw3cc3.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3C3
	rm hdbtmp

# DW3 CoCo Becker
hdbdw3bck.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=1 -aBECKER -q -mr $+ -o$@

hdbdw3bck.bin: preload hdbdw3bck.rom postload 
	cat $+ > $@

hdbdw3bck.wav: hdbdw3bck.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3BK
	rm hdbtmp

# DW3 CoCo 3 Becker
hdbdw3bc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=2 -aBECKER -q -mr $+ -o$@

hdbdw3bc3.bin: preload hdbdw3bc3.rom postload 
	cat $+ > $@

hdbdw3bc3.wav: hdbdw3bc3.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3B3
	rm hdbtmp

# DW3 CoCo 3 J&M CP FDC Becker
hdbdw3jc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=2 -aJMCPBCK -q -mr $+ -o$@

hdbdw3jc3.bin: preload hdbdw3jc3.rom postload 
	cat $+ > $@

hdbdw3jc3.wav: hdbdw3jc3.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3J3
	rm hdbtmp

# DW3 CoCo J&M CP FDC Becker
hdbdw3jc2.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=1 -aJMCPBCK -q -mr $+ -o$@

hdbdw3jc2.bin: preload hdbdw3jc2.rom postload 
	cat $+ > $@

hdbdw3jc2.wav: hdbdw3jc2.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3J2
	rm hdbtmp

# DW3 Arduino (Boisy)
hdbdw3arduino.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=1 -aARDUINO -q -mr $+ -o$@

hdbdw3arduino.bin: preload hdbdw3arduino.rom postload 
	cat $+ > $@

hdbdw3arduino.wav: hdbdw3arduino.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3ARD
	rm hdbtmp


clean:
	-rm -f preload postload $(HDBROM) $(HDBBIN) $(HDBCAS)
