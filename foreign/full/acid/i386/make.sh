#!/bin/sh

if [ ! "`which nasm`" ]
then
    echo "Could not find nasm in PATH."
    exit 0
fi

nasm -f bin -o acid acid.asm
chmod 755 acid
