#!/bin/sh

type=`/usr/athena/libexec/filters/check_unprintable`

cat << EOF
%!
/font { findfont 16 scalefont setfont } bind def

54 720 moveto /Times-Bold font
(Error: spooled $type file rejected) show

54 670 moveto /Times-Roman font
(Consult the OLC stock answers \() show
/Courier-Roman font (olc answers ) show
/Times-Roman font (at the) show
/Courier font ( athena% ) show
/Times-Roman font (prompt\)) show

54 650 moveto
(for more information on how to print this file.) show
showpage
EOF
