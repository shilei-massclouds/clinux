#!/bin/sh

if [ -z "$1" ]
then
    printf "Target component needed!\n";
    exit 1;
fi

make DEP=info 2>&1 | grep " -> $1"
