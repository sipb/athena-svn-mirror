.\" Copyright (c) 1996 Paul Vojta.  All rights reserved.
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
.TH XDVI 1 "20 September 1996" "X Version 11"
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
[\fB\-shrinkbutton\fP\fIn\fP \fIshrink\fP]
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
By default, it is %%mfmode%%.
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
Defines the initial shrink factor.  The default value is %%shrink%%.
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
#ifbuttons
.TP
.BI \-shrinkbutton "n shrink"
.RB ( .shrinkButton\fIn\fP )
Specifies that the
.IR n th
button changing shrink factors shall change to shrink factor
.IR factor .
This is useful, e.g., when using 600 dpi fonts, since in that case shrinking
by a factor of 4 is still not enough.  Here
.I n
may be a number from 1 to 4.
[Note:  this option is likely to change in the future.]
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
.SH SIGNALS
When
.B xdvi
receives a
.SB SIGUSR1
signal, it rereads the
.I dvi
file.
.SH ENVIRONMENT
.TP
.SB DISPLAY
Which bit map display terminal to use.
#ifconfig
.TP
.SB TEXMFCNF
Indicates a (colon-separated) list of directories to search for files named
.BR texmf.cnf ,
which are to be interpreted as configuration files.  An extra colon anywhere
in the list incorporates the compiled-in default value at that point.
See the section CONFIGURATION FILES for more details on configuration files,
how
.B xdvi
searches for them, and what they should contain.
#endif
.TP
.SB TEXMF
Indicates the top directory of \*(Te\& Directory Structure (TDS) trees to use
when searching for files.  It should be a list of directories, separated by
colons.  An extra colon anywhere in the variable incorporates the compiled-in
default value at that point.
See the section on FILE SEARCHING for more details.
.TP
.SB XDVISIZES
Indicates which sizes of fonts are available.
It should consist of a list of numbers separated by colons.
If the list begins with a colon, the system default sizes are used, as well.
Sizes are expressed in dots per inch and must be integers.
The current default set of sizes is %%DEFAULT_FONT_SIZES%%.
.B xdvi
will also try the actual size of the font before trying any of the given sizes.
.TP
.SB XDVIFONTS
Determines the path(s) searched for
.I pk
and
.I gf
font pixel files.  See the section on FILE SEARCHING for more details.
#iftexfonts
.TP
.SB PKFONTS
Determines the path(s) searched for
.I pk
and
.I gf
font pixel files if
.SB XDVIFONTS
is not set.
.TP
.SB TEXPKS
Determines the path(s) searched for
.I pk
and
.I gf
font pixel files if neither
.SB XDVIFONTS
nor
.SB PKFONTS
is set.
.TP
.SB TEXFONTS
Determines the path(s) searched for
.I pk
and
.I gf
font pixel files if none of
.SB XDVIFONTS,
.SB PKFONTS,
and
.SB TEXPKS
are set.  If this is used, it should not contain any
.RB ` % '
signs, since
.B xdvi
interprets this as a special character, but other applications do not.
#endif
.TP
.SB XDVIVFS
Determines the path(s) searched for virtual fonts
.RI ( vf
files).  See the section on FILE SEARCHING for more details.
#iftexfonts
.TP
.SB VFFONTS
Determines the path(s) searched for
.I vf
fonts if
.SB XDVIVFS
is not set.  If this is used, it should not contain any
.RB ` % '
signs, since
.B xdvi
interprets this as a special character, but other applications do not.
#endif
#ifps
.TP
.SB XDVIFIGS
Determines the path(s) searched for PostScript figure files.
See the section on FILE SEARCHING for more details.
.TP
.SB PSFIGURES
Determines the path(s) searched for PostScript figure files if
.SB XDVIFIGS
is not set.
.TP
.SB TEXINPUTS
Determines the path(s) searched for PostScript figure files if neither
.SB XDVIFIGS
nor
.SB XDVIFIGS
is set.
.TP
.SB XDVIHEADERS
Determines the path(s) searched for PostScript header files.
See the section on FILE SEARCHING for more details.
.TP
.SB TEXPSHEADERS
Determines the path(s) searched for PostScript header files if
.SB XDVIHEADERS
is not set.
.TP
.SB PSHEADERS
Determines the path(s) searched for PostScript header files if neither
.SB XDVIHEADERS
nor
.SB TEXPSHEADERS
is set.
#endif
#ifmakepk
.TP
.SB XDVIMAKEPK
Address of the program (and, optionally, the order of its arguments) to
be called when
.B xdvi
attempts to create a
.I gf
or
.I pk
font file.  See the section on CREATING FONT FILES for more details.
#endif
#ifps
.TP
.SB TMPDIR
The directory to use for storing temporary files created when uncompressing
PostScript files.
#endif
#ifconfig
.TP
.SB XDVIDEBUG
The initial (and default) value of the
.B \-debug
command line option.  Setting this option via the environment is the only way
to debug configuration file processing, which occurs before the command line
is processed.
#endif
.SH FILE SEARCHING
In order to accommodate the wide variety of ways in which fonts are stored
on various sites,
.B xdvi
has a fairly elaborate mechanism for indicating where to look for font files.
For other types of files, the mechanism is similar, but simpler.  The method
for looking for font pixel files will be described first; other file types will
then be described.  This section is quite technical; on first reading, it
would probably be better to skip to the section on EXAMPLES OF FONT SEARCHING.
.PP
The environment variable
.SB XDVIFONTS
(or
.SB PKFONTS,
etc., if
.SB XDVIFONTS
is not set) contains a list of specifiers, separated by colons.  An extra
colon anywhere in that list causes the compiled-in default value to be
substituted at that point.  Or, if no such environment variable is used,
the compiled-in default is also used instead.
#ifconfig
(However, see the section on CONFIGURATION FILES to see how they change the
situation concerning defaults.)
#endif
.PP
In each specifier, the following substitutions are first made:
.TP
.B %f
Replaced by the font name.
.TP
.B %F
Replaced by the font name (but without side effects; see below).
.TP
.B %d
Replaced by the size of the font (in dots per inch).
.TP
.B %b
Replaced by the base resolution; i.e., the value of the
.B \-p
parameter or the
.B .pixelsPerInch
resource.
.TP
.B %p
Replaced by the font file format
.RB (`` pk ''
or
.RB `` gf '').
.TP
.B %m
Replaced by the
.IR mode-def ,
as given in the
.B \-mfmode
argument or the
.B .mfMode
resource.
.TP
.B %t
Replaced, sequentially, by the directories given by the
.B TEXMF
environment variable (or its compiled-in default).  This may only be used
at the beginning of a specifier.
.TP
.B %s
Replaced by
.RB `` %qfonts/%p/{%m,gsftopk,ps2pk}// ''.
This is compatible with the \*(Te\& Directory Structure (TDS) standard.
This string may only be used at the end of a specifier.
.TP
.B %S
Replaced by
.RB `` %t/%s ''.
.TP
.B %q
Replaced by the empty string.  This has the side effect of enabling the
``quick find'' feature, which is described below.
.TP
.B %Q
Replaced by the empty string.  Like
.BR %q ,
this enables the ``quick find'' feature.  It also inhibits searching for the
file by normal means if ``quick find'' is not available.
.TP
.B %%
Replaced by a single percent sign.  Likewise,
.BR %: ,
.BR %* ,
etc. can be used to insert those special characters into the destination
string.
.PP
If no
.RB `` %f ''
appears in the specifier, then the string
.RB `` /%f.%d%p ''
is added on the end.
.PP
The characters
.BR * ,
.BR ? ,
.BR [ ,
.BR ] ,
.BR { ,
and
.B }
are interpreted as wild cards, as in the C-shell
.RB ( csh ).
(This is here to pave the way for
.I fli
files, which have not been implemented yet.)
In addition, a double slash
.RB (`` // '')
in the specifier indicates that any number of subdirectories may be inserted
at that point.
.PP
There is an exception to the above procedure.  If the font name begins
with a slash
.RB ( / ),
then the font name is treated as an absolute path:  the single specifier
.RB `` %f.%d%p ''
is used instead of the specifier(s) given by
.SB XDVIFONTS.
.PP
The recursive search over subdirectories triggered by a double slash often
causes a severe performance penalty; therefore,
.B xdvi
implements a speedup called ``quick find.''  This is triggered by the presence
of a
.RB `` %q ''
or
.RB `` %Q ''
in the specifier.  The location of such a string indicates that a file named
.B ls-R
should exist in that directory; that file should be the output of a
.B ls -R
or
.B ls -LR
command executed while in that directory.  If such a file exists, then
.B xdvi
will search that file instead of searching through the directory tree.
If such a file does not exist, and if
.RB `` %Q ''
was used, then
.B xdvi
will skip the specifier entirely.
.PP
In order for ``quick find'' to work,
a few conditions must be met.  First of all, the
.RB `` %q ''
or
.RB `` %Q ''
must occur immediately after a slash, and no later than immediately following
the double slash.  Secondly, there must be exactly one double slash in the
specifier (having more than one double slash requires more complicated
code in
.BR xdvi ;
if there are no double slashes then there is no need for ``quick find'').
Third, there may be no wild cards other than
.B {
and
.B }
in the specifier.  Finally,
.BR %f ,
.BR %F ,
and
.B %d
may not occur in the specifier prior to the double slash.  These conditions
are all satisfied in the case of the \*(Te\& Directory Structure (TDS) standard.
.PP
An additional exception is that if a specifier or one of the alternatives in the
.SB TEXMF
environment variable begins with two exclamation points
.RB (`` !! ''),
then those characters are stripped off, and any subordinate search that
could use an
.B ls-R
file, will be skipped if the
.B ls-R
file does not exist.  In other words, any
.RB `` %q ''
strings are treated as
.RB `` %Q ''.
This feature has been included for compatibility with the
.B kpathsea
library.
.PP
Finally, if a specifier or one of the alternatives in the
.SB TEXMF
environment variable begins with a tilde
.RB ( ~ )
(after the
.RB `` !! '',
if any), then
.B xdvi
will attempt to replace a string of the form
.BI ~ username
with the home directory of
.IR username .
The
.I username
is taken to be everything up through the next slash or the end of the string;
if it is empty, then the current user's home directory is substituted instead.
If the username does not exist, then the string is left unchanged.
.SH SEARCHING FOR FONTS
When
.B xdvi
searches for a font, the first thing it does is to look for a
.I pk
or
.I gf
file, at the size required for the
.I dvi
file, using the strategy mentioned in the above subheading.
#ifdosnames
If that fails, it will try again, but for specifiers lacking a string
.RB `` %f '',
it will add the string
.RB `` /dpi%d/%f.%p ''
at the end (instead of
.RB `` /%f.%d%p '').
#endif
It will also try a slightly different size, in case of rounding errors.
.PP
If no such bitmap file is found, it then searches for a virtual font.
(A virtual font is a recipe for creating a font from characters in other fonts
and from rectangles.)  This uses the procedure described under FILE SEARCHING,
except that:  (1) the environment variable
.SB XDVIVFS
or its associated defaults is used in place of the environment variable
.SB XDVIFONTS
or its associated defaults;
(2)
.RB `` %d '',
.RB `` %b '',
.RB `` %p '',
and
.RB `` %m ''
are not substituted;
(3)
.RB `` %s ''
is replaced by
.RB `` %qfonts/vf// '';
(4) if no
.RB `` %f ''
appears in a specifier, then
.RB `` /%f.vf ''
is added at the end; and finally
(5) if the file name begins with a slash, then
.RB `` %f.vf ''
replaces all the specifiers.
.PP
If no virtual font is found, then
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
font (see
.BR \-altfont ).
.SH EXAMPLES OF FONT SEARCHING
As a first example, if the specifier is
.RB `` /usr/local/tex/fonts ''
and the font is
.B cmr10
at 300 dots per inch, then
.B xdvi
searches for
.B /usr/local/tex/fonts/cmr10.300pk
and
.BR /usr/local/tex/fonts/cmr10.300gf ,
in that order (provided that
.B xdvi
is compiled to accept both
.I pk
and
.I gf
files, which is not necessarily the case).
.PP
For sites using the \*(Te\& Directory Structure (TDS) standard,
.SB XDVIFONTS
(or, better yet, its compiled-in default) should be set to
.RB `` .:%S '';
in that case, if
.SB TEXMF
(or, again, its compiled-in default) is set to
.RB `` /usr/local/texmf '',
then
.B xdvi
will look within that directory for the font file, in accordance with the
TDS standard.
.PP
There may be several such TDS trees.
.PP
A common situation is one in which a user wishes to augment the set of fonts
provided by the system.  It is possible to do this without having to know
or remember what the defaults are.  For example, if the user has a small
number of fonts, and keeps them all in one directory, say
.BR /home/user/fonts ,
then setting
.SB XDVIFONTS
to
.RB `` /home/user/fonts: ''
will cause
.B xdvi
to check that directory for font files before checking its default list.
Similarly, setting
.SB XDVIFONTS
to
.RB `` :/home/user/fonts ''
will cause
.B xdvi
to check that directory
.I after
checking its default locations.  This is true even if the system uses a TDS
tree.
#iftexfonts
.PP
If that directory also contains
.I tfm
files, then it is possible to set
.SB TEXFONTS
instead of
.SB XDVIFONTS;
in that case, \*(Te\& will also look for the
.I tfm
files in that directory.  This feature depends on which implementation
of \*(Te\& is in use.  The
.SB XDVIFONTS
variable overrides the
.SB TEXFONTS
variable, so that on those sites where
.SB TEXFONTS
must be set explicitly, and therefore this feature is not useful, the
.SB XDVIFONTS
variable may be set to an empty string (i.e.,
.RB  `` "setenv XDVIFONTS" '')
to cause
.B xdvi
to ignore
.SB TEXFONTS.
#endif
.PP
If the user has a large number of fonts and wishes to keep them in a TDS
tree, then that is also possible with
.BR xdvi :
if, for example, the TDS tree is
.BR /home/user/texmf ,
then setting
.SB TEXMF
to
.RB `` /home/user/texmf: ''
will cause
.B xdvi
to check that TDS tree before its default actions.  This assumes, however,
that the site uses a TDS tree also (since
.SB TEXMF
is not used unless
.RB `` %t ''
or
.RB `` %S ''
occurs in a specifier somewhere).  If the site does not use a TDS tree,
then it would be best to set
.SB XDVIFONTS
to
.RB `` /home/user/texmf/%s: '',
instead.
#ifmakepk
.SH CREATING FONT FILES
.PP
When
.B xdvi
reaches a point where it cannot find a font in the correct size, it calls
a program to create such a font file.  The name of this program (usually
a shell script) may be controlled by the environment variable
.SB XDVIMAKEPK.
Usually this variable would be set to the name of the script.
In that case the script is called with the following options:
(1) the font name, (2) the requested resolution in dots per inch,
(3) the base resolution in dots per inch, (4) a (possibly more accurate)
indication of the magnification using magsteps (if possible), (5) the
.I mode-def
that Metafont is to use when creating the font file, (6) an empty string,
and (7) a string of the form
.BI ``>& digit '',
enclosed in single quotes, where
.I digit
indicates a file number on which the program is to write the full path of the
font file that it has created.  Arguments (6) and (7) may be omitted, depending
on how
.B xdvi
was compiled.
.PP
The
.I mode-def
(argument (5)) used is the one given by the
.B \-mfmode
argument on the command line, the
.B mfMode
resource, or the compiled-in default (if any).  If none of these are given,
the string
.B default
is used.
.PP
Optionally, the
.SB XDVIMAKEPK
variable may include specifiers
.RB `` %n ,''
.RB `` %d ,''
.RB `` %b ,''
.RB `` %m ,''
.RB `` %o ,''
and
.RB `` %r ''
to indicate arguments (1)\-(5), and (7), respectively.  In that case, if a
.I mode-def
was given, but no
.RB `` %o ''
specifier appears in the
.SB XDVIMAKEPK
string, then the mode will be appended to the end of the string.
Also, if no
.RB `` %r ''
appears, then
.B xdvi
expects the program to write the full path of the font file on its standard
output.
.PP
This is compatible with the font creation mechanism used in
.BR dvips (1).
By default,
.SB XDVIMAKEPK
equals
.BR %%mkpk%% .
#endif
#ifps
.SH SEARCHING FOR POSTSCRIPT FILES
PostScript figure files and header files are searched for using a modification
of the procedure used for font files.
First, if the file name does not begin with a slash, then
.B xdvi
will look in the directory containing the
.I dvi
file.  If it is not found there, then the
.SB XDVIFIGS
and
.SB XDVIHEADERS
environment variables determine the search strategy for PostScript
figure files and header files, respectively.  This is the same procedure as
for font files, except that:
(1)
.RB `` %f ''
and
.RB `` %F ''
refer to the file name, not the font name;
(2)
.RB `` %d '',
.RB `` %b '',
.RB `` %p '',
and
.RB `` %m ''
are not substituted;
(3)
.RB `` %s ''
is replaced by
.RB `` {dvips,%Qtex//} ''
for figure files or
.RB `` dvips ''
for header files;
(4) if no
.RB `` %f ''
appears in a specifier, then
.RB `` /%f ''
is added at the end; and finally
(5) if the file name begins with a slash, then
.RB `` %f ''
replaces all the specifiers.
Customarily,
.SB XDVIFIGS
will be the same as the \*(Te\& input directory.
.PP
There is an additional exception to the above strategy.
If the file name begins with a backtick
.RB ( ` ),
then the remaining characters in the file name give a shell command (often
.BR zcat )
which is executed; its standard output is then sent to be interpreted as
PostScript.  Note that there is some potential for security problems here;
see the
.B \-allowshell
command-line option.  It is better to use compressed files directly (see below).
.PP
If a file name is given (as opposed to a shell command),
if that file name ends in
.RB `` .Z ''
or
.RB `` .gz '',
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
#ifconfig
.SH CONFIGURATION FILES
.B xdvi
allows for any number of configuration files; these provide additional levels
of defaults for file paths.  For example, when searching for font pixel files,
the hierarchy of defaults is as follows:
.TP
First
The value of the
.SB XDVIFONTS,
.SB PKFONTS,
.SB TEXPKS,
or
.SB TEXFONTS
environment variable (if any).
.TP
 .
The value of
.SB PKFONTS
given in the first config file searched.
.TP
 .
 ...
.TP
 .
The value of
.SB PKFONTS
given in the last config file searched.
.TP
Last
The compiled-in default value.
.PP
The first of these in the list that is given is used; if an extra colon is
present in that string, then the next in the list is used.  Additional extra
colons are ignored; if no extra colon is present, then the remainder of the
list is ignored.  Note that, unlike the situation with environment variables,
only one name may be used in the config file.  Additionally, if a config file
defines a variable such as
.SB PKFONTS
more than once, only the first value is used.
.PP
Other special config file variables are:
.TP 12
.SB TEXMF
Directories to substitute for
.BR %t .
But, see the caution below.
.TP
.SB VFFONTS
Search specifiers for virtual fonts.
#endif
#ifconfigps
.TP
.SB PSFIGURES
Search specifiers for PostScript figure files.
.TP
.SB PSHEADERS
Search specifiers for PostScript header files.
#endif
#ifconfig
.PP
The configuration file may also define other variables; these (as well as
.SB PKFONTS
and the other variables listed above) will be substituted whenever they are
referred to, using either the syntax
.BI $ variable\fR,\fP
in which
.I variable
consists of the longest string consisting of letters, digits, and underscores
following the dollar sign, or the syntax
.BI ${ variable }\fR,\fP
which is interpreted without matching the braces.  These substitutions occur
before brace expansions, which in turn occur before tilde expansions.
Substitutions of this sort are global:  only the first definition of a
variable is used, even if there are multiple configuration files.
This holds also when variables such as
.SB PKFONTS
are substituted via
.B $PKFONTS
instead of the hierarchy of defaults mentioned earlier.
Also, an environment variable will override a corresponding configuration
file variable.  Undefined variables are replaced with the empty string.
.PP
.B Note:
Some configuration files define a variable
.SB TEXMF
and access it via
.BR $TEXMF .
When using such files with
.BR xdvi ,
it may be necessary to avoid using
.B %t
and
.B $S
if the definition is incompatible with how that special symbol is used by
.BR xdvi .
For example,
.B xdvi
does not support brace expansion within
.SB TEXMF
when it is used in the context of
.BR %t ,
so if a configuration file defines
.SB TEXMF
as a string involving braces, then
.B $TEXMF/%s
should be used instead of
.BR %S .
#endif
#ifselfauto
.PP
When starting up,
.B xdvi
will determine where its executable file is located, and will define two
special configuration file variables based on that information.  
It defines
.SB SELFAUTODIR
to be the parent of the directory containing the binary, and
.SB SELFAUTOPARENT
to be the parent of that directory.  For example, if
.B xdvi
is in the executable file
.BR /usr/local/tex/texmf/bin/sunos4/xdvi ,
then the value of
.SB SELFAUTODIR
will be
.BR /usr/local/tex/texmf/bin ,
and
.SB SELFAUTOPARENT
will be
.BR /usr/local/tex/texmf .
These two variables are special:  they cannot be defined in a configuration
file or in the environment.
#endif
#ifcfg2res
.PP
Finally,
.B xdvi
allows the default values of certain resources to be set via the configuration
file.  The variables
.SB MFMODE,
.SB PIXELSPERINCH,
.SB SHRINKFACTOR,
#endif
#ifcfg2resbuttons
.SB SHRINKBUTTON1,
.SB SHRINKBUTTON2,
.SB SHRINKBUTTON3,
.SB SHRINKBUTTON4,
#endif
#ifcfg2res
and
.SB PAPER
define the default values for the
.BR mfMode ,
.BR pixelsPerInch ,
.BR shrinkFactor ,
#endif
#ifcfg2resbuttons
.BR shrinkButton1 ,
.BR shrinkButton2 ,
.BR shrinkButton3 ,
.BR shrinkButton4 ,
#endif
#ifcfg2res
and
.B paper
resources, respectively.  These defaults are overriden by any values given
in the app-defaults file, the resources obtained from the X server, and
the command line.
#endif
#ifconfig
.PP 
The syntax of configuration files is as follows.  Blank lines, and lines
in which the first non-white character is
.B #
or
.BR % ,
are assumed to be comment lines, and are ignored.  
All other lines must be of the form
.IB variable = value\fR,\fP
in which
.I variable
is either a variable name or a qualified variable name (discussed below), and
.I value
is the value to be assigned to the variable.  A qualified variable name
is a variable name, followed by a period and the name of a program.
If the name of the program is not the last part of the file name of the
executable file for
.B xdvi
(usually ``xdvi''), then that definition will be ignored.  
Since only the first definition of a variable will have any effect, an
unqualified definition should be placed after all qualified definitions
of that same variable.
.PP
There may be several configuration files.  They are located as follows.
Initially, an environment variable
.SB TEXMFCNF
is used; it should contain a colon-separated list of directories.
All of those directories will be searched for files named
.BR texmf.cnf ,
and those files will all be read.  Those files may also define variables
.SB TEXMFCNF;
if so, then those variables are taken to be colon-separated lists of
directories.  This defines the data structure of a tree; this tree is
read in the obvious depth-first order.  Duplicate directories are skipped
in order to avoid infinite loops.  Finally, an extra colon anywhere in any
of these lists causes the compiled-in default to be substituted at that
point.  In addition, if
.SB TEXMFCNF
is not defined in the environment, then the search starts with the compiled-in
default (it is expected that this will usually be the case).
.PP
If there are more than one
.B texmf
tree, then it is expected that each of them will have its own configuration
file; the easiest way to combine them is to link them together in a chain
(a vertical tree).  The last file in the chain can be set up as if it were
the only configuration file, and all others could contain a definition of
.SB TEXMFCNF
pointing to the directory of the next configuration file in the chain,
and definitions for
.SB TEXMF
or
.SB PKFONTS,
etc., with extra colons at the end so that the values from configuration
files lower in the chain are also used.
#endif
#ifps
.SH LIMITATIONS
.B xdvi
accepts many but not all types of PostScript specials accepted by
.BR dvips .
It accepts most specials generated by
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
#ifconfig
%%DEFAULT_CONFIG_PATH%%   Directories to be searched for configuration files.
#endif
%%DEFAULT_TEXMF_PATH%%   \*(Te\& Directory Structure (TDS) directories.
.TP 40
%%DEFAULT_FONT_PATH%%   Font pixel files.
%%DEFAULT_VF_PATH%%   Virtual font files.
#ifps
.TP
%%DEFAULT_FIG_PATH%%   Directories containing PostScript figures.
.TP
%%DEFAULT_HEADER_PATH%%   Directories containing PostScript header files.
#endif
.PD
.SH SEE ALSO
.BR X (1).
.SH AUTHORS
Eric Cooper, CMU, did a version for direct output to a QVSS.
Modified for X by Bob Scheifler, MIT Laboratory for Computer Science.
Modified for X11 by Mark Eichin, MIT SIPB.
Maintained and enhanced since 1988 by Paul Vojta, UC Berkeley, and many others.
