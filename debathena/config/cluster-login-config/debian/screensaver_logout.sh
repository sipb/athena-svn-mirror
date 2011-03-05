#!/bin/sh
#
# A more ... insistent ... logout command for gnome-screensaver
#

PATH=/usr/bin:/bin:$PATH

if [ "$(machtype -L)" != "debathena-cluster" ]; then
    gnome-session-save --force-logout
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
    gnome-session-save --force-logout
else
    pkill schroot
fi
    
