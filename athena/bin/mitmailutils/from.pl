#!/usr/bin/perl -w

# $Id: from.pl,v 1.6 2004-07-29 19:11:51 rbasch Exp $

# This is an implementation of the Athena "from" utility using the
# Perl interface to the Cyrus imclient IMAP library.

use strict;
use warnings FATAL => 'all';
use Cyrus::IMAP;
use Getopt::Std;

sub usage(;$);
sub send_command($);
sub search_callback(@);
sub fetch_callback(@);
sub number_callback(@);
sub make_msgspecs(@);
sub close_connection();
sub get_localmail();
sub get_terminal_width();
sub errorout($);

sub usage(;$) {
    print STDERR "from: $_[0]\n" if $_[0];
    print STDERR <<EOF;
Usage: from [<options>] [<user>]
  Options:
    -N            check only NEW messages in IMAP mailbox (default is UNSEEN)
    -A            check all messages in IMAP mailbox
    -m <mailbox>  check <mailbox> (default is INBOX)
    -h <host>     query <host> instead of default post office server
    -s <sender>   show mail from <sender> only
    -n            be silent when there is no mail
    -r            include Subject header
    -v            include To, Date, and Subject headers
    -t            display message totals only
    -p            check post office server only
    -u            check local mail only
    -d            turn on debugging
EOF
    exit 1;
}

# By default, we search for UNSEEN messages.  If the user specifies -N,
# we search for NEW messages (NEW is equivalent to "UNSEEN RECENT").
# If -A is given, we check ALL messages.
my $search_key = "unseen";

# Parse the command line arguments.
my %opts;
getopts('Adh:m:Nnprs:tuv', \%opts) || usage;
my $have_user = 0;
my $username = shift @ARGV;
if ($username) {
    $have_user = 1;
} else {
    $username = $ENV{"ATHENA_USER"} || $ENV{'USER'} || getlogin || (getpwuid($<))[0] ||
	errorout "Cannot determine user name";
}
usage "Too many arguments" if @ARGV > 0;
my $checkall = $opts{'A'} && ($search_key = "all");
my $debug = $opts{'d'};
my $host = $opts{'h'} || (gethostbyname("$username.mail.mit.edu"))[0];
errorout "Cannot find Post Office server for $username" unless $host;
errorout "Exchange accounts are not supported yet. Try http://owa.mit.edu/." if $host =~ /EXCHANGE/;
my $mbox = $opts{'m'} || "INBOX";
my $quiet = $opts{'n'};
my $checknew = $opts{'N'} && ($search_key = "new");
my $imaponly = $opts{'p'};
my $report = $opts{'r'};
my $sender = $opts{'s'};
my $totals_only = $opts{'t'};
my $localonly = $opts{'u'};
my $verbose = $opts{'v'};
usage "Cannot specify both -A and -N" if $checkall && $checknew;
usage "Cannot specify both -p and -u" if $imaponly && $localonly;
usage "Cannot specify both -r and -t" if $report && $totals_only;
usage "Cannot specify both -t and -v" if $totals_only && $verbose;

# Check local mail first.
my $localcount = 0;
$localcount = get_localmail() unless $imaponly;

exit 0 if $localonly;

# Check mail on the IMAP server.
# Connect to the server, and authenticate.
my $client = Cyrus::IMAP->new($host) ||
    errorout "Cannot connect to IMAP server on $host";
unless ($client->authenticate(-authz => $username, -maxssf => 0)) {
    close_connection();
    errorout "Cannot authenticate to $host";
}

# Examine the mailbox.  This gives the numbers of existing and recent
# messages, as well as selecting the mailbox for read-only access.
my $recentmsgcount = -1;
my $totalmsgcount = -1;
my @msgids = ();
my @pomsgs = ();
my $cb_numbered = Cyrus::IMAP::CALLBACK_NUMBERED;
$client->addcallback({-trigger => 'EXISTS', -flags => $cb_numbered,
		      -callback => \&number_callback,
		      -rock => \$totalmsgcount});
$client->addcallback({-trigger => 'RECENT', -flags => $cb_numbered,
		      -callback => \&number_callback,
		      -rock => \$recentmsgcount});
send_command "EXAMINE \"$mbox\"";

if ($totalmsgcount && !($checknew && !$recentmsgcount)) {
    # Search the mailbox to obtain the message UID's.
    $client->addcallback({-trigger => 'SEARCH',
			  -callback => \&search_callback,
			  -rock => \@msgids});
    send_command "UID SEARCH $search_key" . ($sender ? " FROM $sender" : "");

    # If there are messages of interest, fetch their size, and any desired
    # headers.
    if (@msgids > 0) {
	my $fetch = "RFC822.SIZE";
	$fetch .= " BODY.PEEK[HEADER.FIELDS (FROM TO SUBJECT DATE)]"
	    unless $totals_only;
	$client->addcallback({-trigger => 'FETCH', -flags => $cb_numbered,
			      -callback => \&fetch_callback,
			      -rock => \@pomsgs});
	foreach (make_msgspecs(@msgids)) {
	    send_command "UID FETCH $_ ($fetch)";
	}
    }
}
my $msgcount = @pomsgs;

# We are done talking to the IMAP server, close down the connection.
close_connection();

my $msg;

# Print out the summary line if appropriate.
if (($verbose || $totals_only) && ($msgcount > 0 || !$quiet)) {
    my $totalsize = 0;
    for $msg (@pomsgs) {
	$totalsize += $msg->{size};
    }

    print $have_user ? "$username has " : "You have ";
    if ($msgcount > 0) {
	print "$msgcount " .
	    ($checkall ? "total" : $search_key) . " message" .
	    ($msgcount > 1 ? 's' : '') .
	    " ($totalsize bytes)" .
	    ($checkall ? "" : ", $totalmsgcount total,");
    } else {
	print "no" .
	    ($checkall || $totalmsgcount == 0 ? "" : " $search_key") .
	    " messages";
    }
    print " in $mbox on $host" .
	($verbose && $msgcount > 0 ? ':' : '.') . "\n";
}

# Show the desired headers if appropriate.
if (!$totals_only && $msgcount > 0) {
    my $subject_width;

    print ucfirst(($checkall ? "" : "$search_key ") .
		  "mail in IMAP folder $mbox:\n") unless $verbose || $imaponly;
    if ($report) {
	my $tty_width = get_terminal_width();
	$subject_width = ($tty_width > 33 ? $tty_width - 33 : 0);
    }
    for $msg (@pomsgs) {
	if ($report) {
	    printf("%-30.30s ", $msg->{from});
	    print substr($msg->{subject}, 0, $subject_width)
		if $msg->{subject} && $subject_width;
	    print "\n";
	} else {
	    if ($verbose) {
		print "\n";
		print "To: $msg->{to}\n" if $msg->{to};
		print "Subject: $msg->{subject}\n" if $msg->{subject};
		print "Date: $msg->{date}\n" if $msg->{date};
	    }
	    print "From: $msg->{from}\n";
	}
    }
}

# Subroutine to send a command to the IMAP server, and wait for the
# response; any defined callbacks for the response are invoked.
# If the server response indicates failure, we error out.
sub send_command($) {
    print "Sending: $_[0]\n" if $debug;
    my ($status, $text) = $client->send('', '', $_[0]);
    print "Response: status $status, text $text\n" if $debug;
    errorout "Premature end-of-file on IMAP connection to $host"
	if $status eq 'EOF';
    if ($status ne 'OK') {
	close_connection();
	errorout "IMAP error for $mbox on $host: $text" 
    }
}

# Callback subroutine to parse the response from a SEARCH command.
# The "-text" hash element contains the returned message UIDs,
# separated by a space.  The "-rock" element is a reference to the
# array in which to store the UIDs.
sub search_callback(@) {
    my %cb = @_;
    print "In SEARCH callback: text $cb{-text}\n" if $debug;
    @{$cb{-rock}} = split(/\s/, $cb{-text});
}

# Callback subroutine to parse the response from a FETCH command.
# This callback will be invoked for each message.  The "-text" hash
# element contains the text returned by the server.  The "-rock"
# element is a reference to the array in which to push a hash of the
# various message data items.
sub fetch_callback(@) {
    my %cb = @_;
    my ($from, $to, $subject, $date) = '';
    my $size = 0;
    print "In FETCH callback: text $cb{-text}\n" if $debug;
    for (split /\r\n/, $cb{-text}) {
	$size = $1 if /RFC822.SIZE\s+(\d+)/io;
	$from = $_ if s/^From:\s*//io;
	$to = $_ if s/^To:\s*//io;
	$subject = $_ if s/^Subject:\s*//io;
	$date = $_ if s/^Date:\s*//io;
	
    }
    push @{$cb{-rock}}, {from => $from, to => $to, subject => $subject,
			 date => $date, size => $size};
}

# Callback subroutine to parse a numeric value.  The "-rock" hash
# element is a reference to the scalar in which to store the number.
sub number_callback(@) {
    my %cb = @_;
    print "In number callback: keyword $cb{-keyword}, number $cb{-msgno}\n"
	if $debug;
    ${$cb{-rock}} = $cb{-msgno};
}

# This subroutine takes a list of IMAP message UID numbers, and constructs
# single-string representations of the set, collapsing sequences into
# ranges where possible.  In order to avoid constructing a specification
# which is too long to be processed, the result is returned as an array
# of manageably-sized specification strings, currently limited to about
# 200 characters each.
sub make_msgspecs(@) {
    return '' if @_ == 0;
    my @specs = ();
    my $first = shift(@_);
    my $last = $first;
    my $spec = $first;
    foreach (@_) {
	if ($_ != $last + 1) {
	    # This UID is not in sequence with the previous element.
	    # If that marks the end of a range, complete it.
	    $spec .= ":$last" if ($first != $last);
	    # Begin a new sequence.  Create another spec string if the
	    # current one is getting long.
	    if (length($spec) > 200) {
		push @specs, $spec;
		$spec = $_;
	    } else {
		$spec .= ",$_";
	    }
	    $first = $_;
	}
	$last = $_;
    }
    # Complete the final range if necessary.
    $spec .= ":$last" if ($first != $last);
    push @specs, $spec if ($spec);
    return @specs;
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

# Get mail from the local ("Unix") mail drop.
# Returns the number of messages found.
sub get_localmail() {
    my $maildrop = $ENV{'MAILDROP'} || "/var/spool/mail/$username";
    # Open the mail drop.
    unless (open MAIL, $maildrop) {
	errorout "Cannot open maildrop $maildrop" if $localonly;
	return 0;
    }
    my $count = 0;
    my $from = '';
    while (<MAIL>) {
	chop;
	if ($_ eq '' && $from) {
	    print "$from\n" unless $totals_only;
	    $count++;
	    $from = '';
	}
	elsif (/^From\s+([^\s\t]*)/o) {
	    next if $sender && $1 !~ /$sender/io;
	    print "Local mail:\n"
		unless ($count > 0 || $totals_only || $localonly);
	    $from = $_;
	}
    }
    if ($from) {
	print "$from\n" unless $totals_only;
	$count++;
    }
    if ($totals_only && $count) {
	my $size = -s MAIL;
	print $have_user ? "$username has" : "You have";
	print " $count local messages ($size bytes).\n";
    }
    close(MAIL);
    return $count;
}    

sub get_terminal_width() {
    my $columns = 80;
    open STTY, "stty -a |" or return $columns;
    while (<STTY>) {
	if (/columns[\s=]+(\d+);/o) {
	    $columns = $1;
	    last;
	}
    }
    close STTY;
    return $columns;
}

sub errorout($) {
    print STDERR "from: $_[0]\n";
    exit 1;
}
