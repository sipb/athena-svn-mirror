# Standard Athena .cshrc file for new users.
#
#	MIT Project Athena
#
#	RCS Information:
#
#	$Source: /afs/dev.mit.edu/source/repository/packs/dotfiles/dot.cshrc,v $
#	$Header: /afs/dev.mit.edu/source/repository/packs/dotfiles/dot.cshrc,v 1.7 1987-12-06 22:14:21 treese Exp $

# Set the initialization directory pathname to find initialization files.

set INITDIR = /usr/athena/lib/init

# Now check to see if this is an interactive shell.  If it isn't (e.g.,
# this is an rsh, we want to know about it.

if ($?prompt) set interactive

# Make sure that the initialization directory is present

if (! -d ${INITDIR} && $?interactive) then
	echo "System-wide initialization files not found."
	echo "Some initialization has not been performed."
	set path = (. /usr/athena /bin/athena /usr/bin/X /usr/new \
			 /usr/new/mh/bin /usr/ucb /bin /usr/bin)
endif

# Source the various standard initialization files:
#
#	set_path -- sets the standard Athena command search path
#	shell_init -- standard csh initialization (e.g., history)
#	aliases -- standard aliases
#	misc -- miscellaneous initialization
#
# All commands in those files can be overridden later by using the
# 'unset' or 'unalias' commands, or by changing things using 'set'
# or 'alias' again.

if (-r ${INITDIR}/set_path) source ${INITDIR}/set_path
if (-r ${INITDIR}/shell_init) source ${INITDIR}/shell_init
if (-r ${INITDIR}/aliases) source  ${INITDIR}/aliases
if (-r ${INITDIR}/misc) source ${INITDIR}/misc

# Finally, source the personal startup file.  By keeping the commands
# separate, you can easily keep up with changes in this file.

if (-r ~/.cshrc.mine) source ~/.cshrc.mine
