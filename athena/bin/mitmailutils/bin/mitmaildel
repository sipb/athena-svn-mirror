#!/usr/bin/perl -w

# $Id: mitmaildel.pl,v 1.3 2004-07-29 19:11:52 rbasch Exp $

# Delete (or undelete) messages in an IMAP folder.

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
my $undelete = ($prog =~ m/undel$/o);

sub usage(;$) {
    print STDERR "$prog: $_[0]\n" if ($_[0] && $_[0] ne "help");
    print STDERR <<EOF;
Usage: $prog [<options>] <message-id> ...
  Options:
    --by-uid               specify message UIDs instead of sequence numbers
    --debug                turn on debugging
    --expunge              expunge all deleted messages from mailbox
    --help                 print this usage information
    --host=<name>          query host <name> instead of default POBOX server
    --mailbox=<name>       examine mailbox <name> instead of INBOX
    --silent               suppress acknowledgement output
EOF
    exit 1;
}

# Parse the command line arguments.
use vars qw($opt_by_uid $opt_debug $opt_expunge $opt_host
	    $opt_mailbox $opt_silent);

GetOptions("by-uid",
	   "debug",
	   "expunge",
	   "help" => \&usage,
	   "host=s",
	   "mailbox=s",
	   "silent") || usage;

usage "Please specify a message number" if @ARGV == 0;

# Check the validity of message ID arguments.
# The ID can be a number or '*', and we accept a range specification,
# of the form 'n:m'.
foreach (@ARGV) {
    errorout "Invalid message specification $_"
	unless (m/^(?:\d+|\*)(?::(?:\d+|\*))?$/o);
}

$opt_mailbox = 'INBOX' unless $opt_mailbox;

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

# Select the mailbox (for read-write access).  Store the value of the
# EXISTS (i.e. highest existing message sequence number) data item,
# and, if returned, the UIDNEXT (next UID to be assigned) value.
my $cb_numbered = Cyrus::IMAP::CALLBACK_NUMBERED;
my $maxseq = 0;
my $uidnext = 0;
$client->addcallback({-trigger => 'EXISTS', -flags => $cb_numbered,
		      -callback => \&number_callback,
		      -rock => \$maxseq});
$client->addcallback({-trigger => 'OK',
		      -callback => sub {
			  my %cb = @_;
			  print "In OK callback: text $cb{-text}\n"
			      if $opt_debug;
			  return unless ($cb{-text} =~ m/UIDNEXT\s+(\d+)/io);
			  $uidnext = $1;
		      }});
send_command "SELECT \"$opt_mailbox\"";

# If we're operating on UIDs, and did not get the UIDNEXT value above,
# use the STATUS command to get it explicitly.
if ($opt_by_uid && !$uidnext) {
    $client->addcallback({-trigger => 'STATUS',
			  -callback => sub {
			      my %cb = @_;
			      print "In STATUS callback: text $cb{-text}\n"
				  if $opt_debug;
			      return
				  unless ($cb{-text} =~ m/UIDNEXT\s+(\d+)/io);
			      $uidnext = $1;
			  }});
    send_command "STATUS \"$opt_mailbox\" (UIDNEXT)";
}

# Note that the STORE command returns success even when the given
# message ID does not exist.  So the most feasible way to determine
# whether the (un)delete was successful for a message is to see if the
# FETCH callback (invoked during the server response to the STORE
# command) was invoked for the message, and whether the \Deleted flag
# was returned.  We thus initialize a hash whose keys are the
# individual message IDs given; the FETCH callback will remove the key
# from the hash when it detects that the message flags have been set
# as desired.  Any keys remaining in the hash upon completion will
# indicate IDs whose flags could not be modified (presumably because
# the message does not exist).
my %unchanged = ();
my $expect = ($undelete ? "undeleted" : "deleted");
my $exitcode = 0;
my $store_cmd = ($opt_by_uid ? 'UID STORE' : 'STORE');
my $store_item = ($undelete ? '-FLAGS' : '+FLAGS');
$client->addcallback({-trigger => 'FETCH', -flags => $cb_numbered,
		      -callback => \&fetch_callback,
		      -rock => \%unchanged});
foreach (@ARGV) {
    if ($opt_by_uid) {
	# When operating on UIDs, the message numbers in a range are
	# not necessarily sequential, so we don't detect unchanged
	# messages in the range.  We can handle '*', though, as that
	# is simply one less than the next UID to be assigned.
	$_ = ($uidnext - 1) if ($_ eq '*' && $uidnext);
	%unchanged = ($_ => 1) if (/^\d+$/);
    } else {
	s/\*/$maxseq/o;
	m/^(\d+)(?::(\d+))?$/o;
	if ($2) {
	    %unchanged = map { $_ => 1 } ($1 < $2 ? $1 .. $2 : $2 .. $1);
	} else {
	    %unchanged = ($1 => 1);
	}
    }
    send_command "$store_cmd $_ $store_item (\\Deleted)";
    foreach my $msg (sort { $a <=> $b } keys %unchanged) {
	print STDERR "$prog: Could not " .
	    ($undelete ? "un" : "") . "delete $msg\n";
	$exitcode = 1;
    }
}

# Expunge the mailbox if so desired, unless there was an error marking
# any message.
send_command "CLOSE" if ($opt_expunge && ($exitcode == 0));

# We are done talking to the IMAP server, close down the connection.
close_connection();

exit $exitcode;

# Subroutine to send a command to the IMAP server, and wait for the
# response; any defined callbacks for the response are invoked.
# If the server response indicates failure, we error out.
sub send_command($) {
    print "Sending: $_[0]\n" if $opt_debug;
    my ($status, $text) = $client->send('', '', $_[0]);
    print "Response: status $status, text $text\n" if $opt_debug;
    errorout "Premature end-of-file on IMAP connection to $opt_host"
	if $status eq 'EOF';
    close_and_errorout "IMAP error from $opt_host: $text"
	if $status ne 'OK';
}

# Callback subroutine to parse the FETCH response from a STORE command.
# This callback will be invoked for each message.  The "-text" hash
# element contains the text returned by the server.  The "-rock"
# element is a reference to the hash containing the message IDs of
# interest; we delete the appropriate key (sequence number or UID)
# from this hash.
sub fetch_callback(@) {
    my %cb = @_;
    my ($number, $flags);
    print "In FETCH callback: msgno $cb{-msgno} text $cb{-text}\n"
	if $opt_debug;
    $number = $cb{-msgno};
    foreach (split /\r\n/, $cb{-text}) {
	$number = $1 if /UID\s+(\d+)/io;
	$flags = $1 if /FLAGS\s+\(([^\)]*)\)/io;
    }
    delete ${$cb{-rock}}{$number};
    my $state = ($flags =~ /\\Deleted\b/io ? "deleted" : "undeleted");
    # Warn if the returned state is not what was expected.
    if ($state ne $expect) {
	print STDERR "$prog: Warning: Message $number $state\n";
    } else {
	# Display an acknowledgement of success if so desired.
	print(($opt_by_uid ? "UID" : "Message") . " $number $state\n")
	    unless $opt_silent;
    }
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
