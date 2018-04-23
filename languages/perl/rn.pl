#!/usr/bin/perl
# Rename files with their md5sum, preserving file extension
# and storing the newname and old name in a text file
# TODO:
#   * Undo function

use strict;
use warnings;
use Digest::file qw(digest_file_hex);
use File::Copy;

my $filelist = "keyval.txt";

open OUTPUT, ">>", $filelist;

for (@ARGV) {
  my $file = $_;
  my @extension = split(/(\.[^.]+)$/, $file);
  my $newfile = "error";

  if($#extension >= 1){
    $newfile = digest_file_hex($file, "MD5") . lc $extension[$#extension];
  } else {
    $newfile = digest_file_hex($file, "MD5");
  }
  print OUTPUT "$newfile $file\n";
  move($file, $newfile) or die("$file -> $newfile failed");
}

close OUTPUT;
