#!/usr/athena/bin/perl

# timestamps: Prepare for a "cvs import -d" by resetting the mod times
# in the current directory tree such that all files are ordered
# properly relative to each other but are all between the current time
# and N seconds in the past, where N is the number of files in the
# tree.  This way, source files don't appear spuriously out of date
# with respect to each other, but checkouts by date do not break
# appreciably.

use File::Find;

sub wanted {
    return if (! -f $_);
    $mtime = (lstat)[9];
    push @{$files{$mtime}}, $File::Find::name;
}

find(\&wanted, '.');

$newmtime = time - scalar keys(%files);
foreach $mtime (sort(keys(%files))) {
    $newmtime++;
    utime $newmtime, $newmtime, @{$files{$mtime}};
}
