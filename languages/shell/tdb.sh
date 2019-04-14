#!/bin/bash
# Tiny Database: https://news.ycombinator.com/item?id=19581721
db_set () {
 echo "$1,$2" >> database
}
db_get () {
 # read database line by line in reverse, stopping at first match
 tac database | grep -m 1 "^$1," | sed -e "s/^$1,//"
}
