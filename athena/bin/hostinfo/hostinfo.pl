#!/usr/athena/bin/perl

$hostprog = "/usr/athena/bin/host";

$long_output = $show_host = $show_addr = $show_hinfo = $show_mx = 1;
$server = "localhost";

while (@ARGV) {
    my $arg = shift(@ARGV);
    if ($arg eq "-h") {
	$show_host = 1;
	$long_output = $show_addr = $show_hinfo = $show_mx = 0;
	next;
    } elsif ($arg eq "-a") {
	$show_addr = 1;
	$long_output = $show_host = $show_hinfo = $show_mx = 0;
	next;
    } elsif ($arg eq "-i") {
	$show_hinfo = 1;
	$long_output = $show_host = $show_addr = $show_mx = 0;
	next;
    } elsif ($arg eq "-m") {
	$show_mx = 1;
	$long_output = $show_host = $show_addr = $show_hinfo = 0;
	next;
    } elsif ($arg eq "-q") {
	$long_output = $show_host = $show_addr = 1;
	$show_hinfo = $show_mx = 0;
	next;
    } elsif ($arg eq "-ns") {
	if ($arg =~ /[^a-zA-Z0-9.-]/) {
	    print STDERR "Bad hostname '$arg'.\n";
	    next;
	}
	$server = shift(@ARGV);
	system "$hostprog -t a $server > /dev/null 2>&1";
	if ($?) {
	    print STDERR "No such host '$server'.\n\n";
	    $server = "localhost";
	}
	next;
    } elsif ($arg =~ /^-/) {
	print STDERR "Usage: hostinfo <options> <host-names-or-addresses>\n";
	print STDERR "  -h: output only hostname\n";
	print STDERR "  -a: output only address\n";
	print STDERR "  -i: output only host info record\n";
	print STDERR "  -m: output only mx record\n";
	print STDERR "  -q: disable additional query for hinfo & mx\n";
	print STDERR "  -ns <server>: ask specified name server\n";
	exit 1;
    }

    if ($arg =~ /[^a-zA-Z0-9.-]/) {
	print STDERR "Bad hostname '$arg'.\n";
	next;
    }

    $host = $mx = $hinfo = "";
    @addr = ();

    if (!$show_mx) {
	$flags = "-t a";
    } else {
	$flags = "";
    }

    open(HOST, "$hostprog $flags '$arg' '$server' 2>&1 |");
    while (<HOST>) {
	if (/(.*) has address (.*)$/) {
	    $host = $1;
	    push(@addr, $2);
	} elsif ( /domain name pointer (.*)$/) {
	    $host = $1;
	    push(@addr, $arg);
	} elsif (/mail is handled \(pri=\d+\) by (.*)$/) {
	    $mx = $1;
	} else {
	    $error = $error . $_;
	}
    }
    close(HOST);

    if (!$host) {
	if ($error =~ /Host not found\./) {
	    print STDERR "No such host '$arg'.\n";
	} elsif ($error =~ /try again|No recovery/) {
	    print STDERR "Cannot resolve name '$arg' due to network difficulties.\n";
	} elsif (!$mx) {
	    print STDERR "No such host '$arg'.\n";
	} else {
	    print STDERR "No address for '$arg'.\n";
	}
	next;
    }

    if ($show_hinfo) {
	open(HOST, "$hostprog -t hinfo '$host' '$server' 2>&1 |");
	while (<HOST>) {
	    if (/host information (.*)$/) {
		$hinfo = $1;
		$hinfo =~ s: :/:;
	    }
	}
    }

    if ($long_output) {
	print "Desired host:\t$arg\n";
	print "Official name:\t$host\n";
	foreach (@addr) { print "Host address:\t$_\n"; }
	print "Host info:\t$hinfo\n" if $show_hinfo && $hinfo;
	print "MX address:\t$mx\n" if $show_mx && $mx;
    } elsif ($show_host && $host) {
	print "$host\n";
    } elsif ($show_addr && @addr) {
	foreach (@addr) { print "$_\n"; }
    } elsif ($show_hinfo && $hinfo) {
	print "$hinfo\n";
    } elsif ($show_mx && $mx) {
	print "$mx\n";
    }

    print "\n" if $long_output && @ARGV;
}
