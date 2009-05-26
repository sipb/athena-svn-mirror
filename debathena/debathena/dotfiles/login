# Prototype global .login file
# $Id: login,v 1.15 2007-08-22 18:11:08 ghudson Exp $

# This file is sourced by default user file ~/.login

# Remember the pid of the login shell; used by dustbuster.
setenv ATHENA_LOGIN_SESSION $$

set initdir=/usr/lib/init

if (! -r ~/.hushlogin) then
	set verbose_login	# Echo messages unless user has ~/.hushlogin.
endif
/bin/stty sane			# Use sane control characters

# *******************   ENVIRONMENT SETUP   *******************

# The ~/.cshrc file sets environment variables, attaches lockers, and sets
# the command search path.  It has already been sourced at this point.


# *******************   DEVICE SETUP   *******************

# Set device type if interactive session (e.g., dialup)

if (("$term" == switch) || ("$term" == network) || ("$term" == dialup) \
    || ("$term" == unknown)) then

    # The standard terminal emulation is vt100.  To use a different emulation,
    # set the environmental variable DEF_TERM in your ~/.environment file to
    # to the device you want to emulate (one that the system will recognize).

    if (! $?DEF_TERM) setenv DEF_TERM vt100
    set noglob; eval `tset -s -I -Q "?${DEF_TERM}"` || unset noglob

endif


# *******************   ACTIVITY STARTUP   *******************

# Run standard system/user startup activities

# Run default system startup activities.  To skip these, put the command
# "set skip_tty_startup" in your ~/.environment file.

if (! $?ZEPHYR_CLIENT) then
	if (! -f /etc/athena/dialuptype || "fallback: true" == "`zctl show fallback`") then
		setenv ZEPHYR_CLIENT zwgc
	else
		setenv ZEPHYR_CLIENT 'echo This is a dialup, so zwgc is not being run on login.'
	endif
endif

if (! $?skip_tty_startup) then
	if ($?verbose_login) echo "Running standard startup activities ..."
	set ignoreeof			# ^D won't log you out
	$ZEPHYR_CLIENT			# Start Zephyr client
	get_message -new -login		# Display current motd
	if ("`hesinfo $USER pobox`" !~ "*EXCHANGE*") then
	  mailquota -n			# Check quota on post office server
	  from.debathena -t -n		# Check for mail
	endif
endif


# Run the tty-session customizing commands in your ~/.startup.tty file.

if (-r ~/.startup.tty) then
	if ($?verbose_login) then		# Don't echo if noninteractive
	  echo "Running custom startup activities listed in ~/.startup.tty ..."
	endif
	source ~/.startup.tty
endif


# Run notification system, lert. To skip this (a generally bad idea unless
# you plan to run it yourself), put the command "set skip_lert" in your
# ~/.environment file.

if (! $?skip_lert) then
	lert -q				# Don't want to see server errors.
endif
