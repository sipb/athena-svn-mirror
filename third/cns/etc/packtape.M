.\" Copyright 1995 by Cygnus Support.
.\"
.\" placed under General Public License.
.\" 
.TH packtape 1 "Cygnus Network Security V4 95q4 Release" "Cygnus Support"
.SH NAME
packtape \- produce a tape from a Cygnus CNS Version 4 release vault
.SH SYNOPSIS
.B packtape
.I release-host
.I device-name
.br
.SH DESCRIPTION
.I packtape
grabs documentation, binaries, and sources out of the vault and puts
them on a tape.
.PP
.I release-host
can currently be one of
.B linux alpha-osf3 alpha-osf2 hppa-hpux9 m68k-hpux9
.B unixware ultrix irix4 irix5 aix4 aix3 solaris2.5
.B sunos4.1.3 sunos4.1.4
all of which produce a pair of 
.I tar
files (installation and source), or one of
.B sco solaris2.4
which produce a pair of
.I pkg
files for use with
.B pkgadd
for installation and source.
.SH FILES 
/usr/cygnus/release-vault/cns-95q4

.SH SEE ALSO
mt(1), pkgadd(1M)
