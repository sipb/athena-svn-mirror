#!/bin/bash --norc

# This is the Athena xsession wrapper for bash users.  Its job is to:
# 1. Set the XSESSION environment variable.
# 2. Process the user's dotfiles including .bash_environment.
# 3. Run standard startup programs if the user has not opted out.
# 4. Run the user's .startup.X.

initdir=/usr/lib/init

source_if_exists() {
  [ -r "$1" ] && . "$1"
}

echo_if_verbose() {
  [ t = "$verbose_login" ] && echo "$@"
}

export XSESSION=$$

source_if_exists /etc/profile
source_if_exists /etc/bashrc
source_if_exists /etc/bash.bashrc
if [ -r "$HOME/.bashrc" ]; then
  . "$HOME/.bashrc"
else
  source_if_exists "$initdir/bashrc"
fi

# Start default initial xterm window.  To skip this, put the command
# "skip_initial_xterm=t" in your ~/.bash_environment file.

if [ "${skip_initial_xterm+set}" != set ]; then
  echo_if_verbose "Creating initial xterm window..."
  (gnome-terminal --geometry=80x40-0-0 >/dev/null 2>&1 &)
fi

if [ "${skip_x_startup+set}" != set ]; then
  : ${ZEPHYR_CLIENT=zwgc}
  $ZEPHYR_CLIENT
  ($initdir/displaymotd &)
  if [ "${skip_quotawarn+set}" != set ]; then
    ($initdir/quotawarn &)	# Display warning dialogs if near quota
  fi
  if [ "${skip_authwatch+set}" != set ]; then
    (authwatch &)
  fi
fi

if [ "${skip_lert+set}" != set ]; then
  ($initdir/displaylert &)
fi

if [ -r "$HOME/.startup.X" ]; then
  ( . "$HOME/.startup.X" &)
fi

# Proceed with the session command, which has been passed as arguments.
exec "$@"
