DWDOS - for booting NitrOS-9 via DriveWire

DWDOS works like the DOS command in DECB, only that it loads
the operating system (typically NitrOS-9) via DriveWire instead
of from the floppy boot track.

Do not use the dw4* files! They are for DW turbo speed (experimental
feature in DW4, hence the name). You should use the dw3* files,
regardless of the DriveWire server software you are using.

The .rom files are for ROM: The _mb_ ones for replacing the
motherboard ROM (bye BASIC!), and the _dsk_ ones for replacing the ROM
in a disk controller.

The .bin files are DECB binaries, simply copy them to a DECB floppy.

The .trk files are boot track files that can be written to track 34 on
floppies so that the RSDOS DOS command will launch DWDOS from the
floppy. Shorter to type than LOAD"DWDOS" :)

The difference between _cc1 and _cc2 flavours is the baud rate.

See the Makefile for more hints about different flavours.

