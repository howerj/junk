#!/usr/bin/wish
# from http://wiki.tcl.tk/41294
foreach row {0 1 2 3 4 5 6} {
    foreach {column columnName} {0 "" 1 A 2 B 3 C 4 D 5 E 6 F} {
        set widget [if {$column == 0 || $row == 0} {
            ::ttk::label .label$columnName$row -text [expr { $row == 0 ? $columnName : $row }]
        } else {
            lassign {"" ""} ::formula($columnName$row) ::$columnName$row
            trace add variable ::$columnName$row read recalc
            ::ttk::entry .cell$columnName$row -textvar ::$columnName$row -width 10 -validate focus \
                    -validatecommand [list ::reveal-formula $columnName$row %V %s]
        }]
        grid $widget -row $row -column $column
    }
}
proc set-cell {cell value} {
    .cell$cell delete 0 end
    .cell$cell insert 0 $value
}
proc recalc {cell args} {
    if {$::formula($cell) ne ""} {
        catch {set-cell $cell [uplevel #0 [list \
                expr [regsub -all {([A-F][1-6])} $::formula($cell) {$\1}]]]}
    }
}
proc reveal-formula {cell event value} {
    if {$event eq "focusin"} {
        if {$::formula($cell) ne ""} { set-cell $cell =$::formula($cell) }
    } else { ;# focusout
        if {[string match "=*" $value]} { set ::formula($cell) [string range $value 1 end] }
        foreach otherCell [array names ::formula] { recalc $otherCell }
    }
    return 1
}
