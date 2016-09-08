package require Tk

namespace eval rr {
    variable data
    set data(curx) 700
    set data(y) 190
    proc init w {
        variable data
        set data(c) $w
        set data(trainspeed) 10
        set data(autospeed)  1
        set data(timespeed) 40 
        if ![winfo exists $w] {
            scrollbar ${w}sc -ori hori -command [list $w xview]
            canvas $w -width 2000 -height 700 -bg lightblue \
            -xscrollcommand [list ${w}sc set]
            grid $w -sticky news
            grid ${w}sc -sticky news
            grid rowconf . 0 -weight 1
            grid columnconf . 0 -weight 1
        }
        $w delete all
        foreach i [after info] {after cancel $i}
        bind .c <Shift-1> [list source [info script]]
        bind .c <1> {incr [namespace current]::data(trainspeed)  1}
        bind .c <2> {set  [namespace current]::data(trainspeed)  0}
        bind .c <3> {incr [namespace current]::data(trainspeed) -1}
        bind .c <Motion> {wm title . [.c canvasx %x],[expr [.c canvasy %y]-190]}
        for {set i 0} {$i<200} {incr i} {
            set x0 [expr -10000+rand()*18000]
            set y0 [expr -50+rand()*50]
            set x1 [expr $x0+100+rand()*260]
            set y1 [expr $y0+15+rand()*25]
            $w create oval $x0 $y0 $x1 $y1 -fill white -outline white \
            -tag cloud
        }
        $w create text 0 5 -text "TclTrains - Richard Suchenwirth 2001"\
            -anchor nw -fill LightSteelBlue2
        $w create poly 1000 1000 1000 390 2000 360 2300 350 2500 300 2500 1000 -fill green4
        waterfall $w 1800 370 12 130
        bridge $w 820 175 1700
        $w create poly 7500 200 8100 50 8200 70 8333 60 8666 100 9000 200\
             -fill gray70
        birds $w 30
        $w create poly -10000 2000 -10000 147 -8500 82 -8100 41 -8000 66 -7920 33\
            -7800 77 -7700 30 -7000 160 42 67 99 130 155 73 199 102 255 83 312 126\
            380 116 433 105 501 75 600 104 1700 450 2100 620 2400 390 2500 180 \
            10000 170 10000 2000  -fill green3 -tag ground
        tower $w -7750 80
        $w create rect -7660 200 -7800 350 -fill tan -outline tan ;# signal base
        $w create poly -8100 1000 -7800 80 -7500 1000 -fill tan -outline tan
        $w create rect -10000 191 10000 200 -fill tan -outline tan;# ballast
        for {set y 1000} {$y>80} {set y [expr {$y*0.96}]} {
            set dx [expr {($y-70)/5}]
            $w create rect [expr {-7800-$dx}] $y [expr {-7800+$dx}] [expr $y*0.982] \
                -fill brown -outline {}
        }
        $w create poly -7920 1000 -7800 80 -7940 1000 -fill grey
        $w create poly -7660 1000 -7800 80 -7680 1000 -fill grey
        signal $w -7680 300
        $w create poly -1100 230 -900 10 -700 20 -500 -20 -300 10 -100 220 -110 270\
             -300 400 -500 278  -fill grey60 -tag {ground fg}
        station 3800 182 200 Springfield
        $w create poly 3200 1000 3600 220 4200 220 4600 1000 \
        -fill yellow4
        $w create rect 3500 185 9000 190 -fill tan ;# ballast
        $w create line 3500 184 9000 184 -width 3 -fill grey50 ;# siding
        $w create poly -500 1000 100 130 700 1000 -fill gray50 ;# road
        $w create poly 80 1000 100 130 120 1000 -outline yellow -fill gray50
        $w create line -10000 190 10000 190 -fill gray75 -width 3 ;# rail
        crossing 210 215
        for {set i 0} {$i<250} {incr i} {
            lappend trees [list [expr round(-9900+rand()*19800)]\
                 [expr round(50+rand()*350)]]
        }
        foreach tree [lsort -integer -index 1 $trees] {
            eval tree $tree
        }
     auto $w 135 800
        $w raise fg
        $w config -scrollregion [$w bbox all]
    }

    proc speed {speed} {
        variable data
        return [expr {round($data(timespeed) / $speed)}]
    }

    proc auto {c x y} {
        $c create rect [expr $x+5] $y [expr $x+40] [expr $y+30]\
          -fill black -tag auto
        $c create rect [expr $x+255] $y [expr $x+295] [expr $y+30]\
          -fill black -tag auto
        $c create poly $x $y $x [expr $y-105] [expr $x+15] [expr $y-200]\
             [expr $x+280] [expr $y-200] [expr $x+300] [expr $y-105]\
             [expr $x+300] $y \
          -fill [random:select {blue green yellow grey pink orange}]\
          -tag auto
        $c create poly [expr $x+15] [expr $y-100] [expr $x+30] [expr $y-180]\
             [expr $x+270] [expr $y-180] [expr $x+285] [expr $y-100]\
          -fill white -outline black -tag auto
        $c create oval [expr $x+5] [expr $y-25] [expr $x+35] [expr $y-70]\
          -fill red -tag auto
        $c create oval [expr $x+265] [expr $y-25] [expr $x+295] [expr $y-70]\
          -fill red -tag auto
        auto'animate $c $x $y auto
    }

    proc auto'animate {c x y tag} {
        foreach {x0 y0 x1 y1} [$c bbox $tag] break
        if {$y0<130} {
             $c delete $tag
             after [expr {int(100+rand()*50)}] [namespace current]::auto $c $x $y
        } else {
             variable data
             if {!$data(blink) || ($y1<190) || ($y0>230)} {
              $c scale $tag 100 120 0.95 0.95
             }
             after [speed $data(autospeed)] [list after idle [list [namespace current]::auto'animate $c $x $y $tag]]
        }
    }

    proc tower {c x y} {
        $c create rect $x $y [expr $x+60] [expr $y+90] -fill bisque
        $c create rect [expr $x-3] [expr $y-3] [expr $x+62] $y -fill gray
        window $c  [expr $x+5] [expr $y+5] 15 15 2
        door $c [expr $x+5] [expr $y+44] 17 45 15
    }

    proc waterfall {c x y w h} {
        $c create rect $x $y [expr $x+$w] [expr $y+$h] -fill lightblue \
            -outline lightblue
        waterfall'animate $c $x $y $w $h
    }

    proc waterfall'animate {c x y w h} {
        set wx [expr {round($x+rand()*$w)}]
        $c create line $wx $y $wx [expr {$y+12}] -fill white \
            -tag waterfall
        $c move waterfall 0 5
        catch {$c lower waterfall ground} ;# might initially fail
        foreach i [$c find withtag waterfall] {
            if {[lindex [$c bbox $i] 1]>$y+$h} {$c delete $i}
        }
        after [speed 1] [list after idle [list [namespace current]::waterfall'animate $c $x $y $w $h]]
    }

    proc bridge {c x y w} {
        $c create rect $x $y [expr $x+$w] [expr $y+40] -fill grey40 -tag fg
        for {set x0 [expr $x+300]} {$x0<$x+$w} {set x0 [expr $x0+250]} {
            set x1 [expr $x0+70]
            $c create line $x0 [expr $y+40] $x0 1000 -width 2
            $c create line $x1 [expr $y+40] $x1 1000 -width 2
            for {set y0 [expr $y+40]} {$y0<1000} {set y0 $y1} {
                set y1 [expr $y0+50]
                $c create line $x0 $y0 $x1 $y0 -width 2
                $c create line $x0 $y0 $x1 $y1
                $c create line $x0 $y1 $x1 $y0
            }
        }
    }

    proc signal {c x y} {
        variable data
        set tag {}; if {$y>$data(y)} {set tag fg}
        $c create rect $x $y [expr $x+5] [expr $y*0.5] -fill grey25\
            -tag $tag
        $c create rect [expr $x-15] [expr $y*0.62] \
            [expr $x+20] [expr $y*0.38] -fill black\
            -tag $tag
        $c create oval [expr $x-5] [expr $y*0.55-5]\
            [expr $x+10] [expr $y*0.55+10] -fill red -tag $tag
        $c create oval [expr $x-5] [expr $y*0.45-5]\
            [expr $x+10] [expr $y*0.45+10] -fill red -tag $tag
    }

    proc birds {w n} {
        for {set i 0} {$i<$n} {incr i} {
            set x0 [expr -10000+rand()*18000]
            set y0 [expr rand()*80]
            set coords [list $x0 $y0 [expr $x0+4] [expr $y0-1] \
                [expr $x0+6] [expr $y0+1] [expr $x0+8] [expr $y0-1]\
                [expr $x0+12] $y0]
            eval $w create line $coords -tag bird
        }
        birds'animate $w -1
    }

    proc birds'animate {c mode} {
        foreach i [$c find withtag bird] {
            $c move $i [expr {round(-2.5+rand()*4)}] [expr {round(rand()*2-1)}]
            set coords  [$c coords $i]
            foreach {x0 y0 x1 y1 x2 y2 x3 y3 x4 y4} $coords break
            set y0 [expr {$y0-2*$mode}]
            set y2 [expr {$y2+1*$mode}]
            set y4 [expr {$y4-2*$mode}]
            $c coords $i $x0 $y0 $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4
        }
        after 300 [list after idle [list [namespace current]::birds'animate $c [expr {$mode*-1}]]]
    }

    proc define {name def} {
        variable data
        set data($name) $def
    }

    proc create {type id args} {
        variable data
        set c $data(c)
        set tag $type:$id
        foreach i [split $data($type) \n] {
            set cmd [lindex $i 0]
            switch $cmd {
            bogie {
                set x [lindex $i 1]
                set diameter 21
                $c create oval $x -$diameter [expr $x+$diameter] 0\
                    -fill black -outline white -tag $tag
                set x1 [expr $x+[lindex $i 2]]
                $c create oval $x1 -$diameter [expr $x1+$diameter] 0\
                    -fill black -outline white -tag $tag
                set m [expr {$x+($x1+$diameter-$x)/2}]
                $c create rect [expr $m-10] -10 [expr $m+10] -30 -fill black -tag $tag
                $c create rect [expr $x-5] [expr -$diameter/2-5]\
                     [expr $x1+$diameter+5] [expr -$diameter/2+5] -fill gray20 -tag $tag
            }
            gp38body {
                set t [list gp38body $tag]
                $c create rect 0 -28 430 -21 -fill black -tag $t
                $c create rect 25 -28 70 -80 -fill red -tag $t    ;#nose
                $c create rect 70 -100 130 -28 -fill red -tag $t  ;#cab
                $c create rect 130 -100 410 -28 -fill red2 -tag $t ;#body
                $c create poly 180 -102 264 -102 264 -102 244 -90 200 -90\
                    180 -102 -fill red -outline black -tag $t
                $c create rect 54 -103 170 -93 -fill red -tag $t  ;#cab roof
                $c create line 290 -105 300 -105 -fill red -arrow both \
                    -arrowshape {-6 -5 3} -width 2 -tag $t ;# horns
                $c create rect 190 -22 255 -6 -fill black -tag $t ;#tank
                window $t 86 -90 26 18
                $c create line 100 -90 100 -72 -tag $tag ;# window separator
                $c create line 15 -8 15 -55 -fill white -width 2 -tag $tag
                $c create line 35 -8 35 -60 45 -60 57 -70 72 -70 -tag $t
                $c create line 128 -70 130 -70 140 -60 315 -60 320 -55 \
                    405 -55 405 -8 -tag "$t handrail"
                for {set x 150} {$x<400} {incr x 45} {
                    $c create line $x -60 $x -20 -tag "$t handrail"
                }
                $c create line 425 -8 425 -55 -fill white -width 2 -tag $tag
                for {set y -70} {$y<=-30} {incr y 10} {
                    $c create line 25 $y 30 [expr $y-8] -width 3 -fill white\
                        -tag $t
                }
                $c create text 100 -60 -text $id\
                     -font {Helvetica 11 {bold italic}} -fill white -tag $t
                $c create text 225 -75 -text "CP Rail" \
                     -font {Helvetica 24 {bold italic}} -fill white -tag $t
                $c create oval 350 -90 400 -40 -fill white -outline white -tag $t
                $c create rect 375 -90 400 -40 -fill white -outline white -tag $t
                $c create poly 375 -65 400 -40 400 -90 -fill black -tag $t
                $c raise handrail
            }
            f7abody {
                set t [list f7abody $tag]
                $c create rect 0 -25 430 -22 -fill black -tag $tag
                $c create rect 400 -30 430 -90 -fill black -tag $tag
                $c create poly \
                17 -9 30 -85 35 -88 58 -90 60 -92 67 -106 70 -108 73 -110 \
                    425 -110 425 -15 410 -15 400 -25 295 -25 290 -15 165 -15 \
                    160 -25 45 -25 35 -9 -fill gold -tag $t
                $c create rect 30 -71 51 -59 -fill black -tag $t
                $c create text 31 -71 -text $id -anchor nw -fill white -tag $t
                $c create poly 67 -102 72 -101 76 -97 70 -87 62 -92 \
                    -fill white -outline black -tag $t
                $c create poly 71 -81 80 -94 94 -94 94 -81 -fill white \
                    -outline black -tag $t
                $c create rect 98 -97 114 -52 -outline gold3 -tag $t
                $c create rect 101 -94 111 -81 -fill white -tag $t ;# cab door window
                $c create rect 118 -97 420 -80 -outline gold3 \
                     -tag $t ;# cooler grill
                for {set i 121} {$i<420} {incr i 3} {
                    $c create line $i -97 $i -80 -fill gold3 -tag $t
                }
                $c create rect 140 -110 424 -100 -fill gray75 \
                    -outline gray75 -tag $t;# roof
                $c create line 100 -113 110 -113 -arrow both \
                    -arrowshape {-5 -5 3} -width 2 -tag $t ;# horns
                $c create rect 103 -115 107 -110 -fill black -tag $t
                $c create oval 150 -77 165 -62 -fill gray50 -tag $t
                $c create oval 300 -77 315 -62 -fill gray50 -tag $t
                $c create text 145 -56 -text "U N I O N    P A C I F I C" -fill red \
                -font {Helvetica 13 bold} -anchor nw -tag $t
                $c create text 55 -56 -text $id -fill red -font {Helvetica 13 bold}\
                     -anchor nw -tag $t
                $c create line 55 -37 423 -37 -fill red -width 3 -tag $t
            }
            f7bbody {
                set t [list f7bbody $tag]
                $c create rect 0 -25 430 -22 -fill black -tag $tag
                $c create rect 0 -30 430 -90 -fill black -tag $tag
                $c create poly 17 -110 \
                    425 -110 425 -15 410 -15 400 -25 295 -25 290 -15 165 -15 \
                    160 -25 45 -25 35 -15 17 -15 -fill gold -tag $t
                $c create rect 22 -97 420 -80 -outline gold3 \
                     -tag $t ;# cooler grill
                for {set i 25} {$i<420} {incr i 3} {
                    $c create line $i -97 $i -80 -fill gold3 -tag $t
                }
                $c create rect 18 -110 424 -100 -fill gray75 \
                    -outline gray75 -tag $t;# roof
                $c create oval 140 -77 155 -62 -fill gray50 -tag $t
                $c create oval 220 -77 235 -62 -fill gray50 -tag $t
                $c create oval 300 -77 315 -62 -fill gray50 -tag $t
                $c create text 145 -56 -text "U N I O N    P A C I F I C" -fill red \
                -font {Helvetica 13 bold} -anchor nw -tag $t
                $c create text 55 -56 -text $id -fill red -font {Helvetica 13 bold}\
                     -anchor nw -tag $t
                $c create line 25 -37 423 -37 -fill red -width 3 -tag $t
            }
            boxcarbody {
                $c create rect 0 -25 380 -22 -fill black -tag $tag
                $c create rect 10 -26 370 -105 -fill [lindex $args 1] -tag $tag
                set rgrey grey[expr round(rand()*40+50)]
                $c create rect 10 -100 370 -105 -fill $rgrey -tag $tag
                $c create rect 160 -95 220 -30 -tag $tag
                $c create text 100 -70 -text [lindex $args 0] -fill white -tag $tag
                $c create text 100 -50 -text $id -fill white -tag $tag
            }
            baggagebody {
                $c create rect 0 -25 420 -22 -fill black -tag $tag
                $c create rect 0 -30 420 -92 -fill black -tag $tag
                $c create rect 5 -24 415 -100 -fill [lindex $args 1] -tag $tag
                set rgrey grey[expr round(rand()*20+70)]
                $c create rect 5 -90 415 -100 -fill $rgrey -tag $tag
                $c create line 8 -48 413 -48 -width 5 -fill red -tag $tag
                $c create line 8 -43 413 -43 -width 5 -fill white -tag $tag
                $c create line 8 -38 413 -38 -width 5 -fill blue -tag $tag
                $c create text 205 -43 -text [lindex $args 0]\
                     -fill black -font {Helvetica 8 bold} -tag $tag
                $c create text 71 -43 -text $id  -tag $tag
                door $tag 20 -75 20 50 15
                $c create rect 120 -80 160 -30 -tag $tag
                window $tag 125 -72 12 12 2 4
                $c create rect 260 -80 300 -30 -tag $tag
                window $tag 265 -70 12 12 2 4
            }
            coachbody {
                $c create rect 0 -25 420 -22 -fill black -tag $tag
                $c create rect 0 -30 420 -90 -fill black -tag $tag
                $c create rect 5 -23 415 -100 -fill [lindex $args 1] -tag $tag
                set rgrey grey[expr round(rand()*20+70)]
                $c create rect 5 -90 415 -100 -fill $rgrey -tag $tag
                for {set y -50} {$y<=-30} {incr y 3} {
                    $c create line 11 $y 410 $y -fill white -tag $tag
                }
                $c create text 205 -83 -text [join [split [lindex $args 0] ""] "       "]\
                     -fill red -font {Times 7 bold} -tag $tag
                $c create text 71 -44 -text $id  -tag $tag
                door $tag 10 -75 20 50 15
                window $tag 48 -72 36 16 7 8
                door $tag 390 -75 20 50 16
            }
            ocoachbody {
                $c create rect 0 -25 420 -22 -fill black -tag $tag
                $c create rect 20 -23 400 -90 -fill [lindex $args 1] -tag $tag
                set rgrey grey[expr round(rand()*20+40)]
                $c create rect 5 -90 415 -95 -fill $rgrey -tag $tag
                $c create poly 20 -95 40 -102 380 -102 400 -95 -fill grey\
                    -outline black -width 2 -tag $tag
                for {set x 50} {$x<=370} {incr x 20} {
                    $c create line $x -95 $x -102 -width 2 -tag $tag
                }
                $c create line 5 -5 5 -90 -tag $tag
                $c create line 415 -5 415 -90 -tag $tag
                $c create line 20 -88 400 -88 -fill yellow -tag $tag
                $c create text 205 -83 -text [join [split [lindex $args 0] ""] "       "]\
                     -fill yellow -font {Times 7 bold} -tag $tag
                $c create line 20 -78 400 -78 -fill yellow -tag $tag
                $c create text 40 -40 -text $id -fill yellow -tag $tag
                $c create text 380 -40 -text $id -fill yellow -tag $tag
                window $tag 50 -72 15 22 16 5
                $c create line 50 -62 350 -62 -tag $tag
                $c create line 20 -27 400 -27 -fill yellow -tag $tag
            }
            domebody {
                $c create rect 0 -25 420 -22 -fill black -tag $tag
                $c create rect 0 -30 420 -90 -fill black -tag $tag
                $c create rect 5 -24 415 -100 -fill [lindex $args 1] -tag $tag
                set rgrey grey[expr round(rand()*20+70)]
                $c create rect 5 -90 415 -100 -fill $rgrey -tag $tag
                $c create poly 110 -92 100 -100 130 -120 300 -120 330 -100 320 -92 \
                    -fill lightcyan -outline black -tag $tag ;# dome
                for {set x 130} {$x<=300} {incr x 34} {
                    $c create line $x -120 $x -90 -tag $tag
                }
                $c create line 123 -115 130 -110 300 -110 308 -118 -tag $tag
                $c create text 210 -83 -text [join [split [lindex $args 0] ""] "       "]\
                     -fill red -font {Times 7 bold} -tag $tag
                for {set y -50} {$y<=-30} {incr y 3} {
                    $c create line 11 $y 410 $y -fill white -tag $tag
                }
                $c create text 71 -44 -text $id  -tag $tag
                door $tag 20 -75 20 50 15
                window $tag 110 -60 25 15 6 10
                door $tag 380 -75 20 50 16
            }
            caboosebody {
                $c create rect 0 -24 300 -21 -fill black -tag $tag
                $c create poly 35 -23 35 -100 120 -100 120 -130 190 -130\
                     190 -100 270 -100 270 -23\
                     -fill [lindex $args 1] -tag $tag
                $c create line 10 -10 10 -116 11 -117 12 -119 15 -118 22 -105 -tag $tag
                $c create line 290 -10 290 -116 289 -117 288 -119 284 -118 277 -105 -tag $tag
                set rgrey grey[expr round(rand()*40+10)]
                $c create rect 10 -100 120 -105 -fill $rgrey -tag $tag
                $c create rect 115 -125 195 -130 -fill $rgrey -tag $tag
                $c create rect 190 -100 290 -105 -fill $rgrey -tag $tag
                $c create rect 210 -105 215 -130 -fill black -tag $tag
                window $tag 130 -120 18 15 2 15
                window $tag  50 -75 19 17 2 15
                window $tag 200 -75 19 17 2 15
                $c create text 150 -90 -text [lindex $args 0] -fill white -tag $tag
                $c create text 150 -50 -text $id -fill white -tag $tag
                $c create arc 40 -30 85 -85 -style arc -start 180 \
                    -extent 90 -outline yellow -width 1 -tag $tag
                $c create arc 220 -30 265 -85 -style arc -start 270 \
                    -extent 90 -outline yellow -width 1 -tag $tag
            }
            flatcarbody {
                $c create rect 0 -25 380 -22 -fill black -tag $tag
                $c create rect 10 -26 370 -35 -fill [lindex $args 1] -tag $tag
                $c create text 80 -29 -text [lindex $args 0] -fill white -tag $tag
                $c create text 220 -29 -text $id -fill white -tag $tag
            }
            gondolabody {
                $c create rect 0 -25 380 -22 -fill black -tag $tag
                $c create rect 10 -26 370 -90 -fill [lindex $args 1] -tag $tag
                $c create text 100 -70 -text [lindex $args 0] -fill white -tag $tag
                $c create text 100 -50 -text $id -fill white -tag $tag
            }
            hopperbody {
                $c create rect 0 -25 380 -22 -fill black -tag $tag
                $c create poly 10 -100 10 -50 120 -10 130 -30 180 -10 190 -30\
                    200 -10 250 -30 260 -10 370 -50 370 -100 -fill [lindex $args 1] -tag $tag
                $c create rect 10 -26 370 -100 -width 2 -tag $tag
                for {set x 52} {$x<360} {incr x 93} {
                    $c create line $x -26 $x -100 -width 2 -tag $tag
                }
                $c create text 90 -50 -text "[lindex $args 0] $id" -fill white -tag $tag
                $c create text 190 -70 -text [lindex $args 0] -font {Times 24 bold}\
                    -fill white -tag $tag
           }
            trailer {
                set color [lindex $i 1]
                $c create rect 40 -110 340 -50 -fill $color -tag $tag
                $c create text 190 -80 -text "ROADWAY" \
                    -font {Helvetica 40} -fill green4 -tag $tag
                $c create line 80 -50 80 -35 -width 3 -tag $tag
                $c create oval 240 -50 260 -30 -fill gray50 -tag $tag
                $c create oval 280 -50 300 -30 -fill gray50 -tag $tag
                $c create oval 245 -45 255 -35 -fill $color -tag $tag
                $c create oval 285 -45 295 -35 -fill $color -tag $tag
            }
            "" continue
            default {error "bad definition word $cmd:\n$i"}
            }
        }
    }

    proc train {name rstock} {
        variable data
        set c $data(c)
        set newx 0
        foreach i $rstock {
                $c move $i $data(curx) $data(y)
                set data(curx) [lindex [$c bbox $i] 2]
                $c addtag $name withtag $i
        }
        incr data(curx) 10000
        $c raise $name
    }

    proc crossing {x y} {
        variable data
        set c $data(c)
        $c create line [expr $x-10] [expr $y-40] [expr $x+15] [expr $y-40]\
            -width 3 -tag fg
        $c create rect $x $y [expr $x+5] [expr $y-70] -fill orange -tag fg
        $c create line [expr $x-15] [expr $y-80] [expr $x+20] [expr $y-60]\
             -width 5 -fill white -tag fg
        $c create line [expr $x-15] [expr $y-60] [expr $x+20] [expr $y-80]\
             -width 5 -fill white -tag fg
        $c create oval [expr $x-8] [expr $y-45] [expr $x-18] [expr $y-35]\
            -fill white -tag fg
        $c create oval [expr $x-10] [expr $y-43] [expr $x-16] [expr $y-37]\
            -fill black -tag {fg blink0}
        $c create oval [expr $x+15] [expr $y-45] [expr $x+25] [expr $y-35]\
            -fill white -tag fg
        $c create oval [expr $x+17] [expr $y-43] [expr $x+23] [expr $y-37]\
            -fill black -tag {fg blink1}
        set data(blink) 1
        flashCrossing 0
    }

    proc flashCrossing {which} {
        variable data
        set c $data(c)
        if $data(blink) {$c itemconfig blink$which -fill red -outline red}
        set which [expr 1-$which]
        $c itemconfig blink$which -fill black -outline black
        after 250 [list after idle [list [namespace current]::flashCrossing $which]]
    }

    proc window {t x y w h {n 1} {space 10}} {
        variable data
        set c $data(c)
        for {set i 0} {$i<$n} {incr i} {
            $c create rect $x $y [expr $x+$w] [expr $y+$h] -fill black -tag $t
            $c create rect [expr $x+3] [expr $y+3] [expr $x+$w] [expr $y+$h]\
                 -fill white -tag $t
            set x [expr $x+$w+$space]
        }
    }

    proc door {t x y w h winh} {
        variable data
        set c $data(c)
        $c create rect $x $y [expr $x+$w] [expr $y+$h] -tag $t
        incr w -8
        incr x 4
        incr y 4
        window $t $x $y $w $winh

    }

    proc run {trains} {
        variable data
        set c $data(c)
        foreach i [$c find withtag cloud] {
            $c move $i [expr {rand()/10}] 0
        }
        foreach train $trains {
            $c move $train -5 0
        }
        after idle [list [namespace current]::run2 $trains]
    }

    proc run2 {trains} {
        variable data; set c $data(c)
        foreach train $trains {
            foreach {x0 y0 x1 y1} [$c bbox $train] break
            if {$x1<-10000} {
                $c move $train 25000 0
            } elseif {$x1<0 && $x1>-1000} {
                set data(blink) 0
            } elseif {$x0<1500 && $x1>-1000} {
                set data(blink) 1
            }
        }
        after [speed $data(trainspeed)] [list after idle [list [namespace current]::run $trains]]
        $c raise fg
    }

    proc tree {x y} {
        set color [random:select {DarkGreen ForestGreen}]
        set stemcolor [random:select {brown salmon4 chocolate4 burlywood4}]
        variable data; set c $data(c)
            set where [lindex [$c find overlapping $x $y $x $y] end]
        if {[lsearch [$c itemcget $where -tags] "ground"]>=0} {
            set tag {}
            if {$y>$data(y)} {set tag fg}
            set dx [expr 1+($y*$y/12000.)]
            set x0 [expr $x-$dx*5]
            set x1 [expr $x+$dx+$dx*5]
            set xm [expr $x+$dx/2.]
            set y1 [expr $y-10-$dx*20*rand()]
            set y2 [expr $y1-6*$dx*(2+2*rand())]
            $c create rect $x $y [expr $x+$dx] $y1 -fill $stemcolor -tag $tag
            $c create poly $x0 $y1 $xm $y2 $x1 $y1 -fill $color \
                -tag $tag -outline black
        }
    }

    proc station {x y w name} {
        variable data; set c $data(c)
        $c create rect $x $y [expr $x+$w] [expr $y-100] -fill PeachPuff1
        window $c [expr $x+20] [expr $y-95] 18 22 4 30
        $c create rect [expr $x-5] [expr $y-100] [expr $x+$w+5] [expr $y-160]\
            -fill gray40
        door $c [expr $x+20] [expr $y-58] 20 50 22
        window $c [expr $x+65] [expr $y-55] 18 22 3 30
        set it [$c create text [expr $x+$w/2.] [expr $y-68] \
            -text [string toupper "   $name   "]]
        eval $c create rect [$c bbox $it] -fill white
        $c raise $it
        $c create rect [expr $x-50] [expr $y-60] \
            [expr $x+$w+100] [expr $y-55] -fill PeachPuff3
        for {set i [expr $x-45]} {$i<$x+$w+100} {incr i 60} {
            $c create rect $i [expr $y-55] \
                [expr $i+3] $y -fill PeachPuff3
        }
        $c create rect [expr $x-200] $y [expr $x+$w+500] [expr $y-10]\
            -fill gray30
    }

    proc flip what {
        variable data; set c $data(c)
        foreach {x0 y0 x1 y1} [$c bbox $what] break
        $c scale $what [expr ($x1-$x0)/2.] [expr ($y0-$y0)/2.] -1 1
    }

    define F7A {
        bogie 55 60
        bogie 305 60
        f7abody
    }
    define F7B {
        bogie 55 60
        bogie 305 60
        f7bbody
    }
    define GP38 {
        bogie 60 50
        bogie 310 50
        gp38body
    }
    define boxcar {
        bogie 40 40
        bogie 280 40
        boxcarbody
    }
    define coach {
        bogie 50 40
        bogie 310 40
        coachbody
    }
    define ocoach {
        bogie 50 40
        bogie 310 40
        ocoachbody
    }
    define domecar {
        bogie 50 40
        bogie 310 40
        domebody
    }
    define baggage {
        bogie 50 40
        bogie 310 40
        baggagebody
    }
    define gondola {
        bogie 40 40
        bogie 280 40
        gondolabody
    }
    define hopper {
        bogie 30 40
        bogie 290 40
        hopperbody
    }
    define flatcar {
        bogie 40 40
        bogie 280 40
        trailer gray85
        flatcarbody
    }
    define caboose {
        bogie 40 40
        bogie 190 40
        caboosebody
    }
}

proc random:select {L} {
    lindex $L [expr {int(rand()*[llength $L])}]
}

namespace eval rr {
    # Usage examples, and demo:
    init .c
    create F7A I50I
    create F7B I308
    create GP38 3018
    create GP38 3022
    flip GP38:3022
    create hopper 12988 "N & W" grey30
    create hopper 5603 "UNION PACIFIC" brown
    create boxcar 42135 ATSF brown
    create boxcar 42199 C&NW orange
    create gondola 745219 N.Y.C. salmon4
    create baggage 93152 "Amtrak" grey80
    create coach 4312 "UNION PACIFIC" grey90
    create domecar 7001 "UNION PACIFIC" grey95
    create coach 4319 "UNION PACIFIC" grey90
    create ocoach 4711 "BALTIMORE & OHIO" darkgreen
    create ocoach 5006 "PENNSYLVANIA" firebrick
    create caboose 18832 "C A N A D I A N   P A C I F I C" red2
    create flatcar 88402 "BOSTON & MAINE" black

    for {set i 43127;set parked {}} {$i<43256} {incr i 13} {
        create boxcar $i N.Y.C. \
            [random:select {brown salmon4 firebrick burlywood4}]
        lappend parked boxcar:$i
    }
    train parked $parked
    .c move parked 3700 -5
    train UP1 {
         F7A:I50I F7B:I308 baggage:93152 coach:4319 domecar:7001 coach:4312
         ocoach:4711 ocoach:5006
    }
    train CP123 {
        GP38:3018 GP38:3022 hopper:12988 hopper:5603
        boxcar:42135 flatcar:88402 gondola:745219 boxcar:42199
        caboose:18832
    }
    run {CP123 UP1}
    .c move UP1 -14000 0 ;# quick start
    .c move CP123 -14000 0 ;# quick start
}
