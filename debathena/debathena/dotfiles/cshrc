# Global cshrc file
#
# $Id: cshrc,v 1.12 2007-08-22 18:12:33 ghudson Exp $

# This file is sourced by default user file ~/.cshrc


set initdir=/usr/lib/init

# Determine if we're in an sftp or scp session, and if so, be quiet
set SILENT=no
if ($?command) then
  switch ("$command")
    case '*/usr/lib/openssh/sftp-server':
      set SILENT=yes
      breaksw
    case 'scp *':
      set SILENT=yes
      breaksw
  endsw
endif

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

# Set up standard system/user environment configuration (including setup of
# environment variables, attachment of lockers, and additions to search path)

if (! $?ENV_SET) then

  setenv ENV_SET				# Avoid unnecessary repeat

  # Ensure user's homedir is attached, for legacy things
  # that care about attachtab
  # Only attach if running as an Athena user, not e.g. using sudo.
  if ( $?DEBATHENA_HOME_TYPE ) then
    if ( $DEBATHENA_HOME_TYPE == afs ) then
      if ( $?ATHENA_USER ) then
        /bin/attach -h -q "$ATHENA_USER"
      else
        /bin/attach -h -q "$USER"
      endif
    endif
  endif

  if (-r ~/.generation) then
    setenv ATHENA_DOTFILE_GENERATION `cat ~/.generation`
  else
    setenv ATHENA_DOTFILE_GENERATION 0
  endif

  limit coredumpsize 0            	# Don't allow coredumps
  setenv MORE -s			# Default "more" behavior
					# we are now down to -s
					# because -d is wrong.
  setenv EDITOR emacs			# Set default editor
  setenv MM_CHARSET iso-8859-1

  setenv EMAIL "$USER@mit.edu"		# Set default email address

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

  # NOTE:
  # The use of the "extend", "sextend", and "textend" aliases in
  # ~/.environment is no longer supported.  Consult the Athena
  # documentation on Dotfiles for supported dotfile operations

  # Run user environment customizations identified in your
  # ~/.environment file.  This is the place to include your own
  # environment variables, attach commands, and other system wide
  # setup commands.  You can also cancel default behaviors listed
  # above with "unsetenv" or "setenv".  ~/.environment is not sourced
  # if NOCALLS is set (i.e., if you selected the xlogin "Ignore your
  # customizations" option when you logged in).

  if ((! $?NOCALLS) && (-r ~/.environment)) then
    if ( $SILENT == "yes" ) then
      source ~/.environment >&/dev/null
    else
      source ~/.environment
    endif
  endif

  # NOTE:
  # .path files are no longer supported, consult Athena documentation
  # for the correct way to specify your PATH
  
  # Standard Athena path additions.
  set path=(`/usr/bin/athdir $HOME` $path .)

endif

# Set appropriate bin directory variable for this platform
# (e.g., vaxbin for VAXstations, decmipsbin for pMAXen, etc.;  this will
# be included in actual searchpath as ~/$bindir -- e.g., ~/vaxbin):
set bindir=arch/${ATHENA_SYS}/bin

# *******************  C SHELL SETUP   *******************

# Set up standard C shell initializations

set noclobber			# Don't overwrite files with redirection

if ($?prompt) then		# For interactive shells only (i.e., NOT rsh):
  set interactive		#   Provide shell variable for compatability
endif

# Set up standard C shell aliases

if ($?XSESSION) then
  if ( -x /usr/bin/gnome-session-quit ) then
    alias logout	'exit && gnome-session-quit --no-prompt'	# logout for X
  else
    alias logout	'exit && gnome-session-save --kill --silent'	# logout for X
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
alias remove 'setenv SUBJECT \!* ; source $initdir/env_remove ; unsetenv SUBJECT'

# If this is a subject window, run the env_setup script
if (($?SUBJECT) && (-r $initdir/env_setup)) source $initdir/env_setup


# All of the C shell initializing commands above can be overridden by
# using "unset" or "unalias" commands (or by changing things using
# "set" or "alias" again) in your ~/.cshrc.mine file, which is sourced
# here.  ~/.cshrc.mine is not sourced if the xlogin "Ignore your
# customizations" option was selected to begin the session.

if ((! $?NOCALLS) && (-r ~/.cshrc.mine)) then
  if ( $SILENT == "yes" ) then
    source ~/.cshrc.mine >&/dev/null
  else
    source ~/.cshrc.mine
  endif
endif

if (! $?skip_sanity_checks) then
  set missing=0
  echo $path | /usr/bin/tr ':' '\n' | /bin/grep -Fqx /bin
  if ( $? != 0 ) then
    set missing=1
  endif
  echo $path | /usr/bin/tr ':' '\n' | /bin/grep -Fqx /usr/bin
  if ( $? != 0 ) then
    set missing=1
  endif
  if ( $missing == 1 ) then
    set text="You appear to have incorrectly modified your PATH variable in your dotiles,\nand as such have deleted /bin and/or /usr/bin from your PATH, which\nwill likely cause this login session to fail.  Please correct this problem.\nThis warning can be disabled with 'set skip_sanity_checks=t' in ~/.cshrc.mine."
    echo "$text" > /dev/stderr
    if ( $?DISPLAY) then
      /usr/bin/zenity --warning --text="$text"
    endif
  endif
  if ( $?LD_ASSUME_KERNEL ) then
    unsetenv LD_ASSUME_KERNEL
    set text="In your dotfiles, you set LD_ASSUME_KERNEL.  This generally causes undesirable behavior.\nIt has been unset for you.\nIf you really wanted it set, you can add 'set skip_sanity_checks=t' to your ~/.cshrc.mine."
    echo "$text" > /dev/stderr
    if ( $?DISPLAY) then
      /usr/bin/zenity --warning --text="$text"
    endif
  endif     
endif
