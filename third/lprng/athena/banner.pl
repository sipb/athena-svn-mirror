#!/usr/athena/bin/perl

# $Id: banner.pl,v 1.2 1999-05-11 18:40:31 mwhitson Exp $

$template = "/usr/athena/lib/filters/banner.ps";
$motdfile = "/etc/athena/print.motd";
$logofile = "/etc/athena/print.logo.ps";

# default to pass to banner.ps
$bannertype = "random";

use Getopt::Std;
use POSIX;     # for strftime()

# See section 6 of the LPRng HOWTO for list of possible command-line
# options; this gets invoked like a filter.

getopt('CFHJLPQRZacdefhijklnprswxy');

# Possible error codes, also from section 6 of the HOWTO:
# Key      Value   Meaning
# JSUCC    0       Successful
# JFAIL    1, 32   Failed - retry later
# JABORT   2, 33   Abort - terminate queue processing
# JREMOVE  3, 34   Failed - remove job
# JHOLD    6, 37   Failed - hold this job
# Other            Abort - terminate queue processing

# Search for -Zbanner=XXX

for $opt (split(/\,/, $opt_Z)) {
  $bannertype = $1 if ($opt =~ /^banner\W*\=\W*(\w+)/i);
}

$jobname = $opt_J;
($host = "\L$opt_h") =~ s/\.mit\.edu$//;   # downcase and chop off .mit.edu
$user = $opt_n;
$queue = $opt_P;
$queuejob = $queue . ":" . $jobname;
$date = strftime("%A, %e %B %Y  %H:%M:%S", localtime);

open(TEMPLATE, $template) || exit 0;  # JSUCC -- if the banner failed,
                                      # that's no reason to nuke the job.

if (open(MOTD, $motdfile)) {    # Try to open the motd file.  If we can't,
  $oldrs = $/;                  # just set the motd to an empty string.
  undef $/;
  $motd = <MOTD>;
  $/ = $oldrs;
  chomp $motd;
  $motd =~ s/([\(\)])/\\$1/g;   # rudimentary PS string quoting semantics
  close MOTD;
} else {
  $motd = "";
}

print <<EOF;
%!
/queuejob ($queuejob) def
/host ($host) def
/user ($user) def
/date ($date) def
/bannertype /$bannertype def
/motd ($motd) def
EOF

# The logo file should define a PS function "logo", which is called by
# banner.ps.  If nonexistent, we define a null logo.

if (open(LOGO, $logofile)) {
   print while <LOGO>;
   close LOGO;
} else {
   print "/logo { } def\n";
}

# Now spew out the template file.

print while <TEMPLATE>;
close TEMPLATE;

exit 0;
