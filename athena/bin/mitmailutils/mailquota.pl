#!/usr/bin/perl -w

# $Id: mailquota.pl,v 1.4 2004-07-29 19:11:52 rbasch Exp $

# Display the IMAP resource usage on the user's post office server.

use strict;
use warnings FATAL => 'all';
use Cyrus::IMAP;
use Getopt::Std;

sub usage(;$);
sub print_quota($$$$$);
sub send_command($);
sub quota_callback(@);
sub quotaroot_callback(@);
sub capability_callback(@);
sub close_and_errorout($);
sub close_connection();
sub errorout($);

sub usage(;$) {
    print STDERR "mailquota: $_[0]\n" if $_[0];
    print STDERR <<EOF;
Usage: mailquota [<options>] [<user>]
  Options:
    -h <host>     query <host> instead of default post office server
    -m <mailbox>  query for <mailbox> (default is INBOX)
    -n            be silent unless usage % is above the threshold
    -u <percent>  usage % threshold (default is 90); implies -n
    -d            turn on debugging
EOF
    exit 1;
}

my $need_header = 1;
my $root_width = 16;
my $num_width = 10;
my $percent_width = 5;
my $format = "%-${root_width}.${root_width}s" .
    " %${num_width}s %${num_width}s %${percent_width}s" .
    " %${num_width}s %${num_width}s %${percent_width}s\n";

# Parse the command line arguments.
my %opts;
getopts('dh:m:nu:', \%opts) || usage;
my $username = shift @ARGV || $ENV{'ATHENA_USER'} || $ENV{'USER'} || getlogin || (getpwuid($<))[0] ||
	errorout "Cannot determine user name";

usage "Too many arguments" if @ARGV > 0;
my $debug = $opts{'d'};
my $warn_only = $opts{'n'};
my $usage_threshold = $opts{'u'};
my $host = $opts{'h'} || (gethostbyname("$username.mail.mit.edu"))[0];
errorout "Cannot find Post Office server for $username" unless $host;
errorout "Exchange accounts are not supported yet. Try http://owa.mit.edu/." if $host =~ /EXCHANGE/;
my $mbox = $opts{'m'} || "INBOX";

# Validate the usage percentage threshold, allowing a trailing %.
# Setting the threshold implies we should only display quotas
# for which any resource usage is above the threshold.
if (defined $usage_threshold) {
    chop $usage_threshold if $usage_threshold =~ /^\d+%$/;
    usage "-u argument must be numeric" if $usage_threshold !~ /^\d+$/;
    $warn_only = 1;
} else {
    $usage_threshold = 90;
}

# Connect to the IMAP server, check for the QUOTA extension, and
# authenticate.
my $client = Cyrus::IMAP->new($host) ||
    errorout "Cannot connect to IMAP server on $host";
my $caps = '';
$client->addcallback({-trigger => 'CAPABILITY',
		      -callback => \&capability_callback,
		      -rock => \$caps});
send_command "CAPABILITY";
$caps =~ '\bQUOTA\b' ||
    close_and_errorout "$host does not support the IMAP QUOTA extension";
$client->authenticate(-authz => $username, -maxssf => 0) ||
    close_and_errorout "Cannot authenticate to $host";

# Send the GETQUOTAROOT command, which returns both the QUOTA and
# QUOTAROOT responses.  Quota information will be displayed via
# the QUOTA callback.
$client->addcallback({-trigger => 'QUOTA',
		      -callback => \&quota_callback});
$client->addcallback({-trigger => 'QUOTAROOT',
		      -callback => \&quotaroot_callback});
send_command "GETQUOTAROOT \"$mbox\"";

# We are done talking to the IMAP server; close down the connection.
close_connection();

# Print the quota information for the given quota root and its
# storage and message resource values.
sub print_quota($$$$$) {
    my ($root, $storage_used, $storage_max, $message_used, $message_max) = @_;
    my $storage_percent;
    my $storage_percent_out;
    my $message_percent;
    my $message_percent_out;

    # Calculate the usage percentages, and format for output.
    if ($storage_max) {
	$storage_percent = ($storage_used / $storage_max) * 100;
	$storage_percent_out = sprintf("%.0f%%", $storage_percent);
    }
    if ($message_max) {
	$message_percent = ($message_used / $message_max) * 100;
	$message_percent_out = sprintf("%.0f%%", $message_percent);
    }

    # Skip this quota if we are only displaying usages above the
    # specified threshold.
    return unless (!$warn_only ||
		   (defined $storage_percent &&
		    $storage_percent >= $usage_threshold) ||
		   (defined $message_percent &&
		    $message_percent >= $usage_threshold));

    # Print a header if this is the first line of output.
    if ($need_header) {
	print "** IMAP e-mail usage for $mbox on $host:\n";
	printf($format,
	       "Quota",
	       "KB Used", "KB Max", "KB %",
	       "# Msgs", "# Max", "# %");
	$need_header = 0;
    }
    printf($format,
	   $root,
	   defined $storage_used ? $storage_used : '-',
	   defined $storage_max ? $storage_max : '-',
	   defined $storage_percent_out ? $storage_percent_out : '-',
	   defined $message_used ? $message_used : '-',
	   defined $message_max ? $message_max : '-',
	   defined $message_percent_out ? $message_percent_out : '-');
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
    close_and_errorout "IMAP error for $mbox on $host: $text"
	if $status ne 'OK';
}

# Callback subroutine to parse the QUOTA response.
# The "-text" hash element contains the quota root name, and a list
# of quota resource names, usages, and limits.  Recognized names are
# STORAGE (sum of message sizes, in kilobytes) and MESSAGE (number of
# messages).  See RFC 2087.
sub quota_callback(@) {
    my %cb = @_;
    my ($root, $quotalist);
    print "In QUOTA callback: text $cb{-text}\n" if $debug;
    if (($root, $quotalist) = ($cb{-text} =~ /(\S*)\s+\((.*)\)/io)) {
	my ($storage_used, $storage_max, $message_used, $message_max);
	while ($quotalist) {
	    my ($resource, $used, $max);
	    ($resource, $used, $max, $quotalist) = split /\s/, $quotalist, 4;
	    last unless $max;
	    $resource = uc $resource;
	    if ($resource eq "STORAGE") {
		$storage_used = $used;
		$storage_max = $max;
	    }
	    elsif ($resource eq "MESSAGE") {
		$message_used = $used;
		$message_max = $max;
	    }
	}
	print_quota($root, $storage_used, $storage_max,
		    $message_used, $message_max)
	    if (defined $storage_max || defined $message_max);
    }
}

# Callback subroutine to parse the QUOTAROOT response.  The "-text"
# hash element contains the mailbox name, and zero or more quota root
# names.  This is currently used for debugging only.
sub quotaroot_callback(@) {
    my %cb = @_;
    print "In QUOTAROOT callback: text $cb{-text}\n" if $debug;
}

# Callback subroutine to parse the CAPABILITY response.  The "-rock" hash
# element is a reference to the string in which to store the space-separated
# capability names.
sub capability_callback(@) {
    my %cb = @_;
    print "In CAPABILITY callback: keyword $cb{-keyword}, text $cb{-text}\n"
	if $debug;
    ${$cb{-rock}} = $cb{-text};
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
    print STDERR "mailquota: $_[0]\n";
    exit 1;
}
