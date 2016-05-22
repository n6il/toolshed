#!/bin/sh -e

DECB=$PWD/build/unix/decb/decb

TDIR=$(mktemp -d)

cd $TDIR || exit 1

$DECB dskini multidsk -h256

for i in 0 1 2
do
	echo "touch $i" > file$i
	$DECB copy file$i multidsk,:$i
done

for i in 0 1 2
do
	$DECB dir multidsk,:$i
done

echo list contents:
for i in 0 1 2
do
	$DECB list multidsk,file$i:$i
done

cd ..
rm -r $TDIR
