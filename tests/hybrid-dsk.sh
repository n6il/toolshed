#!/bin/sh -e

DECB=$PWD/build/unix/decb/decb
OS9=$PWD/build/unix/os9/os9

OFFSET=$((0x5a000))

TDIR=$(mktemp -d)
cd $TDIR || exit 1

$OS9 format -q -e -l$OFFSET os9dsk
for i in 0 1 2
do
	$DECB dskini decbdsk$i
	echo "touch $i" > file$i
	$DECB copy file$i decbdsk$i,
	DECBDSKS="$DECBDSKS decbdsk$i"
done

cat os9dsk $DECBDSKS > franken.dsk

for i in 0 1 2
do
	$DECB dir franken.dsk,:$i+$OFFSET
done

echo list contents:
for i in 0 1 2
do
	$DECB list franken.dsk,file$i:$i+$OFFSET
done

cd ..
rm -r $TDIR
