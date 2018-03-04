
DEPENDS		= ./Makefile

#AS		= mamou -r -q
AS		= lwasm -r --pragma=condundefzero
ASOUT		= -o
ECHO		= echo
MAKEWAV		= makewav -r -c

ROMS		= dw3dos_mb_cc3.rom dw3dos_mb_cc2.rom dw3dos_mb_cc1.rom dw3dos_mb_rs232_pak.rom \
		dw3dos_mb_bck.rom dw3dos_mb_directmodem_pak.rom \
		dw3dos_dsk_cc3.rom dw3dos_dsk_cc2.rom dw3dos_dsk_cc1.rom dw3dos_dsk_rs232_pak.rom \
		dw3dos_dsk_bck.rom dw3dos_dsk_directmodem_pak.rom \
		dw3doscc3.bin dw3doscc2.bin dw3doscc1.bin dw3dos_rs232_pak.bin dw3dosbck.bin \
		dw3dos_directmodem_pak.bin

HSROMS		= dw4dos_mb_cc3.rom dw4dos_mb_cc2.rom \
		dw4dos_dsk_cc3.rom dw4dos_dsk_cc2.rom \
		dw4doscc3.bin dw4doscc2.bin

CASSFILES	= dw3doscc1.wav dw3doscc2.wav dw3doscc3.wav dw3dos_rs232_pak.wav dw3dos_directmodem_pak.wav

HSCASSFILES	= dw4doscc2.wav dw4doscc3.wav

TRACKS		= dw3dos_cc3.trk dw3dos_cc2.trk dw3dos_cc1.trk dw3dos_rs232_pak.trk dw3dos_directmodem_pak.trk

HSTRACKS	= dw4dos_cc3.trk dw4dos_cc2.trk

ALLOBJS		= $(ROMS) $(CASSFILES) $(TRACKS)

HISPEED		= $(HSROMS) $(HSCASSFILES) $(HSTRACKS)

all:	banner $(ALLOBJS) $(DEPENDS) 

experimental:	all $(HISPEED)
banner:
	@$(ECHO) "**************************************************"
	@$(ECHO) "*                                                *"
	@$(ECHO) "*          DriveWire 3/4 DOS Files               *"
	@$(ECHO) "*                                                *"
	@$(ECHO) "**************************************************"

dsk:	objs

# DW3DOS ROMs
dw3dos_mb_bck.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DBECKER=1

dw3dos_mb_directmodem_pak.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DSY6551N=1 -DSY6551B=65388

dw3dos_mb_rs232_pak.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DSY6551N=1

dw3dos_mb_cc3.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=3

dw3dos_mb_cc2.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2

dw3dos_mb_cc1.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=1 -DBAUD38400=1

dw3dos_dsk_bck.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDISKROM=1 -DBECKER=1

dw3dos_dsk_directmodem_pak.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDISKROM=1 -DSY6551N=1 -DSY6551B=65388

dw3dos_dsk_rs232_pak.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDISKROM=1 -DSY6551N=1

dw3dos_dsk_cc3.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=3 -DDISKROM=1

dw3dos_dsk_cc2.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDISKROM=1

dw3dos_dsk_cc1.rom:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=1 -DDISKROM=1 -DBAUD38400=1

dw3dosbck.bin:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DBIN=1 -b -DBECKER=1

dw3dos_directmodem_pak.bin:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DBIN=1 -b -DSY6551N=1 -DSY6551B=65388

dw3dos_rs232_pak.bin:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DBIN=1 -b -DSY6551N=1

dw3doscc3.bin:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=3 -DBIN=1 -b

dw3doscc2.bin:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DBIN=1 -b

dw3doscc1.bin:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=1 -DBIN=1 -b -DBAUD38400=1

dw3dos_directmodem_pak.wav:	dw3dos_directmodem_pak.bin
	$(MAKEWAV) -o$@ -nDW3DOSDM $<

dw3dos_rs232_pak.wav:	dw3dos_rs232_pak.bin
	$(MAKEWAV) -o$@ -nDW3DOSRS $<

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

dw4doscc3.wav:	dw4doscc3.bin
	$(MAKEWAV) -o$@ -nDW3DOSC3 $<

dw4doscc2.wav:	dw4doscc2.bin
	$(MAKEWAV) -o$@ -nDW3DOSC2 $<

# DW3DOS Tracks
dw3dosbck.trk:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDOSTRACK -DBECKER=1

dw3dos_directmodem_pak.trk:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDOSTRACK -DSY6551N=1 -DSY6551B=65388

dw3dos_rs232_pak.trk:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDOSTRACK -DSY6551N=1

dw3dos_cc3.trk:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=3 -DDOSTRACK

dw3dos_cc2.trk:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDOSTRACK

dw3dos_cc1.trk:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=1 -DDOSTRACK -DBAUD38400=1

# DW4DOS Tracks
dw4dos_cc3.trk:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=3 -DDOSTRACK -DDW4=1

dw4dos_cc2.trk:	dw3dos.asm
	$(AS) $(ASOUT)$@ $< $(AFLAGS) -DCoCo=2 -DDOSTRACK -DDW4=1

drivewire.zip: $(ALLOBJS)
	zip $@ $(ALLOBJS)

clean:
	$(RM) $(ALLOBJS) $(HISPEED) drivewire.zip

