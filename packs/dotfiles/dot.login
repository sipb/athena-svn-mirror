# Standard Athena .login file for new users.
#
#	MIT Project Athena
#
#	RCS Information:
#
#	$Source: /afs/dev.mit.edu/source/repository/packs/dotfiles/dot.login,v $
#	$Header: /afs/dev.mit.edu/source/repository/packs/dotfiles/dot.login,v 1.2 1987-07-30 16:58:01 treese Exp $

# First, source the standard login script.  If it can't be found, notify
# the user and execute some backup commands.  In particular, set the path
# to something close to reality.

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

# If the login is on an X display, and the DISPLAY variable is set,
# ask the user if he wants a window manager.  If the DISPLAY variable
# has not been set, ask the user what the DISPLAY should be.

if ( $term =~ xterm* ) then
	if ($?DISPLAY) then
		echo -n "Starting window manager: "
		uwm &
	else
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

set noglob; eval `tset -s -Q -m 'switch>1200:?vt100 -m 'switch<=1200:?vt100'`
