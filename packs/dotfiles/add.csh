#!/dev/null
#
# add <addargs> <-a attachargs> <lockername> <lockername> ...
#
#	-v	verbose
#	-f	add lockers to the front of the path
#	-p	print path environment filtered
#	-w	give warning for adds with no bindirs
#	-e	perform operations for the .environment file (changing
#		  $athena_path, $athena_manpath instead of $PATH,
#		  etc.)
#	-a	pass further options to attach
#
# fix bugs section of attach manpage

# alias add 'set add_opts = (\!:*); source /afs/dev/user/cfields/add'

# MANPATH search too

set add_vars=(add_vars add_usage add_verbose add_front add_warn add_env \
              add_opts add_attach add_dirs add_bin add_bindir \
              add_man add_mandir add_print add_path add_arg add_i)

set add_usage = "Usage: add [-v] [-f] [-p] [-w] [-e] [-a attachflags] [lockername] ..."

while ( $#add_opts > 0 )
  set add_arg = $add_opts[1]

  switch ($add_arg)

    case -v:
      set add_verbose
      breaksw

    case -f:
      set add_front
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

set ATHENA_SYS = `fs sysname | awk -F\' '{ print $2 }'`

set add_bindir = arch/$ATHENA_SYS/bin
set add_mandir = arch/$ATHENA_SYS/man

#
# Print the filtered path and exit.
#

if ( $?add_print ) then
  echo $PATH | sed -e "s-/mit/\([^/]*\)/$add_bindir-{add \1}-g"
  goto finish
endif

#
# Call attach. Once for a normal add, twice for a verbose to get
# interesting output from attach.
#

if ( $?add_verbose ) attach -n -h $add_attach

set add_dirs = `attach -p $add_attach`

#
# Loop through all of the lockers attach told us about.
#

set add_path

foreach add_i ($add_dirs)
  unset add_bin
  unset add_man

#
# Find the bin directory
#

  if ( -d $add_i/$add_bindir ) then
    set add_bin = $add_i/$add_bindir
  else
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
  else
    if ( -d $add_i/man ) then
      set add_man = $add_i/man
    endif
  endif

#
# Add the bin and man directories, as appropriate, to the head or
# tail of the path, to PATH and MANPATH or athena_path and athena_manpath.
#

  if ( $?add_bin || $?add_man ) then
     switch ($?add_env$?add_front)

       case 00:
         if ( $?add_bin && "$PATH" !~ *"$add_bin"* ) then
           if ($?add_verbose) echo $add_bin added to end of \$PATH
           set add_path = ${add_path}:$add_bin
         endif

         if ( $?add_man && "$MANPATH" !~ *"$add_man"* ) then
           if ($?add_verbose) echo $add_man added to end of \$MANPATH
           setenv MANPATH ${MANPATH}:$add_man
         endif
         breaksw

       case 01:
         if ( $?add_bin && "$PATH" !~ *"$add_bin"* ) then
           if ($?add_verbose) echo $add_bin added to front of \$PATH
           set add_path = ${add_bin}:$add_path
         endif

         if ( $?add_man && "$MANPATH" !~ *"$add_man"* ) then
           if ($?add_verbose) echo $add_man added to front of \$MANPATH
           setenv MANPATH ${add_man}:$MANPATH
         endif
         breaksw

       case 10:
         if ( $?add_bin && "$athena_path" !~ *"$add_bin"* ) then
           if ($?add_verbose) echo $add_bin added to end of \$athena_path
           set athena_path = ($athena_path $add_bin)
         endif

         if ( $?add_man && "$athena_manpath" !~ *"$add_man"* ) then
           if ($?add_verbose) echo $add_man added to end of \$athena_manpath
           set athena_manpath = ${athena_manpath}:$add_man
         endif
         breaksw

       case 11:
         if ( $?add_bin && "$athena_path" !~ *"$add_bin"* ) then
           if ($?add_verbose) echo $add_bin added to front of \$athena_path
           set athena_path = ($add_bin $athena_path)
         endif

         if ( $?add_man && "$athena_manpath" !~ *"$add_man"* ) then
           if ($?add_verbose) echo $add_man added to front of \$athena_manpath
           set athena_manpath = ${add_man}:$athena_manpath
         endif
         breaksw
     endsw
   endif

  if ( ! $?add_bin && $?add_warn ) then
    echo add: warning: $add_i has no binary directory
  endif
end

if ( $?add_path ) then
  if ( $?add_front ) then
    setenv PATH ${add_path}${PATH}
  else
    setenv PATH ${PATH}${add_path}
  endif
endif

finish:

#
# We were never here.
#

foreach add_i ($add_vars)
  unset $add_i
end
