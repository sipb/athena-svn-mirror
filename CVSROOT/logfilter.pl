#!/usr/athena/bin/perl

# Usage:
#	logfilter subject commit-address diff-adddress

# Mails the CVS commit log to commit-address.  Also mails the commit log to
# diff-address, followed by context diffs of the modified files and the
# initial contents of added files.

# This script is not specific to any particular repository, but it is
# specific to the Athena environment in that it assumes the existence
# of /usr/athena/bin/perl and of mhmail.

open(COMMITMAIL, "|mhmail -s '$ARGV[0]' $ARGV[1]");
open(DIFFMAIL, "|mhmail -s '$ARGV[0]' $ARGV[2]");

# Display the commit log as proffered by CVS.  Remember the repository
# directory, the branch, and the added and modified files.
while (<STDIN>) {
	print COMMITMAIL;
	print DIFFMAIL;
	if (/^Update of (.*)$/) {
		$repdir = $1;
	}
	if (/^Revision\/Branch: (.*)$/) {
		$branch = $1;
	}
	if (/^Added Files:$/) {
		$line = <STDIN>;
		print COMMITMAIL $line;
		print DIFFMAIL $line;
		$line =~ s/^\s+//;
		@added = split(/\s/, $line);
	}
	if (/^Modified Files:$/) {
		$line = <STDIN>;
		print COMMITMAIL $line;
		print DIFFMAIL $line;
		$line =~ s/^\s+//;
		@modified = split(/\s+/, $line);
	}
	if (/^Log Message:$/) {
		# Flush the rest of the text to avoid confusion.
		while (<STDIN>) {
			print COMMITMAIL;
			print DIFFMAIL;
		}
	}
}

close COMMITMAIL;

foreach $i (@modified) {
	$rcsfile = "$repdir/$i,v";
	open(RCSFILE, "<$rcsfile") || die "Can't open $rcsfile";
	if ($branch) {
		# Skip to the symbols section.
		while (<RCSFILE> !~ /^symbols$/) {
		}

		# Compare each symbol with the branch.
		while (($line = <RCSFILE>) =~ /^\s+([^:]+):([0-9\.]+)$/) {
			if ($1 eq $branch) {
				# The prefix is the symbol value without the
				# .0 in the second-to-last slot.  The base
				# revision for the branch is the symbol value
				# before the .0.
				die if ($2 !~ /^([0-9\.]+)\.0\.([0-9]+)$/);
				$prefix = $1 . "." . $2 . ".";
				$base = $1;
			}
		}
	} else {
		$prefix = "1.";
	}

	# Scan for revisions starting with the prefix.
	$maxrev = 0;
	while (<RCSFILE>) {
		if (/^[0-9\.]+$/) {
			# It's a revision number; does it start with $prefix?
			chop;
			if (substr($_, 0, length($prefix)) eq $prefix) {
				$rev = substr($_, length($prefix));
				next if ($rev !~ /^[0-9]+$/);

				# Remember the highest revision we've seen.
				if ($rev > $maxrev) {
					$maxrev = $rev;
				}
			}
		}
		if (/^desc$/) {
			last;
		}
	}
	close RCSFILE;

	# Determine current and previous revision.
	if ($maxrev == 0 || ($maxrev == 1 && !$branch)) {
		die "Couldn't find suitable current revision.";
	}
	$current = $prefix . $maxrev;
	$prev = ($maxrev == 1) ? $base : $prefix . ($maxrev - 1);

	# Add the diff to the mail message.
	print DIFFMAIL "\n";
	print DIFFMAIL "==================================================\n";
	print DIFFMAIL "Differences for $i (revision $prev -> $current)\n";
	print DIFFMAIL "==================================================\n";
	open(DIFF, "rcsdiff -c -kk -r$prev -r$current $rcsfile 2>/dev/null|");
	print DIFFMAIL while (<DIFF>);
	close DIFF;
}

foreach $i (@added) {
	print DIFFMAIL "\n";
	print DIFFMAIL "==================================================\n";
	print DIFFMAIL "Initial contents of new file $i\n";
	print DIFFMAIL "==================================================\n";
	open(DIFF, "<$i") || die "Can't open added file $i";
	print DIFFMAIL while (<DIFF>);
	close DIFF;
}

close DIFFMAIL;
