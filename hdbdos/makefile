HDBSCSI	= hdbtc3.rom hdbkenton.rom hdblrtech.rom hdbd4n1.rom hdbdhdii.rom
HDBIDE  = hdblba.rom hdbchs.rom
HDBDW   = hdbdw3cc1.rom hdbdw3cc2.rom hdbdw3cc3.rom # hdbdw4cc3.rom
HDBROM  = $(HDBSCSI) $(HDBIDE) $(HDBDW)
HDBBIN	= hdbtc3.bin hdbkenton.bin hdblrtech.bin hdbd4n1.bin hdbdhdii.bin \
	hdblba.bin hdbchs.bin hdbdw3cc1.bin hdbdw3cc2.bin hdbdw3cc3.bin
HDBCAS  = hdbtc3.wav hdbkenton.wav hdblrtech.wav hdbd4n1.wav hdbdhdii.wav \
	hdblba.wav hdbchs.wav hdbdw3cc1.wav hdbdw3cc2.wav hdbdw3cc3.wav

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

clean:
	-rm -f preload postload $(HDBROM) $(HDBBIN) $(HDBCAS)
