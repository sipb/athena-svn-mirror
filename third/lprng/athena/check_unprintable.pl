#!/usr/athena/bin/perl

$_ = <>;

if (/^%PDF-/) { print "PDF\n"; }
elsif (/^<MakerFile /) { print "FrameMaker\n"; }
elsif (/[\x80-\xFF]/) { print "binary\n"; }
else { print "text\n"; }

while (<>) {}
