#!/usr/bin/perl -w

# $Id: mitmailshow.pl,v 1.3 2004-07-29 19:11:54 rbasch Exp $

# Show messages in an IMAP folder.

use strict;
use warnings FATAL => 'all';
use Cyrus::IMAP;
use Getopt::Long;

sub usage(;$);
sub send_command($);
sub fetch_callback(@);
sub number_callback(@);
sub close_and_errorout($);
sub close_connection();
sub errorout($);

my $prog = $0;

sub usage(;$) {
    print STDERR "$prog: $_[0]\n" if ($_[0] && $_[0] ne "help");
    print STDERR <<EOF;
Usage: $prog [<options>] <message-ID> [...]
  Options:
    --by-uid               specify message's unique ID, not sequence number
    --debug                turn on debugging
    --help                 print this usage information
    --host=<name>          query host <name> instead of default POBOX server
    --mailbox=<name>       access mailbox <name> instead of INBOX
    --no-mark              do not mark the message as having been seen
    --peek                 same as --no-mark
EOF
    exit 1;
}

# Parse the command line arguments.
use vars qw($opt_by_uid $opt_debug $opt_expunge $opt_host $opt_mailbox
	    $opt_no_mark);

GetOptions("by-uid",
	   "debug",
	   "help" => \&usage,
	   "host=s",
	   "mailbox=s",
	   "no-mark|peek") || usage;

usage "Please specify a message ID" if @ARGV == 0;

# Check the validity of message ID arguments.
# The ID can be a number or '*', and we accept a range specification,
# of the form 'n:m'.
foreach (@ARGV) {
    errorout "Invalid message specification $_"
	unless (m/^(?:\d+|\*)(?::(?:\d+|\*))?$/o);
}

$opt_mailbox = "INBOX" unless $opt_mailbox;

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
$client->authenticate(-authz => $username, -maxssf => 0) ||
    close_and_errorout "Cannot authenticate to $opt_host";

# Select (or examine, if in no-mark mode) the mailbox.
my $select_cmd = ($opt_no_mark ? 'EXAMINE' : 'SELECT');
send_command "$select_cmd \"$opt_mailbox\"";

# Fetch the messages.  The message body will be displayed by the
# fetch_callback subroutine.
my $cb_numbered = Cyrus::IMAP::CALLBACK_NUMBERED;
my $fetch_cmd = ($opt_by_uid ? 'UID FETCH' : 'FETCH');
my $fetch = ($opt_no_mark ? 'BODY.PEEK[]' : 'BODY[]');
$client->addcallback({-trigger => 'FETCH', -flags => $cb_numbered,
		      -callback => \&fetch_callback});
my $exitcode = 0;
foreach (@ARGV) {
    my ($status, $text) = send_command "$fetch_cmd $_ ($fetch)";
    if ($status ne 'OK') {
	print STDERR "$prog: Cannot fetch $_: $text\n";
	$exitcode = 1;
    }
}

# We are done talking to the IMAP server, close down the connection.
close_connection();

exit $exitcode;

# Subroutine to send a command to the IMAP server, and wait for the
# response; any defined callbacks for the response are invoked.
# If called in list context, the response status and text strings
# are returned; otherwise, if the status indicates failure (i.e. is
# not "OK"), we error out.
sub send_command($) {
    print "Sending: $_[0]\n" if $opt_debug;
    my ($status, $text) = $client->send('', '', $_[0]);
    print "Response: status $status, text $text\n" if $opt_debug;
    errorout "Premature end-of-file on IMAP connection to $opt_host"
	if $status eq 'EOF';
    return ($status, $text) if wantarray;
    close_and_errorout "IMAP error from $opt_host: $text"
	if $status ne 'OK';
}

# Callback subroutine to parse the response from a FETCH command,
# and display the message body on standard output.  The body text
# is converted to Unix-style end-of-line, but is otherwise unfiltered.
# This callback will be invoked for each message.  The "-text" hash
# element contains the text returned by the server.
sub fetch_callback(@) {
    my %cb = @_;
    print "In FETCH callback: msgno $cb{-msgno} text $cb{-text}\n"
	if $opt_debug;
    $_ = $cb{-text};
    # Extract the body size, and strip off the response up to the body.
    my $size = $1 if s/.*?BODY\[\] \{(\d+)\}\r\n//ios;
    return unless $size;
    # Extract the body text.
    $_ = substr($_, 0, $size);
    # Convert to Unix-style EOL.
    s/\r\n/\n/gos;
    print $_;
}

# Callback subroutine to parse a numeric value.  The "-rock" hash
# element is a reference to the scalar in which to store the number.
sub number_callback(@) {
    my %cb = @_;
    print "In number callback: keyword $cb{-keyword}, number $cb{-msgno}\n"
	if $opt_debug;
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
    print STDERR "$prog: $_[0]\n";
    exit 1;
}
