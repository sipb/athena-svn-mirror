add_flags="-a -h -n"
add () { eval "$( /bin/athena/attach -Padd -b $add_flags "$@" )" ; }
PS1='\H# '
