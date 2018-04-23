#!/bin/bash
if
  gcc -Wall -Wextra sine.c -lm -o sine; 
then 
  ./sine;
else
  echo "fail";
  exit 1;
fi
