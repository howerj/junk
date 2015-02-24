#!/bin/sh

as -o acid.o acid.s
ld -o acid acid.o
if [ "`which sstrip`" ]
then
    sstrip acid
else
    strip acid
fi
