Copyright (C) 1994 by Mariusz Olko.  All rights reserved.
Copyright (C) 1997 by Mariusz Olko and Marcin Woli\'nski.
Copyright (C) 2000,2002,2003 by Marcin Woli\'nski.

Bundle `PLaTeX' provides tools to typeset documents in Polish using
LaTeX2e.  Documented source code is in *.dtx and *.fdd files.

`PLaTeX' is free software redistributable in according to LaTeX
Project Public License version 1 or above (see lppl.txt in base LaTeX
distribution).

To install `PLaTeX' run TeX on instalation script platex.ins
    tex platex.ins
which will extract individual pieces of the bundle from the *.dtx
files.

During the instalation you will be asked question whether you have
`PL' fonts installed on your system or not. If you have them
installation script will generate a number of font description files
for Polish fonts.  In case you don't have those font the ot4cmr.fd
file should not be present on the system, as it will confuse
environment scanning routine in `polski.sty'.

More info can be found in documentation of polski.sty source code.  To
obtain DVI file with documentation run
	latex polski.dtx
