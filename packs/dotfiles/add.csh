#!/dev/null
#
# $Id: add.csh,v 1.27 1996-06-03 01:32:07 cfields Exp $
#
# add <addargs> <-a attachargs> <lockername> <lockername> ...
#
#	-v	verbose
#	-v0	verbose w/o debugging
#	-d	debugging
#	-n	specify new attach behavior
#	-f	add lockers to the front of the path
#	-m	move lockers in path if they're already in the path
#	-r	remove lockers from path
#	-p	print path environment filtered
#	-w	give warning for adds with no bindirs
#	-e	perform operations for the .environment file (changing
#		  $athena_path, $athena_manpath instead of $PATH,
#		  etc.)
#	-a	pass further options to attach
#

# alias add 'set add_opts = (\!:*); source $initdir/add'

set add_vars=( add_vars add_usage add_verbose add_front add_warn add_env \
               add_opts add_attach add_dirs add_bin add_bindir \
               add_man add_mandir add_print add_path add_manpath \
               add_new add_oldverbose add_move add_remove add_debug \
               add_arg add_newpath add_newmanpath add_i )

set add_usage = "Usage: add [-v] [-v0] [-d] [-n] [-f] [-m] [-r] [-p] [-w] [-e] [-a attachflags]         [lockername] [lockername] ..."

#
# Parse options
#

if ( $#add_opts == 0 ) set add_print

if ( $?add_flags ) then
  set add_opts = ( $add_flags $add_opts )
endif

while ( $#add_opts > 0 )
  set add_arg = $add_opts[1]

  switch ( $add_arg )

    case -v:
      set add_verbose
      set add_oldverbose
      breaksw

    case -v0:
      set add_verbose
      breaksw

    case -d:
      set add_debug
      breaksw

    case -n:
      set add_new
      breaksw

    case -f:
      set add_front
      breaksw

    case -m:
      set add_move
      breaksw

    case -r:
      set add_remove
      breaksw

    case -p:
      set add_print
      breaksw

    case -w:
      set add_warn
      breaksw

    case -e:
      set add_env
      breaksw

    case -a:
      shift add_opts
      if ( $#add_opts ) then
        set add_attach = "$add_opts"
        set add_opts=
      else
        echo "add: options required after -a"
        echo "$add_usage"
        goto finish
      endif
      breaksw

    default:
      if ( "$add_opts[1]" =~ -* ) then
        echo "add: unrecognized option: $add_opts[1]"
        echo "$add_usage"
        goto finish
      endif

      if ( $#add_opts ) then
        set add_attach = "$add_opts"
        set add_opts=
      endif
  endsw
  shift add_opts

end

if ( $?add_oldverbose && ! $?add_new ) set add_debug

#
# Try to make our environment sane.
#

if ( ! $?ATHENA_SYS ) then
  setenv ATHENA_SYS `machtype -S`
  if ( $ATHENA_SYS == "" ) then
    setenv ATHENA_SYS `fs sysname | awk -F\' '{ print $2 }'`
  endif
  if ( $ATHENA_SYS == "" ) setenv ATHENA_SYS "@sys"
endif

set add_bindir = arch/$ATHENA_SYS/bin
set add_mandir = arch/$ATHENA_SYS/man

if ( ! $?bindir ) then
  set bindir = `machtype`bin
  if ( $bindir == "bin" ) set bindir = $add_bindir
endif

#
# Print the filtered path and exit.
#

if ( $?add_print ) then
  echo $path | sed -e "s-/mit/\([^/]*\)/$add_bindir-{add \1}-g" \
             | sed -e "s-/mit/\([^/]*\)/$bindir-{add \1}-g"
  goto finish
endif

#
# Call attach. Once for a normal add, twice for a verbose to get
# interesting output from attach.
#

if ( $?add_new ) then

#
# New, sane behavior for cases with no authentication and
# fascist lockers, and more reliable output.
#

  if ( $?add_verbose ) then
    attach $add_attach
    set add_dirs = `attach -h -p $add_attach`
  else
    set add_dirs = `attach -p $add_attach`
  endif
else

#
# Old behavior.
#

  if ( $?add_verbose ) then
    attach -n -h $add_attach
  endif
  set add_dirs = `attach -p $add_attach`
endif

#
# For the non-environment add, generate a prototype for the new path.
#
if ( ! $?add_env ) then
  set add_newpath = $PATH
  set add_newmanpath = $MANPATH
endif

#
# Loop through all of the lockers attach told us about.
#

set add_path
set add_manpath

foreach add_i ($add_dirs)
  unset add_bin
  unset add_man

#
# Find the bin directory
#

  if ( -d $add_i/$add_bindir ) then
    set add_bin = $add_i/$add_bindir
  endif

  if ( ! $?add_bin ) then
    if ( -d $add_i/$bindir ) then
      set add_bin = $add_i/$bindir
    endif
  endif

#
# Find the man directory
# Don't use arch/man unless you actually have architecture
# dependent man pages in your locker.
#

  if ( -d $add_i/$add_mandir ) then
    set add_man = $add_i/$add_mandir
  endif

  if ( ! $?add_man ) then
    if ( -d $add_i/man ) then
      set add_man = $add_i/man
    endif
  endif

#
# If we are supposed to move or remove path elements, do it.
#
  if ( ! $?add_env && ( $?add_move || $?add_remove ) ) then
    if ( $?add_bin ) then
      if ( "$add_newpath" =~ *"$add_bin"* ) then
        set add_newpath = `echo $add_newpath | sed -e "s-:${add_bin}--g" | sed -e "s-${add_bin}:--g"`  
        if ( $?add_debug ) echo $add_bin removed from \$PATH
      endif
    endif
    if ( $?add_man ) then
      if ( "$add_newmanpath" =~ *"$add_man"* ) then
        set add_newmanpath = `echo $add_newmanpath | sed -e "s-:${add_man}--g" | sed -e "s-${add_man}:--g"`  
        if ( $?add_debug ) echo $add_man removed from \$MANPATH
      endif
    endif
  endif

  if ( ! $?add_remove ) then
#
# Add the bin and man directories, as appropriate, to the head or
# tail of the path, to PATH and MANPATH or athena_path and athena_manpath.
#

    if ( $?add_bin || $?add_man ) then
      switch ( $?add_env$?add_front )

        case 00:
          if ( $?add_bin ) then
            if ( "$add_newpath" !~ *"$add_bin"* ) then
              if ( $?add_debug ) echo $add_bin added to end of \$PATH
              set add_path = ${add_path}:$add_bin
            endif
          endif

          if ( $?add_man ) then
            if ( "$add_newmanpath" !~ *"$add_man"* ) then
              if ( $?add_debug ) echo $add_man added to end of \$MANPATH
              set add_manpath = ${add_manpath}:$add_man
            endif
          endif
          breaksw

        case 01:
          if ( $?add_bin ) then
            if ( "$add_newpath" !~ *"$add_bin"* ) then
              if ( $?add_debug ) echo $add_bin added to front of \$PATH
              set add_path = ${add_bin}:$add_path
            endif
          endif

          if ( $?add_man ) then
            if ( "$add_newmanpath" !~ *"$add_man"* ) then
              if ( $?add_debug ) echo $add_man added to front of \$MANPATH
              set add_manpath = ${add_man}:$add_manpath
            endif
          endif
          breaksw

        case 10:
          if ( $?add_bin ) then
            if ( "$athena_path" !~ *"$add_bin"* ) then
              if ( $?add_debug ) echo $add_bin added to end of \$athena_path
              set athena_path = ($athena_path $add_bin)
            endif
          endif

          if ( $?add_man ) then
            if ( "$athena_manpath" !~ *"$add_man"* ) then
              if ( $?add_debug ) echo $add_man added to end of \$athena_manpath
              set athena_manpath = ${athena_manpath}:$add_man
            endif
            if ( "$MANPATH" !~ *"$add_man"* ) then
              if ( $?add_debug ) echo $add_man added to end of \$MANPATH
              setenv MANPATH ${MANPATH}:$add_man
            endif
          endif
          breaksw

        case 11:
          if ( $?add_bin ) then
            if ( "$athena_path" !~ *"$add_bin"* ) then
              if ( $?add_debug ) echo $add_bin added to front of \$athena_path
              set athena_path = ($add_bin $athena_path)
            endif
          endif

          if ( $?add_man ) then
            if ( "$athena_manpath" !~ *"$add_man"* ) then
              if ( $?add_debug ) echo $add_man added to front of \$athena_manpath
              set athena_manpath = ${add_man}:$athena_manpath
            endif
            if ( "$MANPATH" !~ *"$add_man"* ) then
              if ( $?add_debug ) echo $add_man added to front of \$MANPATH
              setenv MANPATH ${add_man}:$MANPATH
            endif
          endif
          breaksw
      endsw
    endif

    if ( ! $?add_bin && $?add_warn ) then
      echo add: warning: $add_i has no binary directory
    endif
  endif
end


if ( ! $?add_env ) then
  if ( $?add_path ) then
    if ( $?add_front ) then
      setenv PATH ${add_path}${add_newpath}
    else
      setenv PATH ${add_newpath}${add_path}
    endif
  endif

  if ( $?add_manpath ) then
    if ( $?add_front ) then
      setenv MANPATH ${add_manpath}${add_newmanpath}
    else
      setenv MANPATH ${add_newmanpath}${add_manpath}
    endif
  endif
endif

finish:

#
# We were never here.
#

foreach add_i ( $add_vars )
  unset $add_i
end
