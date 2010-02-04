#!/usr/bin/perl -w

# $Id: mailusage.pl,v 1.4 2004-09-03 20:40:31 rbasch Exp $

# Get the total size of, and number of messages in, mailboxes on an
# IMAP server.

use strict;
use warnings FATAL => 'all';
use Cyrus::IMAP;
use Getopt::Std;

sub usage(;$);
sub get_usage($);
sub send_command(@);
sub list_callback(@);
sub fetch_callback(@);
sub number_callback(@);
sub close_and_errorout($);
sub close_connection();
sub errorout($);

sub usage(;$) {
    print STDERR "mailusage: $_[0]\n" if $_[0];
    print STDERR <<EOF;
Usage: mailusage [<options>] [<user>]
  Options:
    -h <host>     query <host> instead of default post office server
    -m <mailbox>  query for <mailbox> only (default is all)
    -n            suppress the header line
    -r            query recursively for all mailbox descendents
    -s            display only subscribed mailboxes
    -d            turn on debugging
EOF
    exit 1;
}

# Parse the command line arguments.
my %opts;
getopts('dh:m:nrs', \%opts) || usage;
my $username = shift @ARGV || $ENV{'ATHENA_USER'} || $ENV{'USER'} || getlogin || (getpwuid($<))[0] ||
	errorout "Cannot determine user name";

usage "Too many arguments" if @ARGV > 0;
my $debug = $opts{'d'};
my $host = $opts{'h'} || (gethostbyname("$username.mail.mit.edu"))[0];
errorout "Cannot find Post Office server for $username" unless $host;
errorout "Exchange accounts are not supported yet. Try http://owa.mit.edu/." if $host =~ /EXCHANGE/;
my $mbox = $opts{'m'} || '*';
my $noheader = $opts{'n'};
my $recurse = $opts{'r'};
my $list_cmd = ($opts{'s'} ? 'LSUB' : 'LIST');

# Connect to the IMAP server, and authenticate.
my $client = Cyrus::IMAP->new($host) ||
    errorout "Cannot connect to IMAP server on $host";
unless ($client->authenticate(-authz => $username, -maxssf => 0)) {
    close_connection();
    errorout "Cannot authenticate to $host";
}

# Get all mailboxes of interest.  %mailboxes is a hash whose keys are
# the mailbox names; the values are hashes with "attributes" and
# "delimiter" keys.
my %mailboxes = ();
$client->addcallback({-trigger => $list_cmd,
		      -callback => \&list_callback,
		      -rock => \%mailboxes});

# First list the given mailbox.
send_command("$list_cmd %s %s", '', $mbox);

# If recursing, also list all descendents of the mailbox, unless the
# given name contains a trailing wildcard.
send_command("$list_cmd %s %s%s*", '', $mbox, $mailboxes{$mbox}{delimiter})
    if ($recurse && $mailboxes{$mbox} && $mailboxes{$mbox}{delimiter} &&
	$mbox !~ m/\*$/o);

if (%mailboxes) {
    # We now have all of the mailboxes of interest.  Get and display
    # the total size and number of messages for each one.
    foreach my $name (sort keys %mailboxes) {
	# Skip any mailbox that cannot be selected.
	next if $mailboxes{$name}{attributes} =~ m/\\Noselect\b/;
	my ($size, $nmsgs) = get_usage($name);
	unless ($noheader) {
	    print "Size in KB   #Messages  Mailbox\n";
	    $noheader = 1;
	}
	printf("%10d  %10d  %s\n", int(($size + 1023) / 1024), $nmsgs, $name);
    }
} else {
    close_and_errorout "No such mailbox \"$mbox\"";
}

# We are done talking to the IMAP server; close down the connection.
close_connection();

# Subroutine to obtain the usage for a given mailbox name.  It returns
# the total size, i.e. a sum of sizes of all messages in the mailbox,
# and the number of messages.
sub get_usage($) {
    my $mbox = $_[0];
    my %usage = (totalsize => 0, msgcount => 0);
    my $exists = 0;
    my $cb_numbered = Cyrus::IMAP::CALLBACK_NUMBERED;
    $client->addcallback({-trigger => 'EXISTS', -flags => $cb_numbered,
			  -callback => \&number_callback,
			  -rock => \$exists});
    # Select the mailbox for read-only operations.
    send_command("EXAMINE %s", $mbox);
    # If this mailbox has messages, fetch their size.
    if ($exists) {
	# The fetch callback will update the values for totalsize and
	# msgcount in the %usage hash.
	$client->addcallback({-trigger => 'FETCH', -flags => $cb_numbered,
			      -callback => \&fetch_callback,
			      -rock => \%usage});
	send_command("FETCH 1:* RFC822.SIZE");
    }
    return ($usage{totalsize}, $usage{msgcount});
}

# Subroutine to send a command to the IMAP server, and wait for the
# response; any defined callbacks for the response are invoked.
# If the server response indicates failure, we error out.
sub send_command(@) {
    my ($fmt, @args) = @_;
    printf("Sending: $fmt\n", @args) if $debug;
    my ($status, $text) = $client->send('', '', $fmt, @args);
    print "Response: status $status, text $text\n" if $debug;
    errorout "Premature end-of-file on IMAP connection to $host"
	if $status eq 'EOF';
    close_and_errorout "IMAP error from $host: $text"
	if $status ne 'OK';
}

# Callback to parse a LIST (or LSUB) response for a mailbox name
# and its attributes and delimiter.
#
# The response contains three elements, of the form:
#
#     (<attribute> ...) <delimiter> "<name>"
#
# For example:
#
#     (\HasChildren) "." "INBOX"
#
# The delimiter is either a quoted single character, e.g. ".",
# or NIL.
#
# The "-rock" hash element is a reference to a hash to which we add a
# key for the mailbox name, with its value being a hash with "attributes"
# and "delimiter" keys.  For a NIL delimiter, the returned value is undef,
# indicating a flat name.
sub list_callback(@) {
    my %cb = @_;
    print "In LIST callback: text $cb{-text}\n" if $debug;
    return unless $cb{-text} =~ m/^\(([^\)]*)\)\s+(?:"(.)"|NIL)\s+"(.+)"/o;
    ${$cb{-rock}}{$3} = {attributes => $1, delimiter => $2};
}

# Callback to parse the response from a "FETCH ... RFC822.SIZE"
# command for one message. The "-rock" element is a reference to a
# hash containing totalsize and msgcount keys, whose values will be
# updated accordingly.
sub fetch_callback(@) {
    my %cb = @_;
    print "In FETCH callback: text $cb{-text}\n" if $debug;
    if ($cb{-text} =~ /RFC822.SIZE\s+(\d+)/io) {
	${$cb{-rock}}{totalsize} += $1;
	${$cb{-rock}}{msgcount}++;
    }
}

# Callback to parse a numeric value.  The "-rock" element is a
# reference to the scalar in which to store the number.
sub number_callback(@) {
    my %cb = @_;
    print "In number callback: keyword $cb{-keyword}, number $cb{-msgno}\n"
	if $debug;
    ${$cb{-rock}} = $cb{-msgno};
}

# Close the connection to the IMAP server, and error out.
sub close_and_errorout($) {
    close_connection();
    errorout $_[0];
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
    print STDERR "mailusage: $_[0]\n";
    exit 1;
}
