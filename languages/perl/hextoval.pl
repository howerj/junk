#!/usr/bin/perl -w
###############################################################################
# howe.r.j.89@gmail.com
# TODO:
# 	* TK application
# 	* Sanity check on each line
# 	* option -u1 does not work correctly.
# 	* If no command line options give, launch GUI
###############################################################################

use warnings;
use strict;

use Tk;

###############################################################################
### GLOBALS ###################################################################
###############################################################################

## command line option variable
my $invertorder     = 0;
my $numberofbytes   = 2;    # short as default
my $issigned        = 0; 
my $nibbles         = "H4"; # used by pack for signed and floating numbers
my $unpackstr       = "S";  # Should either be c,s or l if signed is selected.
my $isfloat         = 0;
my $verboselevel    = 0;
## internally calibratable variables
my $ignoreXcolumns  = 2;    # number of columns of data to ignore (if any)
my $defaultfile     = "mem.txt";

################################################################################
### TK SETUP ###################################################################
################################################################################

# Main Window #
my $mw = new MainWindow;

### Misc ###
$mw -> title("Perl/Tk Hex to CSV, RJH"); # Set title

# Maximize main window. #
$mw -> geometry($mw->screenwidth . "x" . $mw->screenheight . "+0+0"); 

################################################################################
### KEY BINDINGS ###############################################################
################################################################################

$mw -> bind('<KeyRelease-Escape>' => sub{ exit });
$mw -> bind('<KeyRelease-Return>' => \&tkprocessfiles);

################################################################################
### GUI BUILDING AREA ##########################################################
################################################################################

### Labels and data entry. ###
my $frm_name               = $mw -> Frame();
my $regexLabel             = $frm_name -> Label(-text=>"Regex:");
my $filenameLabel          = $frm_name -> Label(-text=>"File Name:");
my $regexEnt               = $frm_name -> Entry();
my $fileEnt                = $frm_name -> Entry(-textvariable=>"$defaultfile");

### Check button opens and their labels/result variables. ####

## Delete previous search results on new search? ##
my $frm_invertorder     = $frm_name; # $mw -> Frame();
my $lbl_invertorder     = $frm_invertorder -> Label(-text=>"Invert order:");
my $rdb_invOrd_yes      = $frm_invertorder -> Radiobutton(
                             -text=>"Yes",  
                             -value=>1,  -variable=>\$invertorder
                           );
my $rdb_invOrd_no          = $frm_invertorder -> Radiobutton(
                             -text=>"No",
                             -value=>0,-variable=>\$invertorder
                           );

## Is the search going to be case sensitive or not? ##

my $caseSensitive          = "No";
my $frm_caseSensitive      = $frm_name; # $mw -> Frame();
my $lbl_caseSensitive      = $frm_caseSensitive -> Label(-text=>"Case Sensitive Search");
my $rdb_caseSensitive_yes  = $frm_caseSensitive -> Radiobutton(
                             -text=>"Yes",  
                             -value=>"Yes",  -variable=>\$caseSensitive
                           );
my $rdb_caseSensitive_no   = $frm_caseSensitive -> Radiobutton(
                             -text=>"No",
                             -value=>"No",-variable=>\$caseSensitive
                           );

my $processButton           = $frm_name -> Button(-text=>"Process", -command =>\&tkprocessfiles);

### Text Area ###

my $textarea               = $mw -> Frame();
my $textareaFontUsed       = "systemfixed"; # Windows only, should change this.
my $textareaCharWidth      = $textarea -> fontMeasure($textareaFontUsed, "W");  # Use "W" as it is a wide(est?) character just in case we are not using a fixed width font
my $textareacontent        = $textarea -> Text(
                             -font   => $textareaFontUsed, 
                             -width  => (($mw->screenwidth)/$textareaCharWidth) - 10, 
                             -height => 40,
                             -wrap   => "none"
                           );
my $srl_y                  = $textarea -> Scrollbar(-orient=>'v',-command=>[yview => $textareacontent]);
my $srl_x                  = $textarea -> Scrollbar(-orient=>'h',-command=>[xview => $textareacontent]);
$textareacontent ->        configure(-yscrollcommand=>['set', $srl_y], -xscrollcommand=>['set',$srl_x]);

################################################################################
### GEOMETRY MANAGEMENT ########################################################
################################################################################


$frm_name ->               grid(-row=>1,-column=>1,-columnspan=>1, -pady=>10);

## Row 1 ##
$regexLabel ->             grid(-row=>1,-column=>1);
$regexEnt ->               grid(-row=>1,-column=>2);
$lbl_invertorder ->        grid(-row=>1,-column=>3);
$rdb_invOrd_yes ->         grid(-row=>1,-column=>4);
$rdb_invOrd_no ->          grid(-row=>1,-column=>5);

## Row 2 ##
$filenameLabel ->          grid(-row=>2,-column=>1);
$fileEnt ->                grid(-row=>2,-column=>2);
$lbl_caseSensitive ->      grid(-row=>2,-column=>3);
$rdb_caseSensitive_yes ->  grid(-row=>2,-column=>4);
$rdb_caseSensitive_no ->   grid(-row=>2,-column=>5);

$processButton ->           grid(-row=>1,-column=>6, -rowspan=>2,-padx=>10, -sticky=>"ns");

$textareacontent ->        grid(-row=>1,-column=>1);
$srl_y ->                  grid(-row=>1,-column=>2,-sticky=>"ns");
$srl_x ->                  grid(-row=>2,-column=>1,-sticky=>"ew");
$textarea ->               grid(-row=>4,-column=>1,-columnspan=>200);

################################################################################
### SUPPLEMENTARY FUNCTIONS ####################################################
################################################################################

sub printoptions{
  print "\tUsing options:\n";
  print "\t\tBytes           = $numberofbytes\n";
  print "\t\tSigned          = $issigned\n";
  print "\t\tSwap byte order = $invertorder\n";
  print "\t\tFloat           = $isfloat\n";
  print "\t\tVerbosity level = $verboselevel\n";
}

sub printhelp{
  my $help = 
"Usage: hextoval.pl [OPTION]... [FILE]...
Hex To Val (hextoval.pl). Conversion from raw data types
to human readable formats (trivial CSV file):

Options:

  -s  input values are signed
  -u  input values are unsigned (default)
  -1  input values are one byte long
  -2  ... two bytes long (default)
  -4  ... four bytes long
  -f  input values are floats (do not use with -1/-2/-4/-s/-u)
  -i  invert the byte order
  -I  normal byte order (default)
  -v  for each v present increase the verbosity of the output upto three -v
  -h  print this super helpful help message!

The input format looks like this:

  \$00000001 : 5E 15 00 00 18 05 00 00 3B 0D 00 00 62 15 00 00 
  \$00000002 : 16 05 00 00 3A 0D 00 00 5E 15 00 00 1C 05 00 00 
  ...
  \$000000FA : 37 0D 00 00 5C 15 00 00 1B 05 00 00 3C 0D 00 00 

The first two colums are discarded leaving behind:

  5E 15 00 00 18 05 00 00 3B 0D 00 00 62 15 00 00 
  16 05 00 00 3A 0D 00 00 5E 15 00 00 1C 05 00 00 
  ...
  37 0D 00 00 5C 15 00 00 1B 05 00 00 3C 0D 00 00 

Which when our example is processed with -ui4 gives:

  5470
  1304
  3387
  5474
  1302
  3386
  5470
  1308
  ...
  3383
  5468
  1307
  3388

Files are automatically renamed, for example \"filename.extension\" 
will become \"filename.csv\".  

You can changes options halfway through the argument list and any
files after that point will adopt the new options, for example:

  ./hextoval.pl -u4 file1.txt file2.txt -f file3.txt -s1 file4.txt;

Will create four files converted from the following types:

  file1.csv : from unsigned four byte values
  file2.csv : from unsigned four byte values
  file3.csv : from float
  file4.csv : from signed single byte values

Report all problems to <howe.r.j.89\@gmail.com>

";
  print $help;
}

## Process command line options
sub getops($){
  my $returnval = 0; ## return true if we have a valid modifier

	if ($_[0] =~ /^-/){ ## test for -{s,u,{1,2,4},i,I,v,f}
		my @command = split //,$_[0];
		foreach my $commandindex (1 .. $#command){
			my $currentcommand  = $command[$commandindex];
			if($currentcommand eq "1"){
				$numberofbytes    = 1;
				$nibbles          = "H2";
				$unpackstr        = "c";
			} elsif($currentcommand eq "2"){
				$numberofbytes    = 2;
				$nibbles          = "H4";
				$unpackstr        = "s";
			} elsif($currentcommand eq "4"){
				$numberofbytes    = 4;
				$nibbles          = "H8";
				$unpackstr        = "l";
      } elsif($currentcommand eq "f"){
        $numberofbytes    = 4;
        $nibbles          = "H8";
        $isfloat          = 1;
				$unpackstr        = "f";
			} elsif($currentcommand eq "u"){
				$issigned         = 0;
        $isfloat          = 0;
			} elsif($currentcommand eq "s"){
				$issigned         = 1;
        $isfloat          = 0;
			} elsif($currentcommand eq "I"){
				$invertorder         = 0;
			} elsif($currentcommand eq "i"){
				$invertorder         = 1;
      } elsif($currentcommand eq "v"){
        $verboselevel++;  # for different verbosity levels
			} elsif($currentcommand eq "h"){
				&printhelp;
			} else {
				die ("$currentcommand is not a valid option");
			}
		}
    $verboselevel?&printoptions:0;
    $returnval = 1;
	} else {
    $returnval = 0;
  }
  ($issigned == 1) && ($unpackstr eq "S") && die("<Signed is true but length not specified>");
  return $returnval;
}

## Renames the output from filename.extension to filename.csv
#  *or* to from filename to filename.csv adding an extension
#  where there was none.
sub renameoutputfile($){
  my $outputfile;
	my @tmpstr     = split(/\..*$/,$_[0]);
	if(defined($tmpstr[0])){
		$outputfile = $tmpstr[0] . ".csv";
	} else {
		$outputfile = "$_[0].csv";
	}
  return $outputfile;
}

## Process the input stream $_[0], putting the results in the
# output stream $_[1]. This is the function that actually
# *does* stuff, reading in nibbles and turning them into their
# correct type.
sub processinputstream($$){

  my $INPUT         = $_[0];
  my $OUTPUT        = $_[1];
	my $linecount     = 0;
  my $variablecount = 0;

	while(<$INPUT>){
		my @s = split;

		if ((($#s - $ignoreXcolumns) + 1)  % $numberofbytes != 0 ){
#			print "Error:$inputfile:$linecount:$variablecount; Valid Columns % Number of bytes != Zero!\n";
			next;
		}

		for (my $i= $ignoreXcolumns; $i < $#s; $i = $i + $numberofbytes){
			my $tmp = "";
				for(my $j = $i; $j < $i + $numberofbytes; $j++){
					$tmp = $invertorder?$s[$j] . $tmp:$tmp . $s[$j]; # process endianess
          $verboselevel>2?print "$s[$j] ":0;
				}
      $verboselevel>2?print "\n":0;
      $variablecount++;

      # print in the desired format.
      my $printstr = "invalid";
      if($issigned == 1 || $isfloat == 1){
        $printstr = unpack($unpackstr, pack($nibbles, $tmp)) . "\n";
			} else { 
        $printstr = hex $tmp;
        $printstr .= "\n";
			}
				print $OUTPUT $printstr;
        $verboselevel>1?print $printstr:0;
		}
    $linecount++;
	}
  return $linecount, $variablecount;
}

################################################################################
### MAIN #######################################################################
################################################################################

if(defined($ARGV[0])){
  &commandlineprocess;
} else {
  Tk::MainLoop;
}

sub tkprocessfiles{
  print "Not implemented\n";
}

sub commandlineprocess{
  ## for each file, convert to CSV
  #  print "Currently processing:\n";
  for (my $argnum=0; $argnum < $#ARGV + 1; $argnum = $argnum + 1){

    ## if we have an "option", that is any argument that begins
    # with a '-', process this as a command.
    if(&getops($ARGV[$argnum])){
        next;
    }

    my $inputfile  = $ARGV[$argnum];
    my $outputfile = &renameoutputfile($ARGV[$argnum]);
    print "\t$outputfile:\n";

    open my $INPUT,"<","$inputfile"  || die "Could not open $inputfile";
    open my $OUTPUT,">","$outputfile" || die "Coult not open $outputfile for writing";

    my ($linecount, $variablecount) = &processinputstream($INPUT,$OUTPUT);

    print "\t\tLines           = $linecount\n";
    print "\t\tVariables       = $variablecount\n";

    close $INPUT;
    close $OUTPUT;
  }
  print "Done!\n";
}
################################################################################
### EOF ########################################################################
################################################################################
