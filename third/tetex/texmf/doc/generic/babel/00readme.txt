                       Babel Distribution Guide

                            10 March 1999


Welcome to the Babel system!

This file contains the distribution guide for version 3.6 of the
Babel system.

The Babel system supports multilingual typesetting.
This version of the Babel system is compatible with LaTeX2e.
Whenever the instructions talk about LaTeX, read LaTeX2e.

This system is maintained by Johannes Braams in cooperation with
various people around the world.

The Babel system is described in:

 * The LaTeX Companion, Goossens; Mittelbach and Samarin, Addison-Wesley


This distribution is described in the files ending with .txt.  You
should read install.txt before starting to install Babel.

 * 00readme.txt is this file.

 * changes.txt is a chronological list of the changes to babel

 * install.txt describes how to install Babel.
 * install.mac describes how to install Babel on a Mac with OzTeX.

 * manifest.bbl lists all the files in the Babel distribution.

 * bugs.txt describes how to submit a bug report for Babel.

 * todo.txt lists a few things that haven't been done yet

 * howto.get lists the places where to get the Babel system and
             other related software.

You are not allowed to change the files in this distribution.


If you like the babel system, please send me a postcard with a nice
postage stamp for my collection!

Please do not request updates from me.  Distribution is done only
through mail servers and TeX organisations.


Please send bug reports to the LaTeX bug reporting address, 
latex-bugs@uni-mainz.de. Please read the file bugs.txt in this
distribution and follow the guidelines.

WARNINGS:
---------
- The file wnhyphen.tex (russian hyphenation pattern file) contains
  changes of uppercase and lowercase codes. Amongst these is
  \uccode`\~=`\^ which may lead to unexpected results when the ~
  occurs in the argument of \uppercase
- The file francais.dtx (francais.ldf) has been replaced by
  frenchb.dtx (frenchb.ldf). When upgrading from an older version of
  babel you need to remove francais.ldf.

Send any suggestions, additions, complaints, to me:
e-mail:  JLBraams@cistron.nl
address: Kooienswater 62
         2715 AJ Zoetermeer
         The Netherlands
Note that I am more likely to respond to e-mail.
--- Copyright 1999 Johannes Braams.  All rights reserved ---


