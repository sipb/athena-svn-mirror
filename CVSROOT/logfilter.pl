#!/usr/athena/bin/perl

# Usage (with CVS 1.10 loginfo):
# 	logfilter %{sVv} commit-address diff-adddress

# Mails the CVS commit log to commit-address.  Also mails the commit
# log to diff-address, followed by context diffs of the modified
# files.

# This script is not specific to any particular repository, but it is
# specific to the Athena environment in that it assumes the existence
# of /usr/athena/bin/perl.

$sendmail = ( -x "/usr/sbin/sendmail") ? "/usr/sbin/sendmail"
    : "/usr/lib/sendmail";

# Create subject line (derive %s from %{sVv}).  $ARGV[0] is of the
# form "dir filename,oldrev,newrev ..."; what we want is
#  "dir filename ..."
@files = split(/ /, $ARGV[0]);
$dir = shift @files;
map { s/,.*// } @files;
$subject = $dir . " " . join(' ', @files);

open(COMMITMAIL, "|$sendmail $ARGV[1]");
print COMMITMAIL "To: $ARGV[1]\n";
print COMMITMAIL "Subject: $subject\n\n";

open(DIFFMAIL, "|$sendmail $ARGV[2]");
print DIFFMAIL "To: $ARGV[2]\n";
print DIFFMAIL "Subject: $subject\n\n";

# Display the commit log as proffered by CVS.  Remember the repository
# directory.
while (<STDIN>) {
	if (/^Update of (.*)$/) {
		$repdir = $1;
	}
	print COMMITMAIL;
	print DIFFMAIL;
}

close COMMITMAIL;

# Display diffs.  $ARGV[0] is of the form "dir filename,oldrev,newrev ...".
@files = split(/ /, $ARGV[0]);
$dir = shift @files;
foreach (@files) {
	($file, $old, $new) = split(/,/);
	if ($new eq "NONE") {
		next;
	}
	print DIFFMAIL "\n";
	print DIFFMAIL "==================================================\n";
	if ($old eq "NONE") {
		print DIFFMAIL "Initial contents of new file $file\n";
		$cmd = "diff -c /dev/null $i";
	} else {
		print DIFFMAIL "Differences for $file ";
		print DIFFMAIL "(revision $old -> $new)\n";
		$rcsfile = "$repdir/$file";
		$cmd ="rcsdiff -c -kk -r$old -r$new $rcsfile 2>/dev/null";
	}
	print DIFFMAIL "==================================================\n";
	open(DIFF, "$cmd|");
	print DIFFMAIL while (<DIFF>);
	close DIFF;
}

close DIFFMAIL;
