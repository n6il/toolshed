HDBSCSI	= hdbtc3.rom hdbkenton.rom hdblrtech.rom hdbd4n1.rom hdbdhdii.rom
HDBIDE  = hdblba.rom hdbchs.rom
HDBDW   = hdbdw3cc1.rom hdbdw3cc2.rom hdbdw3cc3.rom \
	hdbdw3bck.rom hdbdw3bc3.rom hdbdw3bckt.rom hdbdw3bc3t.rom hdbdw3jc3.rom hdbdw3jc2.rom \
	hdbdw3arduino.rom hdbdw3sy.rom hdbdw3s3.rom hdbdw3dm.rom hdbdw3d3.rom hdbdw3dg.rom
HDBDW4  = hdbdw4cc3.rom hdbdw4cc2.rom
HDBROM  = $(HDBSCSI) $(HDBIDE) $(HDBDW)
HDBBIN	= hdbtc3.bin hdbkenton.bin hdblrtech.bin hdbd4n1.bin hdbdhdii.bin \
	hdblba.bin hdbchs.bin hdbdw3cc1.bin hdbdw3cc2.bin hdbdw3cc3.bin \
	hdbdw3bck.bin hdbdw3bc3.bin hdbdw3bc3t.bin hdbdw3bckt.bin hdbdw3jc3.bin hdbdw3jc2.bin \
	hdbdw3arduino.bin hdbdw3sy.bin hdbdw3s3.bin hdbdw3dm.bin hdbdw3dg.bin hdbdw3d3.bin
HDBBIN4 = hdbdw4cc2.bin hdbdw4cc3.bin
HDBCAS  = hdbtc3.wav hdbkenton.wav hdblrtech.wav hdbd4n1.wav hdbdhdii.wav \
	hdblba.wav hdbchs.wav hdbdw3cc1.wav hdbdw3cc2.wav hdbdw3cc3.wav \
	hdbdw3bck.wav hdbdw3bc3.wav hdbdw3jc3.wav hdbdw3jc2.wav \
	hdbdw3arduino.wav hdbdw3bckt.wav hdbdw3bc3t.wav \
	hdbdw3bck.cas hdbdw3bc3.cas hdbdw3bc3t.cas hdbdw3bckt.cas \
	hdbdw3sy.wav hdbdw3sy.cas hdbdw3s3.wav hdbdw3s3.cas hdbdw3dm.cas \
	hdbdw3d3.cas hdbdw3dg.cas hdbdw3dm.wav hdbdw3d3.wav hdbdw3dg.wav
HDBCAS4	= hdbdw4cc3.wav hdbdw4cc2.wav

ALLOBJS         = preload postload $(HDBROM) $(HDBBIN) $(HDBCAS)
HISPEED         = $(HDBDW4) $(HDBBIN4) $(HDBCAS4)

#AS		= mamou -r -q
AS		= lwasm -r --pragma=condundefzero,nodollarlocal,noindex0tonone
MERGE		= cat
MAKEWAV		= makewav -r -c
CECB		= cecb

all: $(ALLOBJS)
experimental: all $(HISPEED)

preload: preload.asm
	$(AS) $< -opreload

postload: postload.asm
	$(AS) $< -opostload

%.bin : preload %.rom postload
	$(MERGE) $+ > $@

%.rom : hdbdos.asm
	$(AS) $(EQUATES) hdbdos.asm -o$@ $(FLAGS)

%.wav : %.bin
	$(MAKEWAV) -o$@ -n$(LABEL) $<

%.cas : %.bin
	$(CECB) bulkerase $@
	$(CECB) copy -2 -b -g $< $@,$(LABEL)


# For CoCo 1/2/3
$(HDBROM) $(HDBDW4): EQUATES = ../cocoroms/ecb_equates.asm
$(HDBROM) $(HDBDW4): ../cocoroms/ecb_equates.asm

# TC^3 SCSI
hdbtc3.rom:    FLAGS = -DSCSI -DTC3
hdbtc3.wav:    LABEL = HDBTC3

# Ken-Ton SCSI
hdbkenton.rom: FLAGS = -DSCSI -DKENTON
hdbkenton.wav: LABEL = HDBKENTN

# LRTech SCSI
hdblrtech.rom: FLAGS = -DSCSI -DLRTECH
hdblrtech.wav: LABEL = HDBLRTCH

# Disto 4-N-1 SCSI
hdbd4n1.rom:   FLAGS = -DSCSI -DD4N1
hdbd4n1.wav:   LABEL = HDBD4N1

# Disto HD-II
hdbdhdii.rom:  FLAGS = -DSCSI -DDHDII
hdbdhdii.wav:  LABEL = HDBDHDII

# IDE LBA
hdblba.rom:    FLAGS = -DIDE -DUSELBA
hdblba.wav:    LABEL = HDBLBA

# IDE CHS
hdbchs.rom:    FLAGS = -DIDE -DUSECHS
hdbchs.wav:    LABEL = HDBCHS

# DW4 CoCo 3
hdbdw4cc3.rom: FLAGS = -DDW -DLevel=2 -DDW4
hdbdw4cc3.wav: LABEL = HDBDW4C3

# DW4 CoCo 2
hdbdw4cc2.rom: FLAGS = -DDW -DLevel=1 -DDW4
hdbdw4cc2.wav: LABEL = HDBDW4C2

# DW3 CoCo 3
hdbdw3cc3.rom: FLAGS = -DDW -DLevel=2
hdbdw3cc3.wav: LABEL = HDBDW3C3

# DW3 CoCo 2
hdbdw3cc2.rom: FLAGS = -DDW -DLevel=1
hdbdw3cc2.wav: LABEL = HDBDW3C2

# DW3 CoCo 1
hdbdw3cc1.rom: FLAGS = -DDW -DLevel=1 -DBAUD38400
hdbdw3cc1.wav: LABEL = HDBDW3C1

# DW3 CoCo Becker
hdbdw3bck.rom: FLAGS = -DDW -DLevel=1 -DBECKER
hdbdw3bck.wav: LABEL = HDBDW3BK
hdbdw3bck.cas: LABEL = HDBDW3BK

# DW3 CoCo Mode Dragon64/Tano Dragon64  RS232 Port Base Address FF04
hdbdw3dg.rom: FLAGS = -DDW -DLevel=1 -DSY6551N -DSY6551B=65284
hdbdw3dg.wav: LABEL = HDBDW3DG
hdbdw3dg.cas: LABEL = HDBDW3DG

# DW3 CoCo Direct Modem RS232 Pak Base Address FF6C
hdbdw3dm.rom: FLAGS = -DDW -DLevel=1 -DSY6551N -DSY6551B=65388
hdbdw3dm.wav: LABEL = HDBDW3DM
hdbdw3dm.cas: LABEL = HDBDW3DM

# DW3 CoCo 3 Direct Modem RS232 Pak Base Address FF6C
hdbdw3d3.rom: FLAGS = -DDW -DLevel=3 -DSY6551N -DSY6551B=65388
hdbdw3d3.wav: LABEL = HDBDW3D3
hdbdw3d3.cas: LABEL = HDBDW3D3

# DW3 CoCo Deluxe RS232 Pak Base Address FF68
hdbdw3sy.rom: FLAGS = -DDW -DLevel=1 -DSY6551N
hdbdw3sy.wav: LABEL = HDBDW3SY
hdbdw3sy.cas: LABEL = HDBDW3SY

# DW3 CoCo 3 Deluxe RS232 Pak Base Address FF68
hdbdw3s3.rom: FLAGS = -DDW -DLevel=2 -DSY6551N
hdbdw3s3.wav: LABEL = HDBDW3S3
hdbdw3s3.cas: LABEL = HDBDW3S3

# DW3 CoCo Becker With Timeout
hdbdw3bckt.rom: FLAGS = -DDW -DLevel=1 -DBECKERTO
hdbdw3bckt.wav: LABEL = HDBDW3BK
hdbdw3bckt.cas: LABEL = HDBDW3BK

# DW3 CoCo 3 Becker
hdbdw3bc3.rom: FLAGS = -DDW -DLevel=2 -DBECKER
hdbdw3bc3.wav: LABEL = HDBDW3B3
hdbdw3bc3.cas: LABEL = HDBDW3B3

# DW3 CoCo 3 Becker With Timeout
hdbdw3bc3t.rom: FLAGS = -DDW -DLevel=2 -DBECKERTO
hdbdw3bc3t.wav: LABEL = HDBDW3B3
hdbdw3bc3t.cas: LABEL = HDBDW3B3

# DW3 CoCo 3 J&M CP FDC Becker
hdbdw3jc3.rom: FLAGS = -DDW -DLevel=2 -DJMCPBCK
hdbdw3jc3.wav: LABEL = HDBDW3J3

# DW3 CoCo J&M CP FDC Becker
hdbdw3jc2.rom: FLAGS = -DDW -DLevel=1 -DJMCPBCK
hdbdw3jc2.wav: LABEL = HDBDW3J2

# DW3 Arduino (Boisy)
hdbdw3arduino.rom: FLAGS = -DDW -DLevel=1 -DARDUINO
hdbdw3arduino.wav: LABEL = HDBDW3ARD


DRAGONROM = hdbdw3dgn32ram.rom hdbdw4dgn32ram.rom hdblbadgn.rom hdbdw3dgn.rom hdbdw4dgn.rom
DRAGONBIN = hdbdw3dgn32ram.bin hdbdw4dgn32ram.bin
DRAGONWAV = hdbdw3dgn32ram.wav hdbdw4dgn32ram.wav

dragon: $(DRAGONROM) $(DRAGONWAV)

$(DRAGONROM): ../cocoroms/equates.asm ../cocoroms/dragon_equivs.asm
$(DRAGONROM): EQUATES = ../cocoroms/equates.asm ../cocoroms/dragon_equivs.asm

# Dragon 32, DW3 run from lower RAM
hdbdw3dgn32ram.rom: FLAGS = -DDW -DORG=0x6000 -DLevel=1 -DDRAGON -b
hdbdw3dgn32ram.wav: LABEL = HDBDW3DR
# This "rom" is already in DECB binary format (to carry ORG), so just copy it
hdbdw3dgn32ram.bin: hdbdw3dgn32ram.rom
	$(MERGE) $+ > $@

# Dragon 32, DW4 run from lower RAM
hdbdw4dgn32ram.rom: FLAGS =  -DDW -DORG=0x6000 -DLevel=1 -DDRAGON -DDW4 -b
hdbdw4dgn32ram.wav: LABEL = HDBDW4DR
hdbdw4dgn32ram.bin: hdbdw4dgn32ram.rom
	$(MERGE) $+ > $@

# Dragon 32, DW3 ROM cartridge
hdbdw3dgn.rom: FLAGS = -DDW -DLevel=1 -DDRAGON

# Dragon 32, DW4 (turbo speed) ROM cartridge
hdbdw4dgn.rom: FLAGS = -DDW -DLevel=1 -DDRAGON -DDW4

# Dragon, IDE LBA for ROM cartridge
hdblbadgn.rom: FLAGS = -DIDE -DUSELBA -DLevel=1 -DDRAGON

clean:
	-rm -f preload postload $(HDBROM) $(HDBBIN) $(HDBCAS) \
	 $(DRAGONROM) $(DRAGONWAV) $(DRAGONBIN) $(HISPEED)
