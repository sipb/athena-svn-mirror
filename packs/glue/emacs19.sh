#!/bin/sh
# $Id: emacs19.sh,v 1.1 1997-01-29 20:26:32 ghudson Exp $

echo "You have invoked the command 'emacs19' from /usr/athena/bin.  The"
echo "command 'emacs19' was introduced late in the 7.7 release to help users"
echo "transition from emacs 18.  Today, you should just run 'emacs'.  Please"
echo "modify your dotfiles or scripts accordingly.  This script will exist for"
echo "the lifetime of the Athena 8.1 release; the 'emacs19' command will go"
echo "away when Athena 8.2 is released (which will probably be in the summer"
echo "of 1998)."
echo ""
echo "Executing emacs now."
exec /usr/athena/bin/emacs "$@"
