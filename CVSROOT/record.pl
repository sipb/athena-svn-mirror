#!/usr/athena/bin/perl

# In commitinfo, put:
#   ALL  $CVSROOT/CVSROOT/record.pl $CVSROOT
# In loginfo, put:
#   ALL  $CVSROOT/CVSROOT/logfilter.pl %{sVv} commit-address diff-adddress

# This script is invoked by cvs once per directory in the commit, with
# the directory name as an argument.  We record the first directory in
# a file named "firstdir" and the last directory in a file named
# "lastdir".

use strict;
use File::Spec::Functions;
use File::stat;
use Fcntl ':mode';

my ($cvsroot, $dir, $reldir, $tmpdir, $sb, $filename);

# commitinfo gives us the directory with repository path, while
# loginfo will give logfilter.pl the directory relative to the
# repository.  Chop off the repository part here so that we can
# compare there.
$cvsroot = $ARGV[0];
$cvsroot .= "/" unless ($cvsroot =~ /\/$/);
$dir = $ARGV[1];
$reldir = substr($dir, length($cvsroot));

$tmpdir = "/tmp/athena-cvs-" . getpgrp();
if (! -e $tmpdir) {
    mkdir($tmpdir, 0755) || die("Can't mkdir $tmpdir: $!\n");
}
$sb = lstat($tmpdir);
if (!S_ISDIR($sb->mode) || ($sb->mode & (S_IWGRP|S_IWOTH)) || $sb->uid != $>) {
    die "Problem with temporary directory";
}

# Record the first directory if that hasn't been done yet.
$filename = catfile($tmpdir, "firstdir");
if (! -e $filename) {
    open(FILE, "> $filename") || die("Can't open $filename for writing: $!\n");
    print FILE $reldir, "\n";
    close(FILE);
}

# Record the last directory, overwriting previous settings of it.
$filename = catfile($tmpdir, "lastdir");
open(FILE, "> $filename") || die("Can't open $filename for writing: $!\n");
print FILE $reldir, "\n";
close(FILE);
