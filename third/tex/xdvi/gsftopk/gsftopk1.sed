.\" Copyright (c) 1997 Paul Vojta.
.\"
.\" Redistribution and use in source and binary forms, with or without
.\" modification, are permitted provided that the following conditions
.\" are met:
.\" 1. Redistributions of source code must retain the above copyright
.\"    notice, this list of conditions and the following disclaimer.
.\" 2. Redistributions in binary form must reproduce the above copyright
.\"    notice, this list of conditions and the following disclaimer in the
.\"    documentation and/or other materials provided with the distribution.
.\"
.\" THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
.\" ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
.\" IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
.\" ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
.\" FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
.\" DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
.\" OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
.\" HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
.\" LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
.\" OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
.\" SUCH DAMAGE.
.\"
.TH GSFTOPK 1 "18 January 1997"
.SH NAME
gsftopk \- render a ghostscript font in TeX pk format
'	# small and boldface (not all -man's provide it)
.de SB
\&\fB\s-1\&\\$1 \\$2\s0\fR
..
.SH SYNOPSIS
\fBgsftopk\fP [\fB\-q\fP] \fIfont\fP \fIdpi\fP
.SH ARGUMENTS
.IP \fIfont\fP \w'\fIfont\fP'u+2m
Name of the font to be created.
.IP \fIdpi\fP
Desired resolution of the font to be created, in dots per inch.  This may
be a real number.
.SH DESCRIPTION
.B gsftopk
is a program which calls up the ghostscript program
.BR gs (1)
to render a given font at a given resolution.  It packs the resulting
characters into the
.B pk
file format and writes them to a file whose name is formed from the font
name and the resolution (rounded to the nearest integer).
.PP
This program should normally be called by a script, such as
#ifkpathsea
.BR MakeTeXPK ,
#endif
#ifnokpathsea
.BR xdvimakepk ,
#endif
to create fonts on demand.
.PP
.B gsftopk
obtains the character widths from the
.RI . tfm
file, which must exist in the standard search path.  It also must be
able to find a file
.B psfonts.map
(formatted as in
.BR dvips (1)),
listing the available fonts.
.PP
The following
.B pk
"specials" are added at the end of the output file, to provide an internal
check on the contents of the file:
.RB """" "jobname=\fIfont\fP" ""","
.RB """" mag=1 ""","
.RB """" mode=(gsftopk)Unknown ""","
.RB """" pixels_per_inch=\fIdpi\fP """."
This is in accordance with the TeX Directory Standard (TDS).
.SH OPTIONS
.TP
.B \-q
Operate quietly;
.I i.e.,
without writing any messages to the standard output.
.SH ENVIRONMENT VARIABLES
.IP \fBTEXFONTS\fP \w'\fBDVIPSHEADERS\fP'u+2m
Colon-separated list of directories to search for the
.RI . tfm
file associated with the font.  An extra colon in the list will include the
system default path at that point.  A double slash will enable recursive
subdirectory searching at that point in the path.
.IP \fBDVIPSHEADERS\fP
Colon-separated list of directories to search for the ghostscript
driver file
.B render.ps
and for any PostScript font files
.RI (. pfa
or
.RI . pfb
files).  An extra colon in the list behaves as with
.SB TEXFONTS.
.IP \fBTEXCONFIG\fP
Path to search for the file
.BR psfonts.map .
Only the first such file will be used.
.SH BUGS
.B gsftopk
sometimes has trouble with fonts with very complicated characters
(such as the Seal of the University of California).  This is because
.B gsftopk
uses the
.B charpath
operator to determine the bounding box of each character.  If the character
is too complicated, then old versions of
.B ghostscript
fail, causing
.B gsftopk
to terminate with an error message
.IP
.B "Call to gs stopped by signal 10"
.LP
(The number may vary from system to system; it corresponds to a bus error
or a segmentation fault.)  The best way to fix this bug is to install a
current version of
.BR ghostscript .
As an alternative,
.B gsftopk
can be instructed to use the bounding box provided with the font (if one
exists) instead of finding a bounding box for each character.  To do this,
include the string
.IP
.B /usefontbbox true def
.LP
in the
.B psfonts.map
file;
.IR e.g. ,
.IP
.B ucseal """/usefontbbox true def"""
.LP
This will not affect use of the font by
.BR dvips .
.SH SEE ALSO
.BR gs (1),
.BR gftopk (1),
.BR tex (1),
.BR xdvi (1),
.BR dvips (1)
.SH AUTHOR
Written by Paul Vojta.  This program was inspired by
.BR gsrenderfont ,
which was written by Karl Berry.
#ifkpathsea
.SH MODIFICATIONS
Modified by Yves Arrouye to use Karl Berry's kpathsea library.
#endif
