#!/bin/sh
# $Id: gnome-panel.sh,v 1.1 2003-07-24 18:48:44 ghudson Exp $

# gnome-panel - Wrapper around /usr/bin/panel to ensure that the
# Athena GNOME libraries will take preference over the native ones,
# and to display a warning if this is not the case (since we cannot
# put this wrapper around all native GNOME programs).

usrlibfirst='\(\(.*:\)*/usr/lib\(:.*\)*:/usr/athena/lib\(:.*\)*\)$'
usrathena='\(\(.*:\)*/usr/athena/lib\(:.*\)*\)$'

if [ -n "`expr "$LD_LIBRARY_PATH" : "$usrlibfirst"`" ]; then
  problem="puts /usr/lib before /usr/athena/lib"
elif [ -z "`expr "$LD_LIBRARY_PATH" : "$usrathena"`" ]; then
  problem="does not contain /usr/athena/lib"
else
  unset problem
fi

if [ "${problem+set}" = set ]; then
  dialog_text="
  Your dotfiles appear to set LD_LIBRARY_PATH to a value which  
  ${problem}.  This will lead to broken  
  behavior from GNOME programs.  Please change your dotfiles to  
  put /usr/athena/lib first in LD_LIBRARY_PATH.  If you need  
  assistance, contact Athena On-Line Consulting by running the  
  command 'olc'.  

  LD_LIBRARY_PATH value is:  
    ${LD_LIBRARY_PATH:-(empty)}  
"
  gdialog --msgbox "$dialog_text" 10 65 &
  export LD_LIBRARY_PATH="/usr/athena/lib${LD_LIBRARY_PATH:+:}$LD_LIBRARY_PATH"
fi

exec -a "$0" /usr/bin/gnome-panel "$@"
