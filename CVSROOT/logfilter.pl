#!/usr/athena/bin/perl

# In commitinfo, put:
#   ALL  $CVSROOT/CVSROOT/record.pl $CVSROOT
# In loginfo, put:
#   ALL  $CVSROOT/CVSROOT/logfilter.pl %{sVv} commit-address diff-adddress

# Mails the CVS commit logs to commit-address.  Also mails the commit
# logs to diff-address, followed by context diffs of the modified
# files.

use strict;
use File::Compare;
use File::Path;
use File::Spec::Functions ':ALL';
use File::stat;
use Fcntl ':mode';

my ($sendmail, $dir, @files, $commit_addr, $diff_addr, $tmpdir, $sb);
my ($filename, $firstdir, $lastdir, $repdir, $old, $new, $file);
my (@d1, @d2, $i, $subject, $cmd);

$sendmail = ( -x "/usr/sbin/sendmail") ? "/usr/sbin/sendmail"
    : "/usr/lib/sendmail";

# $ARGV[0] is of the form "dir filename,oldrev,newrev ...".
($dir, @files) = split(/ /, $ARGV[0]);
$commit_addr = $ARGV[1];
$diff_addr = $ARGV[2];

if ($files[0] eq "-") {
    # This is a new directory or import notification.  There's no
    # commitinfo step for these operations, so just pass on the
    # supplied log message.

    open(COMMITMAIL, "| $sendmail $commit_addr");
    print COMMITMAIL "To: $commit_addr\n";
    print COMMITMAIL "Subject: $ARGV[0]\n\n";

    open(DIFFMAIL, "| $sendmail $diff_addr");
    print DIFFMAIL "To: $diff_addr\n";
    print DIFFMAIL "Subject: $ARGV[0]\n\n";

    while (<STDIN>) {
	print COMMITMAIL;
	print DIFFMAIL;
    }
    close(COMMITMAIL);
    close(DIFFMAIL);

    exit 0;
}

# Make sure our temporary directory exists and is kosher.
$tmpdir = "/tmp/athena-cvs-" . getpgrp();
$sb = lstat($tmpdir) || die "Can't stat $tmpdir: $!\n";
if (!S_ISDIR($sb->mode) || ($sb->mode & (S_IWGRP|S_IWOTH)) || $sb->uid != $>) {
    die "Problem with temporary directory\n";
}

# Read the first committed directory (recorded from commitinfo by record.pl).
$filename = catfile($tmpdir, "firstdir");
open(FILE, "< $filename") || die "Can't open $filename for reading: $!\n";
$firstdir = <FILE> || die "Can't read $filename: $!\n";
chomp $firstdir;
close(FILE);

# Read the last committed directory (recorded from commitinfo by record.pl).
$filename = catfile($tmpdir, "lastdir");
open(FILE, "< $filename") || die "Can't open $filename for reading: $!\n";
$lastdir = <FILE> || die "Can't read $filename: $!\n";
chomp $lastdir;
close(FILE);

# Write out the log message to a file.
$filename = catfile($tmpdir, "logs");
open(FILE, ">> $filename") || die "Can't open $filename for writing: $!\n";
while (<STDIN>) {
    if (/^Update of (.*)$/) {
	# Remember the repository directory for the diffs.
	$repdir = $1;
    }
    print FILE;
}
close(FILE);

$filename = catfile($tmpdir, "diffs");
open(FILE, ">> $filename") || die "Can't open $filename for append: $!\n";
# Write out the diffs.
foreach (@files) {
    ($file, $old, $new) = split(/,/);
    next if ($new eq "NONE");
    print FILE "\n";
    print FILE "==================================================\n";
    if ($old eq "NONE") {
	print FILE "Initial contents of new file $file\n";
	$cmd = "diff -c /dev/null $file";
    } else {
	print FILE "Differences for $file (revision $old -> $new)\n";
	$cmd ="rcsdiff -c -kk -r$old -r$new $repdir/$file 2>/dev/null";
    }
    print FILE "==================================================\n";
    open(DIFF, "$cmd|");
    print FILE while (<DIFF>);
    close(DIFF);
}
close(FILE);

if ($dir eq $lastdir) {
    # All the logs are in.  Fire off the mail messages.  But first,
    # find the common initial sequence of the first and last
    # directories for the subject line.
    @d1 = splitdir($firstdir);
    @d2 = splitdir($lastdir);
    for ($i = 0; $i < scalar @d1; $i++) {
	last if ($d1[$i] ne $d2[$i]);
    }
    $subject = catdir(@d1[0..($i - 1)]);

    open(COMMITMAIL, "| $sendmail $commit_addr");
    print COMMITMAIL "To: $commit_addr\n";
    print COMMITMAIL "Subject: $subject\n\n";

    open(DIFFMAIL, "| $sendmail $diff_addr");
    print DIFFMAIL "To: $diff_addr\n";
    print DIFFMAIL "Subject: $subject\n\n";

    $filename = catfile($tmpdir, "logs");
    open(FILE, "< $filename") || die "Can't open $filename for reading: $!\n";
    while (<FILE>) {
	print COMMITMAIL;
	print DIFFMAIL;
    }
    close(FILE);
    close(COMMITMAIL);

    $filename = catfile($tmpdir, "diffs");
    open(FILE, "< $filename") || die "Can't open $filename for reading: $!\n";
    print DIFFMAIL while (<FILE>);
    close(FILE);
    close(DIFFMAIL);

    rmtree($tmpdir);
}
