#!/usr/bin/perl

# http://interoperating.info/courses/perl4data/node/26

# Script to illustrate how to parse a simple XML file
# and dump its contents in a Perl hash record.

use strict;
use XML::Simple;
use Data::Dumper;

my $booklist = XMLin('booklist.xml');

print "dump:\n";

# dump
print Dumper($booklist);

print "element:\n";

# by element
my $booklist = XMLin('booklist.xml', KeyAttr => {book => 'isbn'});
print $booklist->{book}->{Book2ISBN}->{title} . "\n";

print "all titles:\n";

my $books = XMLin('booklist.xml');
# all books
foreach my $book (@{$books->{book}}) {
	print $book->{title} . "\n";
}

print "attr.\n";

# by attribute
foreach my $book (@{$books->{book}}) {
	if ($book->{type} eq 'technical') {
		print $book->{title} . "\n";
	}
}


# Writing

use XML::Writer;
use IO::File;

## Open output file
my $output_file = "myfile.xml";
my $output = new IO::File(">$output_file");

my $writer = new XML::Writer(OUTPUT=>$output, DATA_MODE=>1);

# Open a <record> element
$writer->startTag("record");

# Create a <title> element, with content
$writer->startTag("title");
$writer->characters("Snow Crash");
$writer->endTag("title");

$writer->startTag("title");
$writer->characters("1984");
$writer->endTag("title");

# Create other elements...

# Close a <record> element
$writer->endTag("record");

$writer->end();
$output->close();
