#!/bin/sh

BOOTRD_FILE="../../output/bootrd.disk"

rm -f $BOOTRD_FILE
dd if=/dev/zero of=$BOOTRD_FILE bs=1M count=32
./mk_bootrd
