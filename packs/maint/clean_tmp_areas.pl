#!/usr/bin/perl

use strict;
use File::Find;

# The values here are minimum numbers of days.  Negative means mtime,
# positive means atime.
my %dirs = ("/tmp" => 2,
	    "/var/tmp" => 3,
	    "/var/preserve" => -4);
my $days;

my @usernames = `w -h | awk '{print \$1}' | sort -u`;
my @uids = map { chomp; my ($a,$b,$uid) = getpwnam($_); $uid; } @usernames;
my %uidset;
foreach my $uid (@uids) {
    $uidset{$uid} = 1;
}

sub visit {
    my ($dev, $ino, $mode, $nlink, $uid, $gid) = lstat($_);
    return if (-b _ || -c _ || -p _ || -S _);
    return if ($_ eq ".");
    return if ($uidset{$uid});
    if (-d _) {
	return if (-M _ < 2);
	system "/bin/athena/saferm", "-d", "-q", $_;
    } else {
	if ($days < 0) {
	    return if (-M _ < -$days);
	} else {
	    return if (-A _ < $days);
	}
	system "/bin/athena/saferm", $_;
    }
}

foreach my $dir (keys(%dirs)) {
    chdir $dir || die "Can't chdir to $dir";
    $days = $dirs{$dir};
    find({wanted => \&visit,
	  bydepth => 1,
	  no_chdir => 1}, ".");
}

exit 0;
