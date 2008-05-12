# Global bashrc file
#
# $Id: bashrc,v 1.39 2006-07-17 23:14:07 rbasch Exp $

# This file is sourced by default user file ~/.bashrc

initdir=/usr/lib/init

# add alias moved here to avoid having two copies of it.

add () { eval "$( /bin/attach -Padd -b "$@" )" ; }


# *******************   ENVIRONMENT SETUP   *******************



# Set up standard system/user environment configuration (including setup of
# environment variables, attachment of lockers, and setting of search path)

# If we see ENV_SET set to empty, we could be a tcsh user who has
# decided to run bash, or we could be a bash user suffering from the
# misfeature that the standard xsession script runs the tcsh dotfiles
# for all users.  Running the environment setup for the former
# category of user would be unfriendly (it resets the homedir and
# changes the path), so for now, only run environment setup for bash
# users.  If the xsession problem is ever fixed, change this
# conditional to check for '"${ENV_SET+set}" != set' and eliminate the
# shell check.
if [ "${ENV_SET:+set}" != set -a "${SHELL##*/}" = bash ]; then

	export ENV_SET=t			# Avoid unnecessary repeat
	export HOSTTYPE="`/bin/machtype`"

	if [ -r "$HOME/.generation" ]; then
		export ATHENA_DOTFILE_GENERATION=`cat "$HOME/.generation"`
	else
		export ATHENA_DOTFILE_GENERATION=0
	fi

	umask 077				# Strictly protect files
						#  (does not apply in AFS)
	ulimit -S -c 0				# Don't allow coredumps
	export EDITOR=emacs			# Set default editor
	export VISUAL=emacs			# Set default screen editor
	export MM_CHARSET=iso-8859-1

	export MORE=-s

	# Set double-sided printing for sufficiently recent users.
	if [ 1 -le "$ATHENA_DOTFILE_GENERATION" ]; then
		export LPROPT=-Zduplex
	fi

	export ATHENA_SYS=`/bin/machtype -S`
	export ATHENA_SYS_COMPAT=`/bin/machtype -C`

	if [ -z "$ATHENA_SYS" ]; then
		export ATHENA_SYS=@sys
	fi

	if [ "${PRINTER+set}" != set -a -e /var/athena/clusterinfo ]; then
		PRINTER=`awk '/LPR/ { print $3 }' /var/athena/clusterinfo`
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
		. ~/.bash_environment
	fi

	# Standard Athena path modifications
	athena_home_bin=$( /usr/bin/athdir "$HOME" )
	PATH=${athena_home_bin:+$athena_home_bin:}$PATH:.
	unset athena_home_bin

fi


# *******************  BASH SETUP   *******************

# Set up standard bash shell initializations

set -o noclobber		# Don't overwrite files with redirection

#   alias for re-establishing authentication
renew () { kinit -54 $USER && fsid -a && zctl load /dev/null ; }

#   alias for a convenient way to change terminal type
term () { set -f; unset TERMCAP; eval "$( tset -s -I -Q "$@" )"; set +f; }

#   aliases dealing with x window system
xresize () { set -f; eval "$( resize -u )"; set +f ; }

if [ "${XSESSION+set}" = set ]; then
	if [ -z "$XSESSION" ]; then
		logout () { end_session && exit; }		# logout for X
	else
		logout () { kill -HUP $XSESSION && exit; }	# logout for X
	fi
fi

#   aliases dealing with adding locker programs

alias setup='echo "setup is not supported in bash yet"'


# All of the bash initializing commands above can be overridden by using
# "unset" or "unalias" commands (or by changing things using "set" or
# "alias" again) in your ~/.bashrc.mine file, which is sourced here.
# ~/.bashsrc.mine is not sourced if the xlogin "Ignore your customizations"
# option was selected to begin the session.

if [ "${NOCALLS+set}" != set -a -r ~/.bashrc.mine ]; then
	. ~/.bashrc.mine
fi
