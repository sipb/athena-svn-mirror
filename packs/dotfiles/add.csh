#!/bin/csh -f
#
# add <addargs> <attachargs> <lockername> <lockername> ...
#
#	-v	verbose
#	-f	add lockers to the front of the path
#	-p	print path environment filtered
#	-c	clean add - don't add empty paths; return error
#	-a	pass further options to attach (passes 1,2?)
#	-e	perform operations for the .environment file (changing
#		  $athena_path, $athena_manpath instead of $PATH,
#		  etc.)
#
# fix bugs section of attach manpage

# alias add 'set add_opts = (\!:*); source /mit/cfields/add'

# MANPATH search too

while ( $#add_opts > 0 )
  set arg = $add_opts[1]

  switch ($arg)

    case -v:
      set add_verbose
      breaksw

    case -f:
      set add_front
      breaksw

    case -c:
      set add_clean
      breaksw

    case -e:
      set add_env
      breaksw

    case -a:
      shift add_opts

    default:
      if ( $#add_opts ) then
        set add_attach = "$add_opts"
        set add_opts=
      endif
  endsw
  shift add_opts

end

unset add_opts
