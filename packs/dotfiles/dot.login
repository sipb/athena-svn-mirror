# Standard Athena .login file for new users.
#
#	MIT Project Athena
#
#	RCS Information:
#
#	$Source: /afs/dev.mit.edu/source/repository/packs/dotfiles/dot.login,v $
#	$Header: /afs/dev.mit.edu/source/repository/packs/dotfiles/dot.login,v 1.6 1987-08-22 22:27:38 rfrench Exp $

# First, source the standard login script.  If it can't be found, notify
# the user and execute some backup commands.

if (-r /usr/athena/.login) then
	source /usr/athena/.login
else
	echo "Unable to load system-wide login file."
	echo "Some initialization has not been performed."
endif

# Set environment variables

setenv MORE -cs				# default "more" program behavior
setenv EDITOR /usr/athena/emacs		# Set default editor
setenv VISUAL /usr/athena/emacs		# Set default screen editor
setenv RS1HOME ~/rs1home		# Default RS/1 directory

# Set parameters for login shell only

set ignoreeof			# ^D won't logout
umask 66			# Files are not readable or writable by anyone
				# except owner (per privacy committee).

# Miscellaneous startup commands

stty dec			# Set DEC-style control characters

# If the login is on an X display, and the DISPLAY variable is set, start a
# window manager.  If the DISPLAY variable has not been set, ask the user
# what the DISPLAY should be.

if ( $term =~ xterm* ) then
	if ( `tty` =~ ttyv* ) then
		echo -n "Starting window manager: "
		uwm &
	else if (! $?DISPLAY) then
		echo -n "What DISPLAY are you using [default: none]? "
		set response = ($<)
		if ($response != "") then
			if ($response =~ *:*) then
				echo "Setting DISPLAY to $response"
				setenv DISPLAY $response
			else
				echo "Setting DISPLAY to ${response}:0"
				setenv DISPLAY ${response}:0
			endif
		endif
	endif
endif

# The following obscurity gets terminal characteristics into the
# environment:

set noglob; eval `tset -s -I -Q -m 'switch:?vt100' -m 'network:?vt100'`

# If you want to use the Zephyr notification system, uncomment the indicated
# lines below, as well as the indicated line in .logout.  Note that Zephyr
# is EXPERIMENTAL software at this time and is not yet a standard part of
# the Athena environment.
#
# Uncomment the next three lines to use Zephyr
# /usr/athena/zlogin >& /dev/null
# /usr/etc/zwgc &
# (sleep 20; /usr/athena/zinit) &
