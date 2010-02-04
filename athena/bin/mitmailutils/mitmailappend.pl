#!/usr/bin/perl -w

# $Id: mitmailappend.pl,v 1.1 2004-09-03 20:44:43 rbasch Exp $

# Append a message to an IMAP folder.

use strict;
use warnings FATAL => 'all';
use Cyrus::IMAP;
use Getopt::Long;

sub usage(;$);
sub read_file($);
sub send_command(@);
sub close_and_errorout($);
sub close_connection();
sub errorout($);

my $prog = $0;

sub usage(;$) {
    print STDERR "$prog: $_[0]\n" if ($_[0] && $_[0] ne "help");
    print STDERR <<EOF;
Usage: $prog [<options>]
  Options:
    --debug                turn on debugging
    --file=<path>          read message from <path> instead of standard input
    --help                 print this usage information
    --host=<name>          query host <name> instead of default POBOX server
    --mailbox=<name>       access mailbox <name> instead of INBOX
    --no-create            do not create the target mailbox automatically
EOF
    exit 1;
}

# Parse the command line arguments.
use vars qw($opt_debug $opt_file $opt_host $opt_mailbox $opt_no_create);

GetOptions("debug",
	   "file=s",
	   "help" => \&usage,
	   "host=s",
	   "mailbox=s",
	   "no-create") || usage;

usage unless @ARGV == 0;

$opt_mailbox = "INBOX" unless $opt_mailbox;

# By default we read the message from standard input.
$opt_file = "-" unless $opt_file;

my $username = $ENV{'ATHENA_USER'} || $ENV{'USER'} || getlogin || (getpwuid($<))[0] ||
    errorout "Cannot determine user name";

unless ($opt_host) {
    $opt_host = (gethostbyname("$username.mail.mit.edu"))[0];
    errorout "Cannot find Post Office server for $username" unless $opt_host;
}
errorout "Exchange accounts are not supported yet. Try http://owa.mit.edu/." if $opt_host =~ /EXCHANGE/;

# Read the entire message file into a string.
my $msg = read_file($opt_file);

# Convert LF -> CRLF if necessary.
unless ($msg =~ m/\r\n/os) {
    print "Converting LF to CRLF...\n" if $opt_debug;
    $msg =~ s/\n/\r\n/gos;
}

# Connect to the IMAP server, and authenticate.
my $client = Cyrus::IMAP->new($opt_host) ||
    errorout "Cannot connect to IMAP server on $opt_host";
$client->authenticate(-authz => $username, -maxssf => 0) ||
    close_and_errorout "Cannot authenticate to $opt_host";

# Try the APPEND command.  If the server returns an error,
# check for "TRYCREATE" in the response text; this is a hint that
# the target mailbox does not exist, but that it can be created.
my ($status, $text) = send_command("APPEND %s %s", $opt_mailbox, $msg);
if ($status ne 'OK') {
    if ($text =~ m/\bTRYCREATE\b/io) {
	close_and_errorout "Mailbox $opt_mailbox does not exist"
	    if $opt_no_create;
	print "Creating $opt_mailbox\n" if $opt_debug;
	# send_command will error out if the CREATE fails.
	send_command("CREATE %s", $opt_mailbox);
	($status, $text) = send_command("APPEND %s %s", $opt_mailbox, $msg);
    }
    close_and_errorout "IMAP error from $opt_host: $text"
	if ($status ne 'OK');
}

# We are done talking to the IMAP server, close down the connection.
close_connection();

exit 0;

# Read the given file's entire contents, returning it as a scalar.
sub read_file($) {
    my $file = $_[0];
    local $/ = undef;
    open(FILE, $file) || errorout "Cannot open $file: $!";
    my $contents = <FILE>;
    close(FILE);
    return $contents;
}

# Subroutine to send a command to the IMAP server, and wait for the
# response.  If the response status indicates failure (i.e. is not
# "OK"), we error out.
sub send_command(@) {
    my ($fmt, @args) = @_;
    printf("Sending: $fmt\n", @args) if $opt_debug;
    my ($status, $text) = $client->send('', '', $fmt, @args);
    errorout "Premature end-of-file on IMAP connection to $opt_host"
	if $status eq 'EOF';
    print "Response: status $status, text $text\n" if $opt_debug;
    return ($status, $text) if wantarray;
    close_and_errorout "IMAP error from $opt_host: $text"
	if $status ne 'OK';
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
