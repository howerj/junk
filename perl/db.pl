#!/usr/bin/perl
# Check MD5sums against database
# TODO:
#   * print
#   * help
#   * update
#   * error checking
#   * print out all hashes
use strict;
use warnings;
use File::Find; # recursive directory listing
use Digest::file qw(digest_file_hex); # file hash
use Cwd; # current directory

my $working_dir = getcwd;
my @files;
my $dbmfile = "md5";
my $start_dir = $ARGV[0];
my %MANIFEST;

defined $ARGV[0] or die "usage: ./db.pl <directory>\n";

dbmopen(%MANIFEST,$dbmfile,0600);

# make sure our database has the correct data in it.
if(defined($MANIFEST{"pwd"})){
  $MANIFEST{"pwd"} eq $working_dir or die "expected working directory to be $working_dir\n";
} else {
  $MANIFEST{"pwd"} = $working_dir;
}


# get file list (recursive)
find( 
     sub { push @files, $File::Find::name unless -d; }, 
     $start_dir
);

# for each file calculate MD5sum
for(@files){
  my $file = $_;
  my $digest = digest_file_hex($file,"MD5");
#  print "$working_dir/$file $digest\n";
  if(defined($MANIFEST{$file})){
    if(not ($MANIFEST{$file} eq $digest)){
      print "hash mismatch on $file\n";
      print "$MANIFEST{$file} : $digest\n";
    }
  } else {
    $MANIFEST{$file} = $digest;
  }
}
dbmclose(%MANIFEST);
