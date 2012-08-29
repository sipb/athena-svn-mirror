#!/bin/sh
#
# A more ... insistent ... logout command for gnome-screensaver
#

PATH=/usr/bin:/bin:$PATH

GSESSION_END="gnome-session-quit --logout --no-prompt"
if ! hash gnome-session-quit 2>/dev/null; then
    GSESSION_END="gnome-session-save --force-logout"
fi

if [ "$(machtype -L)" != "debathena-cluster" ]; then
    $GSESSION_END
    exit 0
fi

cell=athena.mit.edu
afspath=$HOME
if echo $HOME | grep -q ^/mit; then
    # Shouldn't happen with std dotfiles
    afspath=$(readlink $HOME)
fi
if echo $afspath | grep -q ^/afs; then
    cell=$(echo $afspath | cut -d/ -f 3)
fi
if tokens | fgrep -q $cell; then
    $GSESSION_END
else
    pkill schroot
fi
    
