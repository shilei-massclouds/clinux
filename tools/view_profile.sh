#!/bin/sh

if [ $# != 1 ]; then
    echo "missing [profile name]";
    exit -1;
fi
cat $1 | jq .
