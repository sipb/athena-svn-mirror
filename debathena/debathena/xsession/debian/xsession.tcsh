#!/usr/bin/tcsh -f

# This is the Athena xsession wrapper for tcsh users.  Its job is to:
# 1. Set the XSESSION environment variable.
# 2. Process the user's dotfiles including .environment.
# 3. Run standard startup programs if the user has not opted out.
# 4. Run the user's .startup.X.

set initdir=/usr/lib/init

setenv XSESSION $$

if (-r /etc/.login) source /etc/.login
if (-r /etc/.cshrc) source /etc/.cshrc
if (-r /etc/csh.login) source /etc/csh.login
if (-r /etc/csh.cshrc) source /etc/csh.cshrc

if (-r ~/.cshrc) then
  source ~/.cshrc
else 
  if (-r ${initdir}/cshrc) source ${initdir}/cshrc
endif

if (! $?skip_initial_xterm) then
  setenv ATHENA_INITIAL_X_TERMINAL
endif

if (! $?skip_x_startup) then
  if (! $?ZEPHYR_CLIENT) setenv ZEPHYR_CLIENT zwgc
  $ZEPHYR_CLIENT
  ($initdir/displaymotd &)
  if (! $?skip_quotawarn) then
    ($initdir/quotawarn &)	# Display warning dialogs if near quota
  endif
  if (! $?skip_authwatch) then
    (authwatch &)
  endif
endif

if (! $?skip_lert) then
  ($initdir/displaylert &)
endif

if (-r ~/.startup.X) then
  (source ~/.startup.X &)
endif

# Proceed with the session command, which has been passed as arguments.
exec $argv:q
