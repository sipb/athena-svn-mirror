#! /bin/sh
# dummy entry for unavailable filters
# GOVERNMENT END USERS: See Notice file in TranScript library directory
# -- probably /usr/lib/ps/Notice
# RCSID: $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/transcript-v2.1/psbad.sh,v 1.1 1993-10-12 05:28:02 probe Exp $

# argv is: psbad filtername user host
prog=`basename $0`
filter=$1
printer=$2
user=$3
host=$4

cat <<bOGUSfILTER
%!
72 720 moveto /Courier-Bold findfont 10 scalefont setfont
(lpd: ${printer}: filter \"$filter\" not available [$user $host].)show 
showpage

bOGUSfILTER
exit 0
