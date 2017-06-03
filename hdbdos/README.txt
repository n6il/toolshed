HDBDOS 1.5

Do not use the dw4* files! They are for DW turbo speed (an
experimental feature in DW4, hence the name). You should use
the dw3* files, regardless of the DriveWire server software
you are using.

The .rom files are for replacing the ROM in a disk controller.

The .bin files are DECB binaries, simply copy them to a DECB floppy.
You can then chain-load HDBDOS from DECB, or from another flavor
of HDBDOS.

The .wav files allow you to load HDBDOS via cassette.

The difference between _cc1 and _cc2 flavours is only the baud rate.

See the Makefile for more hints about different flavours.

