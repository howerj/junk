#!/usr/bin/perl -w
######################################################################################################################
#	Richard James Howe
#	Pretty print tables
#	TODO:
#		* auto escape regexes
#		* specify input file, output file, table beginning and ends, ... via command line arguments
#		* use seek instead of reopen/close file
#		* process only table enteries, leave the rest of the file alone
#		* Error on incorrect number of table enteries.
#		* Clean up code
#		* Commas should be handled correctly, ie.
#		    token,
#		  And
#		    token ,
#     Should be treated the same
#   * Read from stdin, save to temporary file, process
#
######################################################################################################################

use strict;

my $begin = "X("; 
my $end = ")\\";
my $extraspace = 2; 

my @maxtablestrlen;
my $numberofenteries;

my $temporary_file = "tmpfile.txt";

if (defined($ARGV[0])){
  my $filename = $ARGV[0];
  open (FH, "<", $filename) or die("Cannot open input file <$filename> for reading\n");
}else{
  open (FH, ">", $temporary_file) or die ("Cannot open temporary file for writing\n");
  while(<STDIN>){
    print FH $_;
  }
  close(FH);
  open (FH, "<", $temporary_file) or die ("Cannot open temporary file for reading.\n");
}
######################################################################################################################
### Get maximum table entry length ###################################################################################
######################################################################################################################


# Get maximum string length for each table entry
while(<FH>){
  my $line = $_;
	my @line = split(" ", $line);	# Split line of table into tokens
  next if not defined $line;
  next if not $line =~ /\Q$begin\E/;
  next if not $line =~ /\Q$end\E/;

	$line[0] =~ s/\Q$begin\E//;		# Remove and fluff at the beginning of the table
	$line[$#line] =~ s/\Q$end\E//;	# Remove end fluff

	$numberofenteries = $#line + 1; ## this should not increase once it has been defined, should check for that

	# For every token calculate its length and update a global list of line lengths for each column
	for(my $i = 0; $i < $numberofenteries; $i++){
		if (defined($maxtablestrlen[$i])){
			my $tokenlength = length $line[$i] ;
			my $currentmax = $maxtablestrlen[$i];
			$currentmax = ($currentmax, $tokenlength) [$currentmax < $tokenlength];
			$maxtablestrlen[$i] = $currentmax;
		} else{
			$maxtablestrlen[$i] = length $line[$i]; # Set up intial value
		}
	}
}

######################################################################################################################
### Pretty print table out to STDOUT #################################################################################
######################################################################################################################

seek FH, 0, 0;

while(<FH>){
  my $line = $_;
	my @line = split(" ", $line);	# Split line of table into tokens
  next if not defined $line;
  if (( not $line =~ /\Q$begin\E/ ) and ( not $line =~ /\Q$end\E/ )){
    print $line;
    next;
  }

	$line[0] =~ s/\Q$begin\E//;		  # Remove and fluff at the beginning of the table
	$line[$#line] =~ s/\Q$end\E//;	# Remove end fluff

	print $begin;

	$numberofenteries = $#line + 1; ## this should not increase once it has been defined, should check for that
	for(my $i = 0; $i < $numberofenteries; $i++){
		print $line[$i];
		for(my $j = 0; $j < ($maxtablestrlen[$i] - length($line[$i]) + $extraspace); $j++){
			print " ";
		}
	}
	print "$end\n";
}

close (FH);


######################################################################################################################
### END ##############################################################################################################
######################################################################################################################
