# $Id: root.cshrc,v 1.3 1998-04-30 20:04:44 ghudson Exp $

# For interactive shells, set the prompt to show the host name.
if ( (! $?ENVONLY) && $?prompt ) then
	set prompt="`uname -n`# "
endif

set path=(/srvd/patch /usr/athena/bin /etc/athena /bin/athena /usr/sbin \
	  /usr/bsd /sbin /usr/bin /bin /etc /usr/athena/etc /usr/etc \
	  /usr/bin/X11)

set add_flags="-a -h -n"
alias add 'eval "`/bin/athena/attach -Padd $add_flags \!:*`"'

# source user's .cshrc if WHO variable is set
if ($?WHO) then
	if ( -r ~$WHO/.cshrc ) then
		source ~$WHO/.cshrc
	endif
endif
umask 022
