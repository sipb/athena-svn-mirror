#!/usr/bin/perl
# $Id: hostinfo.pl,v 1.5 2002-06-11 21:12:59 jweiss Exp $

# Copyright 1998 by the Massachusetts Institute of Technology.
#
# Permission to use, copy, modify, and distribute this
# software and its documentation for any purpose and without
# fee is hereby granted, provided that the above copyright
# notice appear in all copies and that both that copyright
# notice and this permission notice appear in supporting
# documentation, and that the name of M.I.T. not be used in
# advertising or publicity pertaining to distribution of the
# software without specific, written prior permission.
# M.I.T. makes no representations about the suitability of
# this software for any purpose.  It is provided "as is"
# without express or implied warranty.

# This is a simple DNS querying program. It was originally written in
# C, but has been rewritten as a wrapper around "host" in the BIND
# distribution.

$hostprog = "/usr/bin/host";

$long_output = $show_host = $show_addr = $show_hinfo = $show_mx = 1;
$server = "";

sub usage {
    print STDERR "Usage: hostinfo <options> <host-names-or-addresses>\n";
    print STDERR "  -h: output only hostname\n";
    print STDERR "  -a: output only address\n";
    print STDERR "  -i: output only host info record\n";
    print STDERR "  -m: output only mx record\n";
    print STDERR "  -q: disable additional query for hinfo & mx\n";
    print STDERR "  -ns <server>: ask specified name server\n";
    exit 1;
}

if (! @ARGV) {
    &usage();
}

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
	$server = shift(@ARGV) || &usage();
	if ($server =~ /[^a-zA-Z0-9.-]/) {
	    print STDERR "Bad hostname '$server'.\n";
	    &usage();
	}
	system "$hostprog -t a $server > /dev/null 2>&1";
	if ($?) {
	    print STDERR "No such host '$server'.\n\n";
	    $server = "";
	}
	next;
    } elsif ($arg =~ /^-/) {
	&usage();
    }

    if ($arg =~ /[^a-zA-Z0-9.-]/) {
	print STDERR "Bad hostname '$arg'.\n";
	next;
    }

    $host = $hinfo = "";
    @addr = @mx = ();

    if ($show_mx) {
	open(HOST, "$hostprog -t mx '$arg' $server 2>&1 |");
	while (<HOST>) {
	    if (/mail is handled .*by [ \d]*([^ ]*)$/) {
		push(@mx, $1);
	    }
	}
	close(HOST);
    }
    chomp (@mx);

    open(HOST, "$hostprog -t a '$arg' $server 2>&1 |");
    while (<HOST>) {
	if (/(.*) has address (.*)$/) {
	    $host = $1;
	    push(@addr, $2);
	} elsif ( /domain name pointer (.*)$/) {
	    $host = $1;
	    push(@addr, $arg);
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
	} elsif (!@mx) {
	    print STDERR "No such host '$arg'.\n";
	} elsif (!$show_mx) {
	    print STDERR "No address for '$arg'.\n";
	}
	next unless ($show_mx && @mx);
    }

    if ($show_hinfo) {
	open(HOST, "$hostprog -t hinfo '$host' $server 2>&1 |");
	while (<HOST>) {
	    if (/host information (.*)$/) {
		$hinfo = $1;
		$hinfo =~ s: :/:;
	    }
	}
    }

    if ($long_output) {
	print "Desired host:\t$arg\n";
	if ($host) {
	    print "Official name:\t$host\n";
	}
	foreach (@addr) { print "Host address:\t$_\n"; }
	print "Host info:\t$hinfo\n" if $show_hinfo && $hinfo;
	foreach (@mx) { print "MX address:\t$_\n" if $show_mx; }
    } elsif ($show_host && $host) {
	print "$host\n";
    } elsif ($show_addr && @addr) {
	foreach (@addr) { print "$_\n"; }
    } elsif ($show_hinfo && $hinfo) {
	print "$hinfo\n";
    } elsif ($show_mx && @mx) {
	foreach (@mx) { print "$_\n"; }
    }

    print "\n" if $long_output && @ARGV;
}
