# This file is sourced by Xsession(5), not executed.

# Work around the absence of the message function (to pretty-print user
# messages) from some versions of gdm's Xsession script.
if ! hash message 2>/dev/null ; then
  message () {
    text="$@"
    echo "$text" | fold -s
    if [ -n "$DISPLAY" ]; then
      if hash zenity 2>/dev/null ; then
        zenity --info --text "$text"
      elif hash xmessage 2>/dev/null ; then
        echo "$text" | fold -s | xmessage -center -file -
      fi
    fi
  }
fi
