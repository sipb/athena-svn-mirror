# Standard Athena .login file for new users.
#
#	MIT Project Athena
#
#	RCS Information:
#
#	$Source: /afs/dev.mit.edu/source/repository/packs/dotfiles/dot.login,v $
#	$Header: /afs/dev.mit.edu/source/repository/packs/dotfiles/dot.login,v 1.9 1987-12-06 22:14:08 treese Exp $

# Set initialization directory and make sure it exists.

set INITDIR = /usr/athena/lib/init

if (! -d ${INITDIR} && $?interactive) then
	echo "System-wide initialization files not found."
	echo "Some initialization has not been performed."
endif

# Source login initialization files:
#	env_variables -- standard environment variables
#	login_init -- standard login initialization
#	init_terminal -- initialize terminal settings
#	init_windows -- initialization X window system
#	check_mail -- check for new mail
#	login_misc -- miscellaneous login activities
#
# If you do not want one (or more) of these things to happen, delete the
# lines below.
#
# WARNING: if you delete any of the lines below, you will not automatically
# get any changes that Project Athena may make to those files at a later
# date.  Be sure you know what you are doing.

if (-r ${INITDIR}/env_variables) source ${INITDIR}/env_variables
if (-r ${INITDIR}/login_init) source ${INITDIR}/login_init
if (-r ${INITDIR}/init_terminal) source ${INITDIR}/init_terminal
if (-r ${INITDIR}/init_windows) source ${INITDIR}/init_windows
if (-r ${INITDIR}/check_mail) source ${INITDIR}/check_mail
if (-r ${INITDIR}/login_misc) source ${INITDIR}/login_misc

# Finally, source personal login commands.  By keeping them in a separate
# file, you can easily keep up to date with Athena changes.

if (-r ~/.login.mine) source ~/.login.mine


