# Global cshrc file
#
# $Id: cshrc,v 1.12 2007-08-22 18:12:33 ghudson Exp $

# This file is sourced by default user file ~/.cshrc


set initdir=/usr/lib/init

# *******************   ENVIRONMENT SETUP   *******************

# Compatibility with older versions of Athena tcsh
set autolist=""
if ($?tcsh) then
  bindkey "^W" backward-delete-word
  bindkey "^Z" complete-word
  bindkey " " magic-space

endif

# Set host type and hostname variables.
if (! $?ATHENA_HOSTTYPE) setenv ATHENA_HOSTTYPE "`/bin/machtype`"
if (! $?HOST) setenv HOST "`hostname`"
setenv HOSTTYPE "$ATHENA_HOSTTYPE"
set hosttype=$HOSTTYPE
set host=$HOST

# add alias for attaching lockers
alias add 'eval `/bin/attach -Padd \!:*`'

# Set up standard system/user environment configuration (including setup of
# environment variables, attachment of lockers, and additions to search path)

if (! $?ENV_SET) then

  setenv ENV_SET				# Avoid unnecessary repeat

  if (-r ~/.generation) then
    setenv ATHENA_DOTFILE_GENERATION `cat ~/.generation`
  else
    setenv ATHENA_DOTFILE_GENERATION 0
  endif

  umask 077				# Strictly protect files
					#  (does not apply in AFS)
  limit coredumpsize 0            	# Don't allow coredumps
  setenv MORE -s			# Default "more" behavior
					# we are now down to -s
					# because -d is wrong.
  setenv EDITOR emacs			# Set default editor
  setenv VISUAL emacs			# Set default screen editor
  setenv MM_CHARSET iso-8859-1

  # Set double-sided printing for sufficiently recent users.
  if ( $ATHENA_DOTFILE_GENERATION >= 1 ) then
    setenv LPROPT -Zduplex
  endif

  setenv ATHENA_SYS `/bin/machtype -S`
  if ( $ATHENA_SYS == "" ) then
    setenv ATHENA_SYS @sys
  endif
  setenv ATHENA_SYS_COMPAT `/bin/machtype -C`

  set bindir=arch/${ATHENA_SYS}/bin

  if ( ! $?PRINTER && -e /var/run/athena-clusterinfo.csh ) then
    setenv PRINTER `awk '/LPR/ { print $3 }' /var/run/athena-clusterinfo.csh`
    if ( $PRINTER == "" ) unsetenv PRINTER
  endif

  # Reset the "home" and HOME variables to correspond to the actual
  # location of the user's home directory.  This will avoid having
  # long pathnames being printed when /mit/<user> is a symlink to a
  # path within AFS.

  set x=`(cd && /bin/pwd)`
  if ("$x" != "") then
    set home=$x
    if ("$x" == "`/bin/pwd`") then
      set cwd=$x
    endif
  endif
  unset x

  # This "extend" alias and friends have been left in for backwards
  # compatibility with old .environment files, just in case. The new
  # add alias does not use them.
  alias extend 'if (-d \!:2) if ("$\!:1" \!~ *"\!:2"*) set extendyes && \\
  if ($?extendyes && $?verboseadd) echo \!:2 added to end of \$\!:1 && \\
  if ($?extendyes) setenv \!:1 ${\!:1}:\!:2 && \\
  unset extendyes'
  alias sextend 'if (-d \!:2) if ("$\!:1" \!~ *"\!:2"*) set extendyes && \\
  if ($?extendyes && $?verboseadd) echo \!:2 added to end of \$\!:1 && \\
  if ($?extendyes) set \!:1=(${\!:1} \!:2) && \\
  unset extendyes'
  alias textend 'if (-d \!:2) if ("$\!:1" \!~ *"\!:2"*) set extendyes && \\
  if ($?extendyes && $?verboseadd) echo \!:2 added to end of \$\!:1 && \\
  if ($?extendyes) set \!:1=${\!:1}:\!:2 && \\
  unset extendyes'

  # Run user environment customizations identified in your
  # ~/.environment file.  This is the place to include your own
  # environment variables, attach commands, and other system wide
  # setup commands.  You can also cancel default behaviors listed
  # above with "unsetenv" or "setenv".  ~/.environment is not sourced
  # if NOCALLS is set (i.e., if you selected the xlogin "Ignore your
  # customizations" option when you logged in).

  if ((! $?NOCALLS) && (-r ~/.environment)) source ~/.environment

  unalias extend sextend textend

  if ((! $?NOCALLS) && (-r ~/.path)) then
    # Support .path files for compatibility.
    set athena_path=($path)
    source ~/.path
  else
    # Standard Athena path additions.
    set path=(`/usr/bin/athdir $HOME` $path .)
  endif

endif

# Set appropriate bin directory variable for this platform
# (e.g., vaxbin for VAXstations, decmipsbin for pMAXen, etc.;  this will
# be included in actual searchpath as ~/$bindir -- e.g., ~/vaxbin):
set bindir=arch/${ATHENA_SYS}/bin

# *******************  C SHELL SETUP   *******************

# Set up standard C shell initializations

set noclobber			# Don't overwrite files with redirection

if ($?prompt) then		# For interactive shells only (i.e., NOT rsh):
  # Set prompt.
  set promptchars="%#"
  set prompt = "athena%# "
  set cdpath = (~)
  set interactive		#   Provide shell variable for compatability
endif

# Set up standard C shell aliases

#   alias for re-establishing authentication
alias renew 'kinit -54 $USER && fsid -a && zctl load /dev/null'

#   alias for a convenient way to change terminal type
alias term 'set noglob; unsetenv TERMCAP; eval `tset -s -I -Q \!*`'

#   aliases dealing with x window system
alias xresize 'set noglob; eval `resize -c` || unset noglob'

if ($?XSESSION) then
  if ("$XSESSION" == "") then
    alias logout	'exit && end_session'		# logout for X
  else
    alias logout	'exit && kill -HUP $XSESSION'	# logout for X
  endif
endif

#   aliases dealing with subjects
alias setup_X '( setenv SUBJECT \!:1 ; ( xterm -title \!* & ) )'
alias setup_tty '( setenv SUBJECT \!* ; $SHELL )'
if ($?XSESSION) then
  alias setup setup_X
else
  alias setup setup_tty
endif
alias remove 'setenv SUBJECT \!* ; source $initdir/env_remove'

# If this is a subject window, run the env_setup script
if (($?SUBJECT) && (-r $initdir/env_setup)) source $initdir/env_setup


# All of the C shell initializing commands above can be overridden by
# using "unset" or "unalias" commands (or by changing things using
# "set" or "alias" again) in your ~/.cshrc.mine file, which is sourced
# here.  ~/.cshrc.mine is not sourced if the xlogin "Ignore your
# customizations" option was selected to begin the session.

if ((! $?NOCALLS) && (-r ~/.cshrc.mine)) source ~/.cshrc.mine
