HDBSCSI	= hdbtc3.rom hdbkenton.rom hdblrtech.rom hdbd4n1.rom hdbdhdii.rom
HDBIDE  = hdblba.rom hdbchs.rom
HDBDW   = hdbdw3cc1.rom hdbdw3cc2.rom hdbdw3cc3.rom hdbdw3bck.rom hdbdw3bc3.rom \
	hdbdw3jc3.rom hdbdw3jc2.rom hdbdw4cc3.rom hdbdw4cc2.rom
HDBROM  = $(HDBSCSI) $(HDBIDE) $(HDBDW)
HDBBIN	= hdbtc3.bin hdbkenton.bin hdblrtech.bin hdbd4n1.bin hdbdhdii.bin \
	hdblba.bin hdbchs.bin hdbdw3cc1.bin hdbdw3cc2.bin hdbdw3cc3.bin \
	hdbdw3bck.bin hdbdw3bc3.bin hdbdw3jc3.bin hdbdw3jc2.bin hdbdw4cc3.bin \
	hdbdw4cc2.bin
HDBCAS  = hdbtc3.wav hdbkenton.wav hdblrtech.wav hdbd4n1.wav hdbdhdii.wav \
	hdblba.wav hdbchs.wav hdbdw3cc1.wav hdbdw3cc2.wav hdbdw3cc3.wav \
	hdbdw3bck.wav hdbdw3bc3.wav hdbdw3jc3.wav hdbdw3jc2.wav hdbdw4cc3.wav \
	hdbdw4cc2.wav

ALLOBJS         = preload postload $(HDBROM) $(HDBBIN) $(HDBCAS)

all: $(ALLOBJS)

preload: preload.asm
	mamou -q preload.asm -mr -opreload

postload: postload.asm
	mamou -q postload.asm -mr -opostload

# TC^3 SCSI
hdbtc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aTC3 -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbtc3.bin: preload postload hdbtc3.rom
	cat preload hdbtc3.rom postload > $@

hdbtc3.wav: hdbtc3.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBTC3
	rm hdbtmp

# Ken-Ton SCSI
hdbkenton.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aKENTON -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbkenton.bin: preload postload hdbkenton.rom
	cat preload hdbkenton.rom postload > $@

hdbkenton.wav: hdbkenton.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBKENTN
	rm hdbtmp

# LRTech SCSI
hdblrtech.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aLRTECH -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdblrtech.bin: preload postload hdblrtech.rom
	cat preload hdblrtech.rom postload > $@

hdblrtech.wav: hdblrtech.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBLRTCH
	rm hdbtmp

# Disto 4-N-1 SCSI
hdbd4n1.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aD4N1 -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbd4n1.bin: preload postload hdbd4n1.rom
	cat preload hdbd4n1.rom postload > $@

hdbd4n1.wav: hdbd4n1.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBD4N1
	rm hdbtmp

# Disto HD-II
hdbdhdii.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aSCSI -aDHDII -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdhdii.bin: preload postload hdbdhdii.rom
	cat preload hdbdhdii.rom postload > $@

hdbdhdii.wav: hdbdhdii.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDHDII
	rm hdbtmp

# IDE LBA
hdblba.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aIDE -aUSELBA -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdblba.bin: preload postload hdblba.rom
	cat preload hdblba.rom postload > $@

hdblba.wav: hdblba.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBLBA
	rm hdbtmp

# IDE CHS
hdbchs.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aIDE -aUSECHS -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbchs.bin: preload postload hdbchs.rom
	cat preload hdbchs.rom postload > $@

hdbchs.wav: hdbchs.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBCHS
	rm hdbtmp

# DW4 CoCo 3
hdbdw4cc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=2 -aDW4 -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdw4cc3.bin: preload postload hdbdw4cc3.rom
	cat preload hdbdw4cc3.rom postload > $@

hdbdw4cc3.wav: hdbdw4cc3.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3C3
	rm hdbtmp

# DW4 CoCo 2
hdbdw4cc2.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=1 -aDW4 -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdw4cc2.bin: preload postload hdbdw4cc2.rom
	cat preload hdbdw4cc2.rom postload > $@

hdbdw4cc2.wav: hdbdw4cc2.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3C3
	rm hdbtmp

# DW3 CoCo 1
hdbdw3cc1.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=1 -aBAUD38400 -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdw3cc1.bin: preload postload hdbdw3cc1.rom
	cat preload hdbdw3cc2.rom postload > $@

hdbdw3cc1.wav: hdbdw3cc1.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3C1
	rm hdbtmp

# DW3 CoCo 2
hdbdw3cc2.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=1 -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdw3cc2.bin: preload postload hdbdw3cc2.rom
	cat preload hdbdw3cc2.rom postload > $@

hdbdw3cc2.wav: hdbdw3cc2.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3C2
	rm hdbtmp

# DW3 CoCo 3
hdbdw3cc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=2 -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdw3cc3.bin: preload postload hdbdw3cc3.rom
	cat preload hdbdw3cc3.rom postload > $@

hdbdw3cc3.wav: hdbdw3cc3.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3C3
	rm hdbtmp

# DW3 CoCo Becker
hdbdw3bck.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel1 -aBECKER -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdw3bck.bin: preload postload hdbdw3bck.rom
	cat preload hdbdw3bck.rom postload > $@

hdbdw3bck.wav: hdbdw3bck.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3BK
	rm hdbtmp

# DW3 CoCo 3 Becker
hdbdw3bc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=2 -aBECKER -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdw3bc3.bin: preload postload hdbdw3bc3.rom
	cat preload hdbdw3bc3.rom postload > $@

hdbdw3bc3.wav: hdbdw3bc3.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3B3
	rm hdbtmp

# DW3 CoCo 3 J&M CP FDC Becker
hdbdw3jc3.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel=2 -aJMCPBCK -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdw3jc3.bin: preload postload hdbdw3jc3.rom
	cat preload hdbdw3bc3.rom postload > $@

hdbdw3jc3.wav: hdbdw3jc3.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3B3
	rm hdbtmp

# DW3 CoCo J&M CP FDC Becker
hdbdw3jc2.rom: ../cocoroms/ecb_equates.asm hdbdos.asm
	mamou -aDW -aLevel1 -aJMCPBCK -q -mr ../cocoroms/ecb_equates.asm hdbdos.asm -o$@

hdbdw3jc2.bin: preload postload hdbdw3jc2.rom
	cat preload hdbdw3bck.rom postload > $@

hdbdw3jc2.wav: hdbdw3jc2.bin
	dd if=$< bs=1 skip=5 of=hdbtmp count=8240
	makewav hdbtmp -r -d0x4fd0 -e0x4fd0 -o$@ -nHDBDW3BK
	rm hdbtmp

clean:
	-rm -f preload postload $(HDBROM) $(HDBBIN) $(HDBCAS)
