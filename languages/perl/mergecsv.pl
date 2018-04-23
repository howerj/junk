#!/usr/bin/perl

use warnings;
use strict;

my @filehandles;
my $outputfile      = "merged";
my $delimiter       = ",";
my $includelinenum  = 1;

open OUTPUT, ">", $outputfile or die $?;

## open all arguments as filenames
if($includelinenum){ print OUTPUT "0$delimiter" }
for (my $argnum=0; $argnum < $#ARGV + 1; $argnum = $argnum + 1){
  local *FILE;
  open FILE, "<", "$ARGV[$argnum]" || die "file";
  push @filehandles, *FILE;

  print OUTPUT "$ARGV[$argnum]$delimiter";
}
print OUTPUT "\n";

## process each file, merging line by line, we assume that
# all files are of equal length in terms of lines.
my $handprimary = $filehandles[0];
my $linecount = 1;
while(my $line = <$handprimary>){
  $line = substr($line, 0, -1);
  if($includelinenum){ print OUTPUT "$linecount$delimiter" }
  $linecount++;
  print OUTPUT "$line$delimiter";
  for(my $handlenum = 1; $handlenum < $#filehandles + 1; $handlenum++){
    my $currenthandle = $filehandles[$handlenum];
    my $currentline = <$currenthandle>;
    $currentline = substr($currentline, 0, -1);
    print OUTPUT "$currentline$delimiter";
  }
  print OUTPUT "\n";
}

## does this work?
foreach my $i(@filehandles){
  close pop @filehandles;
}
