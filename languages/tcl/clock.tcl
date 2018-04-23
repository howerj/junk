#!/usr/bin/env wish
proc every {ms body} {eval $body; after $ms [info level 0]}         ;# (1)
pack [label .clock -textvar time]                                   ;# (2)
every 1000 {set ::time [clock format [clock sec] -format %H:%M:%S]} ;# (3)
