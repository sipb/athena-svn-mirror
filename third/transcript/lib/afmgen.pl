# Synopsis:
#     perl -n afmgen.pl afmfiles.upr.orig 1>afmfiles.upr 2>install-commands
#
# $Header: /afs/dev.mit.edu/source/repository/third/transcript/lib/afmgen.pl,v 1.1 1996-10-14 04:47:28 ghudson Exp $
#
# This script is used to support the use of full font names for filenames
# of AFM font files.  It reads the original Adobe afmfiles.upr, which
# contains (among other things) lines referring to "short" filenames like
#     AvantGarde-Book=AvGarBk.afm
# and outputs an `expanded' afmfiles.upr on STDOUT and a list of commands
# needed to install the fonts with "long" names on STDERR.

s%@AFMDIR@%$ENV{'AFMDIR'}%g;

if (/^([^=]*)=([^=]*\.afm)$/) {
   print "$1=$1.afm\n";
   print STDERR $ENV{'INSTALL_DATA'} ." lib/$2 \$DESTDIR" .
       $ENV{'AFMDIR'} . "/$1.afm\n";
} else {
   print;
}
