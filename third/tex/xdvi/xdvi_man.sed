.\" Copyright (c) 1994 Paul Vojta.  All rights reserved.
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
.if t .ds Te T\\h'-0.1667m'\\v'0.20v'E\\v'-0.20v'\\h'-0.125m'X
.if n .ds Te TeX
'	# small and boldface (not all -man's provide it)
.de SB
\&\fB\s-1\&\\$1 \\$2\s0\fR
..
.TH XDVI 1 "28 April 1995" "X Version 11"
.SH NAME
xdvi \- DVI Previewer for the X Window System
.SH SYNOPSIS
.B xdvi
.nh
[\fB+\fP[\fIpage\fP]] [\fB\-s\fP \fIshrink\fP] [\fB\-S\fP \fIdensity\fP]
#ifgrey
[\fB\-nogrey\fP] [\fB\-gamma\fP \fIg\fP]
#endif
[\fB\-p\fP \fIpixels\fP]
[\fB\-margins\fP \fIdimen\fP]
[\fB\-sidemargin\fP \fIdimen\fP] [\fB\-topmargin\fP \fIdimen\fP]
[\fB\-offsets\fP \fIdimen\fP]
[\fB\-xoffset\fP \fIdimen\fP] [\fB\-yoffset\fP \fIdimen\fP]
[\fB\-paper\fP \fIpapertype\fP] [\fB\-altfont\fP \fIfont\fP]
#ifmakepk
[\fB\-nomakepk\fP]
#endif
[\fB\-mfmode\fP \fImode-def\fP]
[\fB\-l\fP]
[\fB\-rv\fP]
#ifbuttons
[\fB\-expert\fP]
#endif
[\fB\-mgs\fP[\fIn\fP] \fIsize\fP]
[\fB\-hush\fP]
[\fB\-hushspecials\fP] [\fB\-hushchars\fP] [\fB\-hushchecksums\fP]
[\fB\-safer\fP]
[\fB\-fg\fP \fIcolor\fP] [\fB\-bg\fP \fIcolor\fP] [\fB\-hl\fP \fIcolor\fP]
[\fB\-bd\fP \fIcolor\fP] [\fB\-cr\fP \fIcolor\fP]
[\fB\-bw\fP \fIwidth\fP]
[\fB\-display\fP \fIhost:display\fP] [\fB\-geometry\fP \fIgeometry\fP]
[\fB\-icongeometry\fP \fIgeometry\fP] [\fB\-iconic\fP]
#ifbuttons
[\fB\-font\fP \fIfont\fP]
#endif
[\fB\-keep\fP] [\fB\-copy\fP] [\fB\-thorough\fP]
#ifps
[\fB\-nopostscript\fP]
[\fB\-noscan\fP]
[\fB\-allowshell\fP]
#endif
#ifdps
[\fB\-nodps\fP]
#endif
#ifnews
[\fB\-nonews\fP]
#endif
#ifghost
[\fB\-noghostscript\fP]
[\fB\-interpreter\fP \fIpath\fP]
[\fB\-nogssafer\fP]
[\fB\-gspalette\fP \fIpalette\fP]
#endif
[\fB\-debug\fP \fIbitmask\fP] [\fB\-version\fP]
.I dvi_file
.hy
.SH DESCRIPTION
.B xdvi
is a program which runs under the X window system. It is used to preview
.I dvi
files, such as are produced by
.BR tex (1).
.PP
This program has the capability of showing the file shrunken by various
(integer) factors, and also has a ``magnifying glass'' which allows one
to see a small part of the unshrunk image momentarily.
.PP
Before displaying any page or part thereof, it checks to see if the
.I dvi
file has changed since the last time it was displayed.  If this is the case,
then
.B xdvi
will reinitialize itself for the new
.I dvi
file.  For this reason, exposing parts of the
.B xdvi
window while \*(Te\& is running should be avoided.  This feature allows you
to preview many versions of the same file while running
.B xdvi
only once.
#ifbuttons
.PP
In addition to using keystrokes to move within the file,
.B xdvi
provides buttons on the right side of the window, which are synonymous
with various sequences of keystrokes.
#endif
#ifps
.PP
.B xdvi
can show PostScript<tm> specials by any of three methods.
It will try first to use Display PostScript<tm>, then NeWS, then it
will try to use Ghostscript to render the images.  All of these options
depend on additional software to work properly; moreover, some of them
may not be compiled into this copy of
.BR xdvi .
.PP
For performance reasons,
.B xdvi
does not render PostScript specials in the magnifying glass.
#endif
.SH OPTIONS
In addition to specifying the
.I dvi
file (with or without the
.B .dvi
extension),
.B xdvi
supports the following command line options.  If the option begins with a
.RB ` + '
instead of a
.RB ` \- ',
the option is restored to its default value.  By default, these options can
be set via the resource names given in parentheses in the description of
each option.
.TP
.BI + page
Specifies the first page to show.  If
.B +
is given without a number, the last page is assumed; the first page is
the default.
#ifps
.TP
.B \-allowshell
.RB ( .allowShell )
This option enables the shell escape in PostScript specials.
(For security reasons, shell escapes are usually disabled.)
This option should be rarely used; in particular it should not be used just
to uncompress files: that function is done automatically if the file name
ends in
.B .Z
or
.BR .gz .
Shell escapes are always turned off if the
.B \-safer
option is used.
#endif
.TP
.BI \-altfont " font"
.RB ( .altFont )
Declares a default font to use when the font in the
.I dvi
file cannot be found.  This is useful, for example, with PostScript <tm> fonts.
.TP
.BI \-background " color"
.RB ( .background )
Determines the color of the background.  Same as
.BR -bg .
.TP
.BI \-bd " color"
.RB ( .borderColor )
Determines the color of the window border.
.TP
.BI \-bg " color"
.RB ( .background )
Determines the color of the background.
.TP
.BI \-bordercolor " color"
Same as
.BR -bd .
.TP
.BI \-borderwidth " width"
.RB ( .borderWidth )
Specifies the width of the border of the window.  Same as
.BR -bw .
.TP
.BI \-bw " width"
.RB ( .borderWidth )
Specifies the width of the border of the window.
.TP
.B \-copy
.RB ( .copy )
Always use the
.I copy
operation when writing characters to the display.
This option may be necessary for correct operation on a color display, but
overstrike characters will be incorrect.
#ifgrey
If greyscale anti-aliasing is in use, the
.B \-copy
operation will disable the use of colorplanes and make overstrikes come
out incorrectly.
#endif
See also
.BR \-thorough .
.TP
.BI \-cr " color"
.RB ( .cursorColor )
Determines the color of the cursor.  The default is the color of the page
border.
.TP
.BI \-debug " bitmask"
.RB ( .debugLevel )
If nonzero, prints additional debugging information on standard output.
The bitmask should be given as a decimal number.  The values of the bits
are defined in the source file
.BR xdvi.h .
.TP
.BI \-density " density"
.RB ( .densityPercent )
Determines the density used when shrinking bitmaps for fonts.
A higher value produces a lighter font.  The default value is 40.  Same as
.BR \-S .
.TP
.BI \-display " host" : display
Specifies the host and screen to be used for displaying the
.I dvi
file.  By default this is obtained from the environment variable
.SB DISPLAY.
#ifbuttons
.TP
.B \-expert
.RB ( .expert )
Prevent the buttons from appearing.  See also the
.RB ` x '
keystroke.
#endif
.TP
.BI \-fg " color"
.RB ( .foreground )
Determines the color of the text (foreground).
.TP
.BI \-foreground " color"
Same as
.BR -fg .
#ifbuttons
.TP
.BI \-font " font"
.RB ( *font )
Sets the font for use in the buttons.
#endif
#ifgrey
.TP
.BI \-gamma " gamma"
.RB ( .gamma )
Controls the interpolation of colors in the greyscale anti-aliasing color
palette.  Default value is 1.0.  For 0 < 
.I gamma
< 1, the fonts will be lighter (more like the background), and for
.I gamma
> 1, the fonts will be darker (more like the foreground).  Negative
values behave the same way, but use a slightly different algorithm.
#endif
.TP
.BI \-geometry " geometry"
.RB ( *geometry )
Specifies the initial geometry of the window.
#ifghost
.TP
.BI \-gspalette " palette"
.RB ( .palette )
Specifies the palette to be used when using Ghostscript for rendering
PostScript specials.  Possible values are
.BR Color ,
.BR Greyscale ,
and
.BR Monochrome .
The default is
.BR Color .
#endif
.TP
.BI \-hl " color"
.RB ( .highlight )
Determines the color of the page border.  The default is the foreground color.
.TP
.B \-hush
.RB ( .Hush )
Causes
.B xdvi
to suppress all suppressible warnings.
.TP
.B \-hushchars
.RB ( .hushLostChars )
Causes
.B xdvi
to suppress warnings about references to characters which are not defined
in the font.
.TP
.B \-hushchecksums
.RB ( .hushChecksums )
Causes
.B xdvi
to suppress warnings about checksum mismatches between the
.I dvi
file and the font file.
.TP
.B \-hushspecials
.RB ( .hushSpecials )
Causes
.B xdvi
to suppress warnings about
.B \especial
strings that it cannot process.
.TP
.BI \-icongeometry " geometry"
.RB ( .iconGeometry )
Specifies the initial position for the icon.
.TP
.B \-iconic
.RB ( .iconic )
Causes the
.B xdvi
window to start in the iconic state.  The default is to start with the
window open.
#ifghost
.TP
.BI \-interpreter " filename"
.RB ( .interpreter )
Use
.I filename
as the Ghostscript interpreter.  By default it uses
.BR %%gspath%% .
#endif
.TP
.B \-keep
.RB ( .keepPosition )
Sets a flag to indicate that
.B xdvi
should not move to the home position when moving to a new page.  See also the
.RB ` k '
keystroke.
.TP
.B \-l
.RB ( .listFonts )
Causes the names of the fonts used to be listed.
.TP
.BI \-margins " dimen"
.RB ( .Margin )
Specifies the size of both the top margin and side margin.
This should be a decimal number optionally followed by
.RB `` cm '',
.IR e.g. ,
.B 1.5
or
.BR 3cm ,
giving a measurement in inches or centimeters.
It determines the ``home'' position of the page within the window as
follows.  If the entire
page fits in the window, then the margin settings are ignored.  If, even
after removing the margins from the left, right, top, and bottom, the page
still cannot fit in the window, then the page is put in the window such that
the top and left margins are hidden, and presumably the upper left-hand corner
of the text on the page will be in the upper left-hand corner of the window.
Otherwise, the text is centered in the window.  See also
.BR \-sidemargin ", " \-topmargin ,
and the keystroke
.RB ` M .'
.TP
.BI \-mfmode " mode-def"
.RB ( .mfMode )
Specifies a
.I mode-def
string, which can be used in searching for fonts (see ENVIRONMENT, below).
#ifmakepk
It is also passed to
.B metafont
during automatic creation of fonts.
#endif
.TP
.BI \-mgs " size"
Same as
.BR \-mgs1 .
.TP
.BI "\-mgs\fR[\fIn\fR]" " size"
.RB ( .magnifierSize\fR[\fIn\fR] )
Specifies the size of the window to be used for the ``magnifying glass''
for Button
.IR n .
The size may be given as an integer (indicating that the magnifying glass
is to be square), or it may be given in the form
.IR width x height .
See the MOUSE ACTIONS section.  Defaults are 200x150, 400x250, 700x500,
1000x800, and 1200x1200.
#ifdps
.TP
.B \-nodps
.RB ( .dps )
Inhibits the use of Display PostScript<tm> for displaying PostScript<tm>
specials.  Other forms of PostScript emulation, if installed, will be used
instead.
(For this option, the logic of the corresponding resource is reversed:
.B \-nodps
corresponds to
.BR dps:off ;
.B +nodps
to
.BR dps:on .)
#endif
#ifghost
.TP
.B \-noghostscript
.RB ( .ghostscript )
Inhibits the use of Ghostscript for displaying PostScript<tm> specials.
(For this option, the logic of the corresponding resource is reversed:
.B \-noghostscript
corresponds to
.BR ghostscript:off ;
.B +noghostscript
to
.BR ghostscript:on .)
#endif
#ifgrey
.TP
.B \-nogrey
.RB ( .grey )
Turns off the use of greyscale anti-aliasing when printing shrunken bitmaps.
(For this option, the logic of the corresponding resource is reversed:
.B \-nogrey
corresponds to
.BR grey:off ;
.B +nogrey
to
.BR grey:on .)
See also the
.RB ` G '
keystroke.
#endif
#ifghost
.TP
.B \-nogssafer
.RB ( .gsSafer )
Normally, if Ghostscript is used to render PostScript specials, the Ghostscript
interpreter is run with the option
.BR \-dSAFER .
The
.B \-nogssafer
option runs Ghostscript without
.BR \-dSAFER .
The
.B \-dSAFER
option in Ghostscript disables PostScript operators such as
.BR deletefile ,
to prevent possibly malicious PostScript programs from having any effect.
If the
.B \-safer
option is specified, then this option has no effect; in that case Ghostscript
is always run with
.BR \-dSAFER .
(For the
.B \-nogssafer
option, the logic of the corresponding resource is reversed:
.B \-nogssafer
corresponds to
.BR gsSafer:off ;
.B +nogssafer
to
.BR gsSafer:on .)
#endif
#ifmakepk
.TP
.B \-nomakepk
.RB ( .makePk )
Turns off automatic generation of font files that cannot be found by other
means.
(For this option, the logic of the corresponding resource is reversed:
.B \-nomakepk
corresponds to
.BR makePk:off ;
.B +nomakepk
to
.BR makePK:on .)
#endif
#ifnews
.TP
.B \-nonews
.RB ( .news )
Inhibits the use of NeWS<tm> for displaying PostScript<tm> specials.
Ghostscript, if enabled by the installation, will be used instead.
(For this option, the logic of the corresponding resource is reversed:
.B \-nonews
corresponds to
.BR news:off ;
.B +news
to
.BR news:on .)
#endif
#ifps
.TP
.B \-nopostscript
.RB ( .postscript )
Turns off rendering of PostScript<tm> specials.  Bounding boxes, if known,
will be displayed instead.  This option can also be toggled with the
.RB ` v '
keystroke.
(For this option, the logic of the corresponding resource is reversed:
.B \-nopostscript
corresponds to
.BR postscript:off ;
.B +postscript
to
.BR postscript:on .)
#endif
#ifps
.TP
.B \-noscan
.RB ( .prescan )
Normally, when PostScript<tm> is turned on,
.B xdvi
will do a preliminary scan of the
.I dvi
file, in order to send any necessary header files before sending the
PostScript code that requires them.  This option turns off such prescanning.
(It will be automatically be turned back on if
.B xdvi
detects any specials that require headers.)  (For the
.B \-noscan
option, the logic of the corresponding resource is reversed:
.B \-noscan
corresponds to
.BR prescan:off ;
.B +noscan
to
.BR prescan:on .)
#endif
.TP
.BI \-offsets " dimen"
.RB ( .Offset )
Specifies the size of both the horizontal and vertical offsets of the
output on the page.  This should be a decimal number optionally followed by
.RB `` cm '',
.IR e.g. ,
.B 1.5
or
.BR 3cm ,
giving a measurement in inches or centimeters.
By decree of the Stanford \*(Te\& Project, the default \*(Te\& page origin is
always 1 inch over and down from the top-left page corner, even when
non-American paper sizes are used.  Therefore, the default offsets
are 1.0 inch. See also
.B \-xoffset
and
.BR \-yoffset .
.TP
.BI \-p " pixels"
.RB ( .pixelsPerInch )
Defines the size of the fonts to use, in pixels per inch.  The
default value is %%bdpi%%.
.TP
.BI \-paper " papertype"
.RB ( .paper )
Specifies the size of the printed page.  This may be of the form
\fIwidth\fBx\fIheight\fR (or \fIwidth\fBx\fIheight\fBcm\fR), where
.I width
is the width in inches (or cm) and
.I height
is the height in inches (or cm), respectively.
There are also synonyms which may be used:
.B us
(8.5x11),
.B usr
(11x8.5),
.B legal
(8.5x14),
.B foolscap
(13.5x17),
as well as the ISO sizes
.BR a1 - a7 ,
.BR b1 - b7 ,
.BR c1 - c7 ,
.BR a1r - a7r
.RB ( a1 - a7
rotated), etc.  The default size is %%defaultpagesize%%.
.TP
.B \-rv
.RB ( .reverseVideo )
Causes the page to be displayed with white characters on a black background,
instead of vice versa.
.TP
.BI \-s " shrink"
.RB ( .shrinkFactor )
Defines the initial shrink factor.  The default value is 3.
.TP
.BI \-S " density"
.RB ( .densityPercent )
Determines the density used when shrinking bitmaps for fonts.
A higher value produces a lighter font.  The default value is 40.
Same as
.BR \-density .
.TP
.B \-safer
.RB ( .safer )
This option turns on all available security options; it is designed for use when
.B xdvi
is called by a browser that obtains a
.I dvi
or \*(Te\& file from another site.
#ifps
In the present case, this option selects
#endif
#ifghost
.B +nogssafer
and
#endif
#ifps
.BR +allowshell .
#endif
#ifnops
In the present case, this option is accepted but has no effect, since
.B xdvi
has been compiled without support for PostScript specials.
#endif
.TP
.BI \-sidemargin " dimen"
.RB ( .sideMargin )
Specifies the side margin (see
.BR \-margins ).
.TP
.B \-thorough
.RB ( .thorough )
.B xdvi
will usually try to ensure that overstrike characters
.RI ( e.g. ,
.BR \enotin )
are printed correctly.  On monochrome displays, this is always possible
with one logical operation, either
.I and
or
.IR or .
On color displays, however, this may take two operations, one to set the
appropriate bits and one to clear other bits.  If this is the case, then
by default
.B xdvi
will instead use the
.I copy
operation, which does not handle overstriking correctly.  The
.B \-thorough
option chooses the slower but more correct choice.  See also
.BR \-copy .
.TP
.BI \-topmargin " dimen"
.RB ( .topMargin )
Specifies the top and bottom margins (see
.BR \-margins ).
.TP
.BI \-version
Print information on the version of
.BR xdvi .
.TP
.BI \-xoffset " dimen"
.RB ( .xOffset )
Specifies the size of the horizontal offset of the output on the page.  See
.BR \-offsets .
.TP
.BI \-yoffset " dimen"
.RB ( .yOffset )
Specifies the size of the vertical offset of the output on the page.  See
.BR -offsets .
.SH KEYSTROKES
.B xdvi
recognizes the following keystrokes when typed in its window.
Each may optionally be preceded by a (positive or negative) number, whose
interpretation will depend on the particular keystroke.
Also, the ``Home'', ``Prior'', ``Next'', and arrow cursor keys are synonyms for
.RB ` ^ ',
.RB ` b ',
.RB ` f ',
.RB ` l ',
.RB ` r ',
.RB ` u ',
and
.RB ` d '
keys, respectively.
.TP
.B q
Quits the program.  Control-C and control-D will do this, too.
.TP
.B n
Moves to the next page (or to the
.IR n th
next page if a number is given).  Synonyms are
.RB ` f ',
Space, Return, and Line Feed.
.TP
.B p
Moves to the previous page (or back
.I n
pages).  Synonyms are
.RB ` b ',
control-H, and Delete.
.TP
.B g
Moves to the page with the given number.  Initially, the first page is assumed
to be page number 1, but this can be changed with the
.RB ` P '
keystroke, below.  If no page number is given, then it goes to the last page.
.TP
.B P
``This is page number
.IR n .''
This can be used to make the
.RB ` g '
keystroke refer to actual page numbers instead of absolute page numbers.
.TP
Control-L
Redisplays the current page.
.TP
.B ^
Move to the ``home'' position of the page.  This is normally the upper
left-hand corner of the page, depending on the margins as described in the
.B \-margins
option, above.
.TP
.B u
Moves up two thirds of a window-full.
.TP
.B d
Moves down two thirds of a window-full.
.TP
.B l
Moves left two thirds of a window-full.
.TP
.B r
Moves right two thirds of a window-full.
.TP
.B c
Moves the page so that the point currently beneath the cursor is moved to
the middle of the window.  It also (gasp!) warps the cursor to the same place.
.TP
.B M
Sets the margins so that the point currently under the cursor is the upper
left-hand corner of the text in the page.  Note that this command itself does
not move the image at all.  For details on how the margins are used, see
the
.B \-margins
option.
.TP
.B s
Changes the shrink factor to the given number.  If no number is given, the
smallest factor that makes the entire page fit in the window will be used.
(Margins are ignored in this computation.)
.TP
.B S
Sets the density factor to be used when shrinking bitmaps.  This should
be a number between 0 and 100; higher numbers produce lighter characters.
#ifgrey
If greyscaling mode is in effect, this changes the value of gamma instead.
The new value of gamma is the given number divided by 100; negative values
are allowed.
#endif
.TP
.B R
Forces the
.I dvi
file to be reread.  This allows you to preview many versions of the same
file while running
.B xdvi
only once.
.TP
.B k
Normally when
.B xdvi
switches pages, it moves to the home position as well.  The
.RB ` k '
keystroke toggles a `keep-position' flag which, when set, will keep
the same position when moving between pages.  Also
.RB ` 0k '
and
.RB ` 1k '
clear and set this flag, respectively.  See also the
.B \-keep
option.
#ifbuttons
.TP
.B x
Toggles expert mode (in which the buttons do not appear).  Also
.RB ` 0x '
and
.RB ` 1x '
clear and reset this mode, respectively.  See also the
.B \-expert
option.
#endif
#ifgrey
.TP
.B G
This key toggles the use of greyscale anti-aliasing for displaying shrunken
bitmaps.  In addition, the key sequences
.RB ` 0G '
and
.RB ` 1G '
clear and set this flag, respectively.  See also the
.B \-nogrey
option.
#endif
#ifps
.TP
.B v
This key toggles the rendering of PostScript<tm> specials.  If rendering
is turned off, then bounding boxes are displayed when available.
In addition the key sequences
.RB ` 0v '
and
.RB ` 1v '
clear and set this flag, respectively.  See also the
.B \-nopostscript
option.
#endif
.SH MOUSE ACTIONS
If the shrink factor is set to any number other than one, then clicking
any mouse button will pop up a ``magnifying glass'' which shows the unshrunk
image in the vicinity of the mouse click.  This subwindow disappears when
the mouse button is released.  Different mouse buttons produce different sized
windows, as indicated by the
.B \-mgs
option.  Moving the cursor while holding the button down will move the
magnifying glass.
.PP
Also, the scrollbars (if present) behave in the standard way:  pushing Button 2
in a scrollbar moves the top or left edge of the scrollbar to that point
and optionally drags it;
pushing Button 1 moves the image up or right by an amount equal to the distance
from the button press to the upper left-hand corner of the window; pushing
Button 3 moves the image down or left by the same amount.
.SH ENVIRONMENT
The environment variable
.SB DISPLAY
specifies which bit map display terminal to use.
.PP
The environment variable
.SB XDVIFONTS
determines the path(s) searched for
fonts in the following manner.  The string consists of one or more strings
separated by colons.  In each such string, the substring
.B %f
is changed to the font name;
.B %d
is changed to the magnification; and
.B %p
is changed to the font file format
.RB (`` pk ''
or
.RB `` gf '').
If no
.B %f
appears in the string, then the string
.RB `` /%f.%d%p ''
is added on the end.  For example, if the string is
.RB `` /usr/local/tex/fonts ''
and the font is
.B cmr10
at 300 dots per inch, then it searches for
.B /usr/local/tex/fonts/cmr10.300pk
and
.BR /usr/local/tex/fonts/cmr10.300gf ,
in that order.  An extra colon anywhere in
.SB XDVIFONTS
causes the system default paths to be tried at that point.  If the font is not
found in the desired size, then
.B xdvi
will
#ifmakepk
invoke Metafont to create the font in the correct size.  Failing that, it will
#endif
try to find the nearest size.
If the font cannot be found at all, then
.B xdvi
will try to vary the point size of the font (within a certain range),
and if this fails, then it will use the font specified as the alternate
font (cf.
.BR \-altfont ).
.PP
In addition, a
.B %F
specifier is available; it is a synonym for
.BR %f ,
but it does not inhibit putting the string
.RB `` /%f.%d%p ''
at the end.  A
.B %b
specifier is available; it is converted to the current resolution being used
.RI ( i.e. ,
the value of the
.B \-p
parameter or the
.B .pixelsperinch
resource).  And finally, the string
.B %m
will be changed to the
.I mode-def
specified in the
.B \-mfmode
argument or the
.B .mfMode
resource.
#iftexfonts
.PP
For compatibility with \*(Te\&, you may also use
.SB TEXFONTS
in place of
.SB XDVIFONTS,
although in that case the variable should not include any
.RB `` % ''
specifiers.  The reason for recognizing
.SB TEXFONTS
is that certain versions of \*(Te\& also support the convention regarding
an extra colon in the font path; therefore, users who create their own fonts
can put both their
.RI . tfm
and raster files in the same directory and do
.RB `` "setenv TEXFONTS :MFdir" ''
or
.RB `` "setenv TEXFONTS MFdir:" ''
in order to get both \*(Te\& and
.B xdvi
to search their directory in addition to the system standard directories.  The
.SB XDVIFONTS
variable overrides the
.SB TEXFONTS
variable, so that on those sites where
.SB TEXFONTS
must be set explicitly, and therefore this feature is not useful, the
.SB XDVIFONTS
variable may be set to an empty string
.RI ( i.e. ,
.RB  `` "setenv XDVIFONTS" '')
to cause
.B xdvi
to ignore
.SB TEXFONTS.
.PP
.B xdvi
also recognizes the
.SB PKFONTS
and
.SB TEXPKS
variables, which are checked after
.SB XDVIFONTS
but before
.SB TEXFONTS.
#endif
#ifmakepk
.PP
The script used to create fonts may be controlled by the environment
variable
.SB XDVIMAKEPK.
Usually this variable would be set to the name of the script.
In that case the script is called with the following options:
(1) the font name, (2) the requested resolution in dots per inch,
(3) the base resolution in dots per inch, and (4) a (possibly more accurate)
indication of the magnification using magsteps (if possible).
Optionally, the variable may include specifiers
.RB `` %n ,''
.RB `` %d ,''
.RB `` %b ,''
and
.RB `` %m ''
to indicate each of the above arguments, respectively.
It is possible to specify the
.I mode-def
that Metafont is to use when creating the fonts.  This is specified by the
.B \-mfmode
argument on the command line, or the
.B mfMode
resource.  If a
.RB `` %o ''
specifier appears in the
.SB XDVIMAKEPK
string, it will be changed to the given
.I mode-def
name, or to the string
.B default
if no
.I mode-def
was given.  If no
.RB `` %o ''
specifier appears and a
.I mode-def
was given, then the mode will be appended to the end of the string.
This is compatible with the font creation mechanism used in
.BR dvips (1).
By default,
.SB XDVIMAKEPK
equals
.BR %%mkpk%% .
#endif
#ifsubdir
.PP
You can also enable recursive searching in the font path by using the
.RB `` * ''
and
.RB `` ** ''
specifiers.  At this point in the path,
.B xdvi
will recursively
search subdirectories of the given directory in order to find font files.
A single asterisk limits the search to one level; a double asterisk will search
through an arbitrary number of levels.  Also, the variable
.SB TEXFONTS_SUBDIR
can be used to give a colon-separated list of directories to recursively search.
This is equivalent to including the specifiers with a
.RB `` * ''
after each; the usual conventions regarding extra colons applies here, too,
relative to a default subdirectory path.  Asterisks may not be preceded by a
.RB `` % ''
specifier in any path component.
#endif
.PP
The
.SB XDVISIZES
variable may be set to indicate which sizes of fonts are available.
It should consist of a list of numbers separated by colons.  If
the list begins with a colon, the system default sizes are used, as well.
Sizes are expressed in dots per inch and must be integers.
The current default set of sizes is %%DEFAULT_FONT_SIZES%%.
.B xdvi
will also try the actual size of the font before trying any of the given sizes.
.PP
Virtual fonts are also supported, although
.B xdvi
does not have any built-in fonts to which they can refer.  The search path for
.RB . vf
files can be specified with the environment variable
.SB XDVIVFS
in a similar manner to that for the
.SB XDVIFONTS
variable.
#iftexfonts
.B xdvi
will also check the
.SB VFFONTS
variable if the
.SB XDVIFONTS
variable is not set.
#endif
Virtual fonts are searched for immediately after looking for the font
as a normal font in the exact size specified.
#ifps
.PP
The
.SB XDVIFIGS
and
.SB XDVIHEADERS
variables determine the search strategy for PostScript figure files and
header files, respectively.  First
.B xdvi
will look in the directory containing the
.I dvi
file.  If no file is found, then it will look in the directories given by
the
.SB XDVIFIGS
or
.SB XDVIHEADERS
environment variable, for PostScript figure files and header files,
respectively.  These variables should contain lists of directories separated
by colons.  As usual, an extra colon anywhere in either of these paths
causes the default compiled-in list to be checked at that point in the
path list.  Customarily,
.SB XDVIFIGS
will be the same as the \*(Te\& input directory.
If
.SB XDVIFIGS
or
.SB XDVIHEADERS
is not set, then
.B xdvi
will also check the
.SB TEXINPUTS
or
.SB HEADERPATH
environment variables; this is for compatibility with
.BR dvips (1).
.PP
There are two exceptions to the above strategy.  First, if the file name
begins with a slash
.RB ( / ),
then the file name is treated as an absolute path, and is searched for as
given, without using the above strategy.  Or, if the file name begins
with a backtick
.RB ( ` ),
then the following characters give a shell command (often
.BR zcat )
which is executed; its standard output is then sent to be interpreted as
PostScript.  Note that there is some potential for security problems here;
see the
.B \-allowshell
command-line option.  It is better to use compressed files directly (see below).
.PP
If a file name is given (as opposed to a shell command),
if that file name ends in
.B .Z
or
.BR .gz ,
and if the first two bytes of the file indicate that it was compressed with
.BR compress (1)
or
.BR gzip (1),
respectively, then the file is first uncompressed with
.B uncompress \-c
or
.BR "gunzip \-c" ,
respectively.  This is preferred over using a backtick to call the command
directly, since you do not have to specify
.B \-allowshell
and since it allows for path searching.
#endif
#ifps
.SH LIMITATIONS
.B xdvi
accepts many but not all types of PostScript specials accepted by
.BR dvips .
It does accept most specials generated by
.B epsf
and
.BR psfig ,
for example.  It does not, however, support
.BR bop\-hook ,
nor does it do the ``NEAT'' or rotated ``A'' example in the
.B dvips
manual.  These restrictions are due to the design of
.BR xdvi ;
in all likelihood they will always remain.
.PP
La\*(Te\&2e color and rotation specials are not currently supported.
#endif
.SH FILES
.PD 0
.TP 40
%%DEFAULT_FONT_PATH%%   Font pixel files.
%%DEFAULT_VF_PATH%%   Virtual font files.
#ifps
%%DEFAULT_FIG_PATH%%   Directories containing PostScript figures.
%%DEFAULT_HEADER_PATH%%   Directories containing PostScript header files.
#endif
.PD
.SH "SEE ALSO"
.BR X (1).
.SH AUTHORS
Eric Cooper, CMU, did a version for direct output to a QVSS.
Modified for X by
Bob Scheifler, MIT Laboratory for Computer Science.
Modified for X11 by Mark Eichin, MIT SIPB.
Additional enhancements by many others.
