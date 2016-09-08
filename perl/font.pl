#!/usr/bin/env perl
use Tk;
use Tk::BrowseEntry;
use strict;

my $mw = MainWindow->new(-title => 'Font Viewer');
my $f = $mw->Frame->pack(-side => 'top');

my $family = 'Courier';
my $be = $f->BrowseEntry(-label => 'Family:', -variable => \$family,
  -browsecmd => \&apply_font)->pack(-fill => 'x', -side => 'left');
$be->insert('end', sort $mw->fontFamilies);

my $size = 24;
my $bentry = $f->BrowseEntry(-label => 'Size:', -variable => \$size, 
  -browsecmd => \&apply_font)->pack(-side => 'left');
$bentry->insert('end', (3 .. 32));

my $weight = 'normal';
$f->Checkbutton(-onvalue => 'bold', -offvalue => 'normal', 
  -text => 'Weight', -variable => \$weight, 
  -command => \&apply_font)->pack(-side => 'left');

my $slant = 'roman';
$f->Checkbutton(-onvalue => 'italic', -offvalue => 'roman', 
  -text => 'Slant', -variable => \$slant, 
  -command => \&apply_font)->pack(-side => 'left');

my $underline = 0;
$f->Checkbutton(-text => 'Underline', -variable => \$underline, 
  -command => \&apply_font)->pack(-side => 'left');

my $overstrike = 0; 
$f->Checkbutton(-text => 'Overstrike', -variable => \$overstrike, 
  -command => \&apply_font)->pack(-side => 'left');

my $stext = 'Sample Text';
my $sample = $mw->Entry(-textvariable => \$stext)->pack(-fill => 'x');

&apply_font;

MainLoop;

sub apply_font {
  # Specify all options for font in an anonymous array
  print "here\n";
  $sample->configure(-font => 
    [-family => $family,
     -size => $size,
     -weight => $weight,
     -slant => $slant,
     -underline => $underline,
     -overstrike => $overstrike]);
}
