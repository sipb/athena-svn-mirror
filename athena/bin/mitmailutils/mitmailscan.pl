#!/usr/bin/perl -w

# $Id: mitmailscan.pl,v 1.5 2004-10-26 20:56:30 rbasch Exp $

# Scan messages in an IMAP folder.

use strict;
use warnings FATAL => 'all';
use Cyrus::IMAP;
use Getopt::Long;

sub usage(;$);
sub send_command(@);
sub search_callback(@);
sub fetch_callback(@);
sub number_callback(@);
sub make_msgspecs(@);
sub parse_date_opt($$);
sub month_nametonum($);
sub get_terminal_width();
sub close_and_errorout($);
sub close_connection();
sub errorout($);

my $prog = $0;

sub usage(;$) {
    print STDERR "$prog: $_[0]\n" if ($_[0] && $_[0] ne "help");
    print STDERR <<EOF;
Usage: $prog [<options>] [<message-id> ...]
  Options:
    --answered             show messages which have been marked as answered
    --before=<dd-Mon-yyyy> show messages sent before given date
    --by-uid               show message's unique ID instead of sequence number
    --debug                turn on debugging
    --deleted              show messages which have been marked as deleted
    --from=<sender>        show messages with <sender> in From field
    --help                 print this usage information
    --host=<name>          query host <name> instead of default POBOX server
    --id-only              output message IDs only
    --key=<string>         same as --search-key=<string>
    --larger=<n>           show messages whose size is greater than <n> bytes
    --mailbox=<name>       examine mailbox <name> instead of INBOX
    --new                  show new messages (same as "--recent --unseen")
    --old                  show messages not marked as recent
    --on=<dd-Mon-yyyy>     show messages sent on given date
    --recent               show recent messages
    --search-key=<string>  specify explicit IMAP search key (see RFC 2060)
    --seen                 show messages with the SEEN flag set
    --since=<dd-Mon-yyyy>  show messages sent since given date
    --smaller=<n>          show messages whose size is less than <n> bytes
    --subject=<string>     show messages with <string> in Subject field
    --text=<string>        show messages with <string> in header or body
    --to=<recipient>       show messages with <recipient> in To field
    --unanswered           show messages which have not been marked as answered
    --undeleted            show messages which have not been marked as deleted
    --unseen               show messages which do not have the SEEN flag set
EOF
    exit 1;
}

# Parse the command line.
use vars qw($opt_answered $opt_before $opt_by_uid $opt_debug
	    $opt_deleted $opt_from $opt_host $opt_id_only
	    $opt_larger $opt_mailbox $opt_new $opt_old $opt_on
	    $opt_recent $opt_search_key $opt_seen $opt_since
	    $opt_smaller $opt_subject $opt_text $opt_to
	    $opt_unanswered $opt_undeleted $opt_unseen);

# Map month names to numbers.
my %monthnum = (
    Jan => 1, Feb => 2, Mar => 3, Apr =>  4, May =>  5, Jun =>  6,
    Jul => 7, Aug => 8, Sep => 9, Oct => 10, Nov => 11, Dec => 12,
);

# Parse and validate the given date option string.
# The first argument is the option name, and the second is the value.
# We error out if the value is not of the form dd-Mon-yyyy.  Otherwise,
# we set the value of the corresponding $opt_<name> variable.
sub parse_date_opt($$) {
    # Disable strict refs locally so we can use a symbolic reference
    # to set the appropriate $opt_<name> variable below.
    no strict 'refs';
    my ($name, $value) = @_;
    usage "\"$value\" is not a valid date (dd-Mon-yyyy expected)"
	unless (($value =~ m|^\d{1,2}-([A-Za-z]{3})-\d{4}$|o) &&
		month_nametonum($1));
    ${"opt_" . $name} = $value;
}

GetOptions("answered",
	   "before=s" => \&parse_date_opt,
	   "by-uid",
	   "debug",
	   "deleted",
	   "from=s",
	   "help" => \&usage,
	   "host=s",
	   "id-only",
	   "larger=i",
	   "mailbox=s",
	   "new",
	   "old",
	   "on=s" => \&parse_date_opt,
	   "recent",
	   "search-key|key=s",
	   "seen",
	   "since=s" => \&parse_date_opt,
	   "smaller=i",
	   "subject=s",
	   "text=s",
	   "to=s",
	   "unanswered",
	   "undeleted",
	   "unseen") || usage;

my $msgset = '';
foreach (@ARGV) {
    errorout "Invalid message specification $_"
	unless (m/^(?:\d+|\*)(?::(?:\d+|\*))?$/o);
    $msgset .= ',' if $msgset;
    $msgset .= "$_";
}

usage "Cannot specify both --new and --old" if ($opt_new && $opt_old);
usage "Cannot specify both --recent and --old" if ($opt_recent && $opt_old);
usage "Cannot specify both --seen and --unseen" if ($opt_seen && $opt_unseen);
usage "Cannot specify both --deleted and --undeleted"
    if ($opt_deleted && $opt_undeleted);

$opt_mailbox = 'INBOX' unless $opt_mailbox;
$opt_search_key = 'ALL' unless $opt_search_key;

my $username = $ENV{'ATHENA_USER'} || $ENV{'USER'} || getlogin || (getpwuid($<))[0] ||
    errorout "Cannot determine user name";

unless ($opt_host) {
    $opt_host = (gethostbyname("$username.mail.mit.edu"))[0];
    errorout "Cannot find Post Office server for $username" unless $opt_host;
}
errorout "Exchange accounts are not supported yet. Try http://owa.mit.edu/." if $opt_host =~ /EXCHANGE/;

# Build the search key based on the specified command line options.
if ($msgset) {
    $opt_search_key .= " UID" if $opt_by_uid;
    $opt_search_key .= " $msgset";
}
$opt_search_key .= " FROM $opt_from" if $opt_from;
$opt_search_key .= " SUBJECT $opt_subject" if $opt_subject;
$opt_search_key .= " TO $opt_to" if $opt_to;
$opt_search_key .= " TEXT $opt_text" if $opt_text;
$opt_search_key .= " SENTBEFORE $opt_before" if $opt_before;
$opt_search_key .= " SENTSINCE $opt_since" if $opt_since;
$opt_search_key .= " SENTON $opt_on" if $opt_on;
$opt_search_key .= " LARGER $opt_larger" if defined $opt_larger;
$opt_search_key .= " SMALLER $opt_smaller" if defined $opt_smaller;
$opt_search_key .= " NEW" if $opt_new;
$opt_search_key .= " OLD" if $opt_old;
$opt_search_key .= " RECENT" if $opt_recent;
$opt_search_key .= " SEEN" if $opt_seen;
$opt_search_key .= " UNSEEN" if $opt_unseen;
$opt_search_key .= " ANSWERED" if $opt_answered;
$opt_search_key .= " UNANSWERED" if $opt_unanswered;
$opt_search_key .= " DELETED" if $opt_deleted;
$opt_search_key .= " UNDELETED" if $opt_undeleted;

# Connect to the IMAP server, and authenticate.
my $client = Cyrus::IMAP->new($opt_host) ||
    errorout "Cannot connect to IMAP server on $opt_host";
$client->authenticate(-authz => $username, -maxssf => 0) ||
    close_and_errorout "Cannot authenticate to $opt_host";

# Examine the mailbox.  This gives the numbers of existing messages,
# as well as selecting the mailbox for read-only access.
my $totalmsgcount = -1;
my @msgids = ();
my @pomsgs = ();
my $cb_numbered = Cyrus::IMAP::CALLBACK_NUMBERED;
$client->addcallback({-trigger => 'EXISTS', -flags => $cb_numbered,
		      -callback => \&number_callback,
		      -rock => \$totalmsgcount});
send_command("EXAMINE %s", $opt_mailbox);

if ($totalmsgcount) {
    # Search the mailbox to obtain the desired message numbers.
    $client->addcallback({-trigger => 'SEARCH',
			  -callback => \&search_callback,
			  -rock => \@msgids});
    send_command("UID SEARCH %a", $opt_search_key);

    # If there are messages of interest, fetch them.
    if (@msgids > 0) {
	#@msgids = sort {$a <=> $b} @msgids;
	my $fetch = "FLAGS BODY.PEEK[HEADER.FIELDS (FROM SUBJECT DATE)]";
	$client->addcallback({-trigger => 'FETCH', -flags => $cb_numbered,
			      -callback => \&fetch_callback,
			      -rock => \@pomsgs});
	foreach (make_msgspecs(@msgids)) {
	    send_command("UID FETCH %a (%a)", $_, $fetch);
	}
    }
}

# We are done talking to the IMAP server; close down the connection.
close_connection();

# Quit now if there are no messages to display.
exit 0 unless @pomsgs;

# Display the message(s), sorted by the message ID (sequence number or UID).
my $msg;
my $id_key = ($opt_by_uid ? 'uid' : 'number');
@pomsgs = sort { $a->{$id_key} <=> $b->{$id_key} } @pomsgs;
if ($opt_id_only) {
    # We are only outputting the message ID.
    foreach $msg (@pomsgs) {
	print "$msg->{$id_key} ";
    }
    print "\n";
} else {
    # Here for the standard formatted output.
    my $id_width = length $pomsgs[$#pomsgs]->{$id_key};
    my $tty_width = get_terminal_width();
    my $from_width = 20;

    # Calculate the line width remaining for the message subject.
    # Allow for the ID, the flag character, date (mm/dd), and "From"
    # field widths, plus 2 spaces between the fields.
    my $subject_width = $tty_width - 1 -
	($id_width + 1 + 2 + 5 + 2 + $from_width + 2);
    $subject_width = 0 if ($subject_width < 0);

    # Construct the format string.
    my $format = "%${id_width}s%s  %02d/%02d  %-${from_width}.${from_width}s" .
		     "  %-${subject_width}.${subject_width}s\n";

    # Loop to display each message.
    foreach $msg (@pomsgs) {
	my ($month, $day);
	my $flag;

	# Parse a date of the form "DD Mon ...", with an optional leading
	# "Day, ", or of the form "MM/DD/YY...".
	if ($msg->{date} =~ m|^\s*(?:...,\s+)?(\d{1,2})\s+([A-Za-z]{3})|o) {
	    # Parsed "DD Mon ...".
	    ($month, $day) = (month_nametonum($2), $1);
	} elsif ($msg->{date} =~ m|^\s*(\d{1,2})/(\d{1,2})/\d+|o) {
	    # Parsed "MM/DD/YY...".
	    ($month, $day) = ($1, $2);
	} else {
	    # Unrecognized date format.
	    ($month, $day) = (0, 0);
	}

	# Strip double quotes from the "From" header.
	my $from = $msg->{from} || '';
	$from =~ tr/"//d;

	# Flag a deleted or unseen message.
	if ($msg->{flags} =~ /\\Deleted\b/io) {
	    $flag = 'D';
	} elsif ($msg->{flags} !~ /\\Seen\b/io) {
	    $flag = 'U';
	} else {
	    $flag = ' ';
	}

	# Display the line.
        printf($format, $msg->{$id_key}, $flag, int($month), int($day),
	       $from, $msg->{subject} ? $msg->{subject} : '');
    }
}

# Subroutine to send a command to the IMAP server, and wait for the
# response; any defined callbacks for the response are invoked.
# If the server response indicates failure, we error out.
sub send_command(@) {
    my ($fmt, @args) = @_;
    if ($opt_debug) {
	local $" = ', ';
	print "Send($fmt, @args) ...\n";
    }
    my ($status, $text) = $client->send('', '', $fmt, @args);
    print "Response: status $status, text $text\n" if $opt_debug;
    errorout "Premature end-of-file on IMAP connection to $opt_host"
	if $status eq 'EOF';
    close_and_errorout "IMAP error from $opt_host: $text"
	if $status ne 'OK';
}

# Callback subroutine to parse the response from a SEARCH command.
# The "-text" hash element contains the returned message numbers,
# separated by a space.  The "-rock" element is a reference to the
# array in which to store the message numbers.
sub search_callback(@) {
    my %cb = @_;
    print "In SEARCH callback: text $cb{-text}\n" if $opt_debug;
    @{$cb{-rock}} = split(/\s/, $cb{-text});
}

# Callback subroutine to parse the response from a FETCH command.
# This callback will be invoked for each message.  The "-text" hash
# element contains the text returned by the server.  The "-rock"
# element is a reference to the array in which to push a hash of the
# various message data items.
sub fetch_callback(@) {
    my %cb = @_;
    my ($number, $uid, $flags, $from, $to, $subject, $date);
    print "In FETCH callback: msgno $cb{-msgno} text $cb{-text}\n"
	if $opt_debug;
    $number = $cb{-msgno};
    my @response_lines = split /\r\n/, $cb{-text};
    $_ = shift @response_lines;
    $uid = $1 if /\bUID\s+(\d+)/io;
    $flags = $1 if /\bFLAGS\s+\(([^\)]*)\)/io;
    foreach (@response_lines) {
	$from = $_ if s/^From:\s*//io;
	$to = $_ if s/^To:\s*//io;
	$subject = $_ if s/^Subject:\s*//io;
	$date = $_ if s/^Date:\s*//io;
    }
    push @{$cb{-rock}}, {number => $number, uid => $uid, flags => $flags,
			 from => $from, to => $to, subject => $subject,
			 date => $date}
	if $number;
}

# Callback subroutine to parse a numeric value.  The "-rock" hash
# element is a reference to the scalar in which to store the number.
sub number_callback(@) {
    my %cb = @_;
    print "In number callback: keyword $cb{-keyword}, number $cb{-msgno}\n"
	if $opt_debug;
    ${$cb{-rock}} = $cb{-msgno};
}

# This subroutine takes a list of IMAP message sequence or UID
# numbers, and constructs single-string representations of the set,
# collapsing sequences into ranges where possible.  In order to avoid
# constructing a specification which is too long to be processed, the
# result is returned as an array of manageably-sized specification
# strings, currently limited to about 200 characters each.
sub make_msgspecs(@) {
    return '' if @_ == 0;
    my @specs = ();
    my $first = shift(@_);
    my $last = $first;
    my $spec = $first;
    foreach (@_) {
	if ($_ != $last + 1) {
	    # This number is not in sequence with the previous element.
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

# Convert month names to numbers.
sub month_nametonum($) {
    my $num = $monthnum{ucfirst(lc($_[0]))};
    return ($num ? $num : 0);
}

# Return the terminal line width.  Unfortunately, the only feasible
# way to get the width is to parse stty output.
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
