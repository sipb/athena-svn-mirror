#!/bin/csh -f
#
# add <addargs> <attachargs> <lockername> <lockername> ...
#
#	-v	verbose
#	-f	add lockers to the front of the path
#	-p	print path environment filtered
#	-w	give warning for adds with no bindirs
#	-e	perform operations for the .environment file (changing
#		  $athena_path, $athena_manpath instead of $PATH,
#		  etc.)
#	-a	pass further options to attach (passes 1,2?)
#
# fix bugs section of attach manpage

# alias add 'set add_opts = (\!:*); source /afs/dev/user/cfields/misc/add'

# MANPATH search too

set add_vars=(add_vars add_usage add_verbose add_front add_warn add_env \
              add_opts add_attach add_dirs add_bin add_item add_i)

set add_usage = "Usage: add [-v] [-f] [-p] [-w] [-e] [-a attachflags] [lockername] ..."

while ( $#add_opts > 0 )
  set arg = $add_opts[1]

  switch ($arg)

    case -v:
      set add_verbose
      breaksw

    case -f:
      set add_front
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

if ( $?add_verbose ) attach -n -h $add_attach

set add_dirs = `attach -p $add_attach`

set ATHENA_SYS = `fs sysname | awk -F\' '{ print $2 }'`

set add_bin = arch/$ATHENA_SYS/bin

foreach add_i ($add_dirs)
  unset add_item
  if ( -d $add_i/$add_bin ) then
    set add_item = $add_i/$add_bin
  else
    if ( -d $add_i/$bindir ) then
      set add_item = $bindir
    endif
  endif

  if ( $?add_item ) then

    echo foo

    if ( ! $?add_env && ! $?add_front ) then
      if ( "$PATH" !~ *"$add_item"* ) then
        if ($?add_verbose) echo $add_item added to end of \$PATH
        setenv PATH ${PATH}:$add_item
      endif
    endif

    echo bar

    if ( ! $?add_env && $?add_front ) then
         if ( "$PATH" !~ *"$add_item"* ) then
           if ($?add_verbose) echo $add_item added to front of \$PATH
           setenv PATH $add_item:$PATH
         endif
    endif

    if ( $?add_env && ! $?add_front ) then
         if ( "$athena_path" !~ *"$add_item"* ) then
           if ($?add_verbose) echo $add_item added to end of \$athena_path
           set athena_path ($athena_path $add_item)
         endif
    endif

    if ( $?add_env && $?add_front) then
         if ( "$athena_path" !~ *"$add_item"* ) then
           if ($?add_verbose) echo $add_item added to front of \$athena_path
           set athena_path ($add_item $athena_path)
         endif
    endif
  else
    if ( $?add_warn ) then
      echo add: warning: $add_i has no binaries
    endif
  endif
end

finish:

foreach add_i ($add_vars)
  unset $add_i
end
