#!/usr/bin/perl -w

# $Id: mitmailcreate.pl,v 1.2 2004-07-29 19:11:52 rbasch Exp $

# Create, remove, subscribe to or unsubscribe to IMAP mailboxes.

use strict;
use warnings FATAL => 'all';
use Cyrus::IMAP;
use Getopt::Long;

sub usage(;$);
sub send_command($);
sub close_connection();
sub errorout($);

my $prog = $0;

my $imap_cmd;
my $create = 0;
my $create_subscribe_cmd;

if ($prog =~ m/create/o) {
    $create = 1;
    $imap_cmd = 'CREATE';
    # If the --no-subscribe option is given, we will unsubscribe the
    # mailbox explicitly, as a formerly existing mailbox of the same
    # name may still be in the subscription list.
    $create_subscribe_cmd = 'SUBSCRIBE';
} elsif ($prog =~ m/remove/o) {
    $imap_cmd = 'DELETE';
} elsif ($prog =~ m/unsubscribe/o) {
    $imap_cmd = 'UNSUBSCRIBE';
} elsif ($prog =~ m/subscribe/o) {
    $imap_cmd = 'SUBSCRIBE';
}

sub usage(;$) {
    print STDERR "$prog: $_[0]\n" if ($_[0] && $_[0] ne "help");
    print STDERR <<EOF;
Usage: $prog [<options>]
  Options:
    --debug                turn on debugging
    --help                 print this usage information
    --host=<name>          query host <name> instead of default POBOX server
EOF
    if ($create) {
	print STDERR <<EOF;
    --no-subscribe         do not add to list of subscribed mailboxes
EOF
    }
    exit 1;
}

# Parse the command line arguments.
use vars qw($opt_debug $opt_host $opt_no_subscribe);

GetOptions("debug",
	   "help" => \&usage,
	   "host=s",
	   "no-subscribe" => sub {
	       unless ($create) {
		   usage "Unknown option: no-subscribe";
	       }
	       $opt_no_subscribe = 1;
	       $create_subscribe_cmd = 'UNSUBSCRIBE';
	   }
	   ) || usage;

usage "Please specify a mailbox name" if @ARGV == 0;

my $username = $ENV{'ATHENA_USER'} || $ENV{'USER'} || getlogin || (getpwuid($<))[0] ||
    errorout "Cannot determine user name";

unless ($opt_host) {
    $opt_host = (gethostbyname("$username.mail.mit.edu"))[0];
    errorout "Cannot find Post Office server for $username" unless $opt_host;
}
errorout "Exchange accounts are not supported yet. Try http://owa.mit.edu/." if $opt_host =~ /EXCHANGE/;

# Connect to the IMAP server, and authenticate.
my $client = Cyrus::IMAP->new($opt_host) ||
    errorout "Cannot connect to IMAP server on $opt_host";
unless ($client->authenticate(-authz => $username, -maxssf => 0)) {
    close_connection();
    errorout "Cannot authenticate to $opt_host";
}

# Loop to act upon mailboxes given on the command line.
foreach (@ARGV) {
    send_command "$imap_cmd \"$_\"";
    # If creating the mailbox, subscribe or unsubscribe to it.
    if ($create) {
	send_command "$create_subscribe_cmd \"$_\"";
    }
}

# We are done talking to the IMAP server, close down the connection.
close_connection();

# Subroutine to send a command to the IMAP server, and wait for the
# response; any defined callbacks for the response are invoked.
# If the server response indicates failure, we error out.
sub send_command($) {
    print "Sending: $_[0]\n" if $opt_debug;
    my ($status, $text) = $client->send('', '', $_[0]);
    print "Response: status $status, text $text\n" if $opt_debug;
    errorout "Premature end-of-file on IMAP connection to $opt_host"
	if $status eq 'EOF';
    if ($status ne 'OK') {
	close_connection();
	errorout "IMAP error on $opt_host: $text" 
    }
}

# Logout from the IMAP server, and close the connection.
sub close_connection() {
    $client->send('', '', "LOGOUT");
    # Set the client reference to undef, so that perl invokes the
    # destructor, which closes the connection.  Note that if we invoke
    # the destructor explicitly here, then perl will still invoke it
    # again when the program exits, thus touching memory which has
    # already been freed.
    $client = undef;
}

sub errorout($) {
    print STDERR "$prog: $_[0]\n";
    exit 1;
}
