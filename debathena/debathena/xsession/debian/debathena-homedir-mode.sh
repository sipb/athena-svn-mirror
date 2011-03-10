# This file is sourced by Xsession(5), not executed.

if [ afs = "$DEBATHENA_HOME_TYPE" ] && \
  stat -L --format='%A' "$HOME" | egrep -q '^(.....w|........w)'; then
  hm=$(stat -L --format='%a' "$HOME")
  if zenity --question --text="Your home directory is mode $hm, probably \
because it was created a long time ago.  You likely received a spurious \
error dialog about .dmrc as a result of the mode bits on your home \
directory.  To fix this condition, you can run: 
        chmod 755 \$HOME 

Would you like to run this command now?"; then
      chmod 755 $HOME
  fi
  unset hm
fi
