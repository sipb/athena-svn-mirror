#! /bin/sh
# dummy entry for unavailable filters
# GOVERNMENT END USERS: See Notice file in TranScript library directory
# -- probably /usr/lib/ps/Notice
# RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/lib/psbad.sh,v 1.1.1.1 1996-10-07 20:25:31 ghudson Exp $

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
