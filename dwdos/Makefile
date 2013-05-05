
DEPENDS		= ./Makefile

#AS		= mamou -r -q
AS		= lwasm -r --pragma=condundefzero
ASOUT		= -o
ECHO		= echo
MAKEWAV		= makewav -r -c

ROMS		= dw3dos_mb_cc3.rom dw3dos_mb_cc2.rom dw3dos_mb_cc1.rom \
		dw3dos_dsk_cc3.rom dw3dos_dsk_cc2.rom dw3dos_dsk_cc1.rom \
		dw3doscc3.bin dw3doscc2.bin dw3doscc1.bin \
		dw4dos_mb_cc3.rom dw4dos_mb_cc2.rom \
		dw4dos_dsk_cc3.rom dw4dos_dsk_cc2.rom \
		dw4doscc3.bin dw4doscc2.bin

CASSFILES	= dw3doscc1.wav dw3doscc2.wav dw3doscc3.wav \
		dw4doscc2.wav dw4doscc3.wav	

TRACKS		= dw3dos_cc3.trk dw3dos_cc2.trk dw3dos_cc1.trk \
		dw4dos_cc3.trk dw4dos_cc2.trk

ALLOBJS		= $(ROMS) $(CASSFILES) $(TRACKS)

all:	banner $(ALLOBJS) $(DEPENDS) 

banner:
	@$(ECHO) "**************************************************"
	@$(ECHO) "*                                                *"
	@$(ECHO) "*          DriveWire 3/4 DOS Files               *"
	@$(ECHO) "*                                                *"
	@$(ECHO) "**************************************************"

dsk:	objs

# DW3DOS ROMs
dw3dos_mb_cc3.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=3

dw3dos_mb_cc2.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2

dw3dos_mb_cc1.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=1

dw3dos_dsk_cc3.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=3 -DDISKROM=1

dw3dos_dsk_cc2.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDISKROM=1

dw3dos_dsk_cc1.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=1 -DDISKROM=1

dw3doscc3.bin:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=3 -DBIN=1 -b

dw3doscc2.bin:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DBIN=1 -b

dw3doscc1.bin:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=1 -DBIN=1 -b

dw3doscc3.wav:	dw3doscc3.bin
	$(MAKEWAV) -o$@ -nDW3DOSC3 $<

dw3doscc2.wav:	dw3doscc2.bin
	$(MAKEWAV) -o$@ -nDW3DOSC2 $<

dw3doscc1.wav:	dw3doscc1.bin
	$(MAKEWAV) -o$@ -nDW3DOSC1 $<

# DW4DOS ROMs
dw4dos_mb_cc3.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=3 -DDW4=1

dw4dos_mb_cc2.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDW4=1

dw4dos_dsk_cc3.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=3 -DDISKROM=1 -DDW4=1

dw4dos_dsk_cc2.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDISKROM=1 -DDW4=1

dw4doscc3.bin:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=3 -DBIN=1 -b -DDW4=1

dw4doscc2.bin:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DBIN=1 -b -DDW4=1

dw4doscc3.wav:	dw3doscc3.bin
	$(MAKEWAV) -o$@ -nDW3DOSC3 $<

dw4doscc2.wav:	dw3doscc2.bin
	$(MAKEWAV) -o$@ -nDW3DOSC2 $<

# DW3DOS Tracks
dw3dos_cc3.trk:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=3 -DDOSTRACK

dw3dos_cc2.trk:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDOSTRACK

dw3dos_cc1.trk:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=1 -DDOSTRACK

# DW4DOS Tracks
dw4dos_cc3.trk:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=3 -DDOSTRACK -DDW4=1

dw4dos_cc2.trk:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDOSTRACK -DDW4=1

drivewire.zip: $(ALLOBJS)
	zip $@ $(ALLOBJS)

clean:
	$(RM) $(ALLOBJS) drivewire.zip

