#!/bin/athena/bash --norc
# Global xsession file for bash users
# Accepts one optional argument, either "nocalls" (use no user files)
# or "dash" (use old dash-based interface).
#
# $Id: xsession.bash,v 1.1 2008-05-08 18:02:32 ghudson Exp $

initdir=/usr/lib/init
gconftool=/usr/bin/gconftool-2

source_if_exists() {
  [ -r "$1" ] && . "$1"
}

echo_if_verbose() {
  [ t = "$verbose_login" ] && echo "$@"
}

# Determine whether to use user customizations:  use no customization files
# ONLY if the single argument "nocalls" was provided to xsession (e.g., if
# the user selected the "Ignore your customizations" option from the
# initial xlogin menu).

case $# in
0)
  ;;
1)
  if [ nocalls = "$1" -o x-nocalls = "x$1" ]; then
    echo -n "xsession: Running standard xsession "
    echo "with NO user configuration files..."
    export NOCALLS=
  elif [ dash = "$1" ]; then
    echo -n "xsession: Running standard xsession "
    echo "with dash-based interface..."
    export ATHENA_DASH_INTERFACE=t
  else
    bad_args=
  fi
  ;;
*)
  bad_args=t
  ;;
esac

if [ t = "$bad_args" ]; then
  echo "xsession: Arguments ($*) not recognized."
  echo "Running standard xsession with user configuration files..."
  unset NOCALLS
  unset bad_args
fi

# Echo messages unless user has ~/.hushlogin file AND the session was not 
# started using the xlogin "SYSTEM" option.

if [ ! -r "$HOME/.hushlogin" -o "${NOCALLS+set}" = set ]; then
  verbose_login=t
fi

# Use old dash-based interface if the user has
# ~/.athena_dash_interface and the session was not started with
# NOCALLS.

if [ -r "$HOME/.athena_dash_interface" -a "${NOCALLS+set}" != set ]; then
  export ATHENA_DASH_INTERFACE=
fi

# *******************   ENVIRONMENT SETUP   *******************


# Identify this as a window session.

export XSESSION=$$

# The ~/.bashrc file sets environment variables, attaches lockers, and sets
# the command search path.

echo_if_verbose "Setting up environment..."

# Source the system startup scripts if necessary.
source_if_exists /etc/profile
source_if_exists /etc/bashrc
source_if_exists /etc/bash.bashrc

if [ "${NOCALLS+set}" = set ]; then
  source_if_exists "$initdir/bashrc"
else
  if [ -r "$HOME/.bashrc" ]; then
    . "$HOME/.bashrc"
  else
    source_if_exists "$initdir/bashrc"
  fi
fi

if [ "${ATHENA_DASH_INTERFACE+set}" = set ]; then
  default_windowmanager=mwm
  terminal='xterm -geometry 80x48+0+38'
else
  default_windowmanager=metacity
  terminal='gnome-terminal --geometry=80x40-0-52'

  # GNOME processes can apparently collide trying to start gconfd, so
  # spawn one before we start any in the background.
  $gconftool --spawn
fi


# *******************   DEVICE SETUP   *******************

# Load user X resources.

if [ "${NOCALLS+set}" != set -a -r "$HOME/.Xresources" ]; then
  xrdb -merge ~/.Xresources
fi


# *******************   ACTIVITY STARTUP   *******************

# Start window manager.  To change your default window manager, set the 
# WINDOW_MANAGER environment variable in your ~/.bash_environment file.
if [ "${WINDOW_MANAGER+set}" != set ]; then
  if [ -r "$HOME/.athena-sawfish" ]; then
    export WINDOW_MANAGER=sawfish
  else
    export WINDOW_MANAGER=$default_windowmanager
  fi
fi

echo_if_verbose "Starting $WINDOW_MANAGER window manager..."

if [ "${NOCALLS+set}" = set ]; then
  (metacity >/dev/null 2>&1 &)
else
  ($WINDOW_MANAGER >/dev/null 2>&1 &)
fi

# Run standard system/user startup activities

if [ "${ATHENA_DASH_INTERFACE+set}" != set ]; then
  # Run gnome-settings-daemon unless the user has chosen to skip it.
  # (We used to run a series of capplets with an init flag; thus the
  # skip variable is named skip_capplets.)
  if [ "${skip_capplets+set}" != set ]; then
    echo_if_verbose "Initializing settings..."

    # This incantation causes gnome-settings-daemon to be run via
    # Bonobo, such that we won't continue until the settings-daemon
    # object has been initialized and all settings have been
    # propagated.  If we just naively run gnome-settings-daemon in the
    # background, settings may not propagate before GNOME programs
    # run.  In particular, GNOME programs might render text before the
    # Xft settings have been propagated, leading to incorrect font
    # selection.
    activation-client -s "iid == 'OAFIID:GNOME_SettingsDaemon'" >/dev/null
  fi

  # Run the GNOME panel unless the user has chosen to skip it.
  if [ "${skip_panel+set}" != set ]; then
    echo_if_verbose "Starting panel..."
    (gnome-panel >/dev/null 2>&1 && kill -HUP "$XSESSION" &)
  fi

  # If you decide to set skip_nautilus, GNOME will stop taking care
  # of your background image unless you run:
  #   gconftool-2 -s -t bool /apps/nautilus/preferences/show_desktop false
  if [ "${skip_nautilus+set}" != set ]; then
    (nautilus -n >/dev/null 2>&1 &)
  fi

elif [ "${skip_dash+set}" != set ]; then
  if [ "${skip_xlogout+set}" != set ]; then
    echo_if_verbose "Starting the Athena Dashboard..."
    echo_if_verbose "Creating logout button..."
    (dash -default -logout &)
  else
    echo_if_verbose "Starting the Athena Dashboard..."
    (dash &)
  fi
elif [ "${skip_xlogout+set}" != set ]; then
  echo_if_verbose "Creating logout button..."
  (dash -logout &)
fi

# Start default initial xterm window.  To skip this, put the command
# "skip_initial_xterm=t" in your ~/.bash_environment file.

# BE CAREFUL:  If you skip this, make sure that your ~/.startup.X file
# provides a way for you to exit your session (e.g., an xterm from which
# you can type "logout"). 

if [ "${skip_initial_xterm+set}" != set ]; then
  echo_if_verbose "Creating initial xterm window..."
  ($terminal >/dev/null 2>&1 &)
fi

# Run default system startup activities.  To skip these, put the command
# "skip_x_startup=t" in your ~/.bash_environment file.

if [ "${skip_x_startup+set}" != set ]; then
  echo_if_verbose "Running standard startup activities..."

  # Start Zephyr client, and if zephyr started okay, send message of
  # the day as windowgram
  : ${ZEPHYR_CLIENT=zwgc}
  if [ "${NOCALLS+set}" = set ]; then
    zwgc -f /etc/zephyr/zwgc.desc -subfile /dev/null && \
      get_message -new -zephyr
  else
    $ZEPHYR_CLIENT && get_message -new -zephyr
  fi
  if [ "${skip_quotawarn+set}" != set ]; then
    ($initdir/quotawarn &)	# Display warning dialogs if near quota
  fi
  from -t -n			# Check for mail
  if [ "${skip_xss+set}" != set ]; then
    (xscreensaver -no-splash &)
  fi
  if [ "${skip_authwatch+set}" != set ]; then
    (authwatch &)
  fi
fi

# Maybe start the Disco-Athena daemon.
if [ -r /var/run/athstatusd.sock -a "${skip_athneteventd+set}" != set ]; then
  athneteventd
fi

# Run the window-session customizing commands in your ~/.startup.X
# file.  We want to background this process in case something in the
# startup hangs.  Thus, the user will not be left with active windows
# and no method of easily logging out.

# We also run the notification system, lert. Ideally, we wish the user
# to receive the alert in the form of a zephyrgram, and since they may
# run zwgc from their .startup.X, we wait until that has finished.  To
# skip lert (a generally bad idea unless you plan to run it yourself -
# it is not at all intended for frivolous messages), put the command
# "skip_lert=t" in your ~/.bash_environment file.

if [ "${NOCALLS+set}" != set -a -r "$HOME/.startup.X" ]; then
  echo_if_verbose "Running custom startup activities listed in ~/.startup.X..."
  ( . "$HOME/.startup.X"; \
    if [ "${skip_lert+set}" != set ]; then lert -q -z; fi; \
    echo_if_verbose "Session initialization completed." &)
else
  if [ "${skip_lert+set}" != set ]; then
    lert -q -z
  fi
  echo "Session initialization completed."
fi


# Gate the session.

# This command replaces this script and will wait for you to log out.
# To terminate this process, logout (type the "logout" command in any
# xterm window, use the Logout button, or select the "Logout of
# Athena" option from Dash), or invoke the "end_session" program.

if [ "${NOCALLS+set}" = set ]; then
  exec session_gate
else
  exec session_gate -logout
fi

# The session gate will source .logout when it gets a SIGHUP, if
# invoked with the -logout flag.
