#!/bin/sh

TARGETS="
    linux-5.8.1
    linux-5.9.1
    linux-5.11.1
    linux-5.13.1
    linux-5.15.1
    linux-5.17.1
    linux-5.19.1
    linux-6.1.1
"

for TARGET in $TARGETS
do
    printf "\n[$TARGET]: ...\n"
    target/debug/deptool /home/cloud/study/$TARGET/ .o
done
