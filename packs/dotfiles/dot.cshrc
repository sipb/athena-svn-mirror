# Standard Athena .cshrc file for new users.
#
#	MIT Project Athena
#
#	RCS Information:
#
#	$Source: /afs/dev.mit.edu/source/repository/packs/dotfiles/dot.cshrc,v $
#	$Header: /afs/dev.mit.edu/source/repository/packs/dotfiles/dot.cshrc,v 1.5 1987-08-08 16:44:43 treese Exp $

# First, source the standard csh startup script.  If it can't be found, notify
# the user and execute some backup commands.  In particular, set a sane path.
# Note that this path may be out of date, but it should be good enough in
# an emergency.

if (-r /usr/athena/.cshrc) then
	source /usr/athena/.cshrc
else
	echo "Unable to load system-wide cshrc file."
	echo "Some initialization has not been performed."
	set athena_path = ( /usr/athena /bin/athena /usr/new \
			 /usr/new/mh/bin /usr/ucb /bin /usr/bin /usr/games)
endif

# The personal bin directory to use is determined by the machine type:
# rtbin for RT/PC's, vaxbin for VAXstations.
# Note that the variable athena_path is set in the standard Athena .cshrc
# file. It contains the standard system directories, which may be changed
# in the future.  Additional directories should be added at the desired
# place in the following command:

set bindir = `/bin/athena/machtype`bin
set path = (. $HOME/$bindir $athena_path)

# Set important CSH variables

set history = 20			# Save 20 past commands.
set cdpath = (~)			# Search home directory on cd's
set noclobber				# Don't overwrite existing files on
					# redirection

# Limit resources

limit coredumpsize 0

# Set the prompt depending on the hostname

if ($?prompt) then
	set prompt = "`hostname`% "
endif

# Useful aliases

alias mail	'Mail'
alias Mail	"echo 'Mail' is obsolete -- use 'inc' to read mail or 'comp' to send it; echo"

# To enable T-shell line editing, uncomment the following line
# set lineedit

