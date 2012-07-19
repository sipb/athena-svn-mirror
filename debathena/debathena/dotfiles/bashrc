# Global bashrc file
#
# $Id: bashrc,v 1.39 2006-07-17 23:14:07 rbasch Exp $

# This file is sourced by default user file ~/.bashrc

initdir=/usr/lib/init

# Determine if we're in an sftp or scp session and if so, be quiet
SILENT=no
case "$BASH_EXECUTION_STRING" in
    */usr/lib/openssh/sftp-server)
	SILENT=yes
	;;
    scp\ *)
	SILENT=yes
	;;
esac

# *******************   ENVIRONMENT SETUP   *******************

# Set up standard system/user environment configuration (including setup of
# environment variables, attachment of lockers, and setting of search path)

if [ "${ENV_SET:+set}" != set ]; then

	export ENV_SET=t			# Avoid unnecessary repeat
	export HOSTTYPE="`/bin/machtype`"

	# Ensure user's homedir is attached, for legacy things
	# that care about attachtab
	# Only attach if running as an Athena user, not e.g. using sudo.
	if [ "$DEBATHENA_HOME_TYPE" = afs ]; then
		/bin/attach -h -q "${ATHENA_USER:-$USER}"
	fi

	if [ -r "$HOME/.generation" ]; then
		export ATHENA_DOTFILE_GENERATION=`cat "$HOME/.generation"`
	else
		export ATHENA_DOTFILE_GENERATION=0
	fi

	ulimit -S -c 0				# Don't allow coredumps
	export EDITOR=emacs			# Set default editor
	export MM_CHARSET=iso-8859-1

	export EMAIL="$USER@mit.edu"		# Set default email address

	export MORE=-s

	export ATHENA_SYS=`/bin/machtype -S`
	export ATHENA_SYS_COMPAT=`/bin/machtype -C`

	if [ -z "$ATHENA_SYS" ]; then
		export ATHENA_SYS=@sys
	fi

	if [ "${PRINTER+set}" != set \
	  -a -e /var/run/athena-clusterinfo.csh ]; then
		PRINTER=`awk '/LPR/ { print $3 }' \
		  /var/run/athena-clusterinfo.csh`
		if [ -n "$PRINTER" ]; then export PRINTER; fi
	fi

	# Reset the HOME variable to correspond to the actual location
	# of the user's home directory.  This will avoid having long
	# pathnames being printed when /mit/<user> is a symlink to a
	# path within AFS.

	x=`(cd && /bin/pwd) 2>/dev/null`
	if [ -n "$x" ]; then
		export HOME=$x
		if [ "$x" = "`/bin/pwd`" ]; then
			cd "$HOME"
		fi
	fi
	unset x

	# Run user environment customizations identified in your
	# ~/.bash_environment file.  This is the place to include your
	# own environment variables, attach commands, and other system
	# wide setup commands.  You can also cancel default behaviors
	# listed above.
	# ~/.bash_environment is not sourced if NOCALLS is set (i.e.,
	# if you selected the xlogin "Ignore your customizations"
	# option when you logged in).

	if [ "${NOCALLS+set}" != set -a -r ~/.bash_environment ]; then
	    if [ "$SILENT" = "yes" ]; then
		. ~/.bash_environment &> /dev/null
	    else
		. ~/.bash_environment
	    fi
	fi

	# If the user has a bindir in $HOME, put it in front of the path.
	athena_home_bin=$( /usr/bin/athdir "$HOME" )
	PATH=${athena_home_bin:+$athena_home_bin:}$PATH
	unset athena_home_bin

fi


# *******************  BASH SETUP   *******************

# Set up standard bash shell initializations

if [ "${XSESSION+set}" = set ]; then
   if [ -x /usr/bin/gnome-session-quit ]; then
	logout () { /usr/lib/init/check-for-reboot; gnome-session-quit --no-prompt; exit; } # logout for X
   else   
	logout () { /usr/lib/init/check-for-reboot; gnome-session-save --kill --silent; exit; } # logout for X
   fi
fi

#   aliases dealing with adding locker programs

setup_tty () {(export SUBJECT="$@"; $SHELL)}
setup_X () {(export SUBJECT="$@"; xterm -title "$@" &)}

if [ -n "$XSESSION" ]; then
   alias setup=setup_X
else
   alias setup=setup_tty
fi

remove () { export SUBJECT="$@"; 
       	    source $initdir/env_remove.bash; 
	    unset SUBJECT; }

if [ -n "$SUBJECT" -a -r $initdir/env_setup.bash ]; then
	source $initdir/env_setup.bash
fi


# All of the bash initializing commands above can be overridden by using
# "unset" or "unalias" commands (or by changing things using "set" or
# "alias" again) in your ~/.bashrc.mine file, which is sourced here.
# ~/.bashsrc.mine is not sourced if the xlogin "Ignore your customizations"
# option was selected to begin the session.

if [ "${NOCALLS+set}" != set -a -r ~/.bashrc.mine ]; then
    if [ "$SILENT" = "yes" ]; then
	. ~/.bashrc.mine &> /dev/null
    else
	. ~/.bashrc.mine
    fi
fi
if [ "${skip_sanity_checks+set}" != set ]; then
    missing=0
    echo $PATH | /usr/bin/tr ':' '\n' | /bin/grep -Fqx /bin
    [ $? -eq 0 ] || missing=1
    echo $PATH | /usr/bin/tr ':' '\n' | /bin/grep -Fqx /usr/bin
    [ $? -eq 0 ] || missing=1
    if [ $missing -eq 1 ]; then
	text="You appear to have incorrectly modified your PATH variable in your dotfiles,\nand as such have deleted /bin and/or /usr/bin from your PATH, which\nwill likely cause this login session to fail.  Please correct this problem.\nThis warning can be disabled with 'skip_sanity_checks=t' in ~/.bashrc.mine."
	echo -e "$text" >&2
	[ -n "$DISPLAY" ] && /usr/bin/zenity --warning --text="$text"
    fi
    if [ -n "$LD_ASSUME_KERNEL" ]; then
	unset LD_ASSUME_KERNEL
	text="In your dotfiles, you set LD_ASSUME_KERNEL.  This generally causes undesirable behavior.\nIt has been unset for you.\nIf you really wanted it set, you can add 'skip_sanity_checks=t' to your ~/.bashrc.mine."
	echo -e "$text" >&2
	[ -n "$DISPLAY" ] && /usr/bin/zenity --warning --text="$text"
    fi
fi

	
