#!/bin/sed -f
#
# Fix up file names which are either invalid on MSDOS, or clash with each
# other in the restricted 8+3 file-name space
#
s/americansml/amersml/g
s/americanmed/amermed/g
s/americanlrg/amerlrg/g
s/americanxlg/amerxlg/g
s/altamersml/altasml/g
s/altamermed/altamed/g
s/altamerlrg/altalrg/g
s/altamerxlg/altaxlg/g
s/britishsml/britsml/g
s/britishmed/britmed/g
s/britishlrg/britlrg/g
s/britishxlg/britxlg/g
s/\(..*\)-alt\./alt\1./g
s/+\.hash/x.hash/g
s/\([^.]\)\.\([^.+][^.+]*\)+/\1x.\2/g
s/americanx/americax/g
#
# Make sure Bash uses Unix-style PATH
#
/^SHELL *=/i\
PATH_SEPARATOR=:
