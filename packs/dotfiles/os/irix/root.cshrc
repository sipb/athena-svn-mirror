# $Id: root.cshrc,v 1.7 2001-07-09 13:04:49 ghudson Exp $

set path=(/srvd/patch /usr/athena/bin /usr/athena/etc /etc/athena /bin/athena \
	  /usr/sbin /usr/bsd /sbin /usr/bin /bin /etc /usr/etc /usr/bin/X11)
setenv MANPATH /usr/athena/man:/usr/freeware/catman:/usr/share/catman:/usr/share/man:/usr/catman:/usr/man
if ( (! $?ENVONLY) && $?prompt ) then
	set prompt="`uname -n`# "
	set nostat = (/afs/)
	set autolist
endif

set add_flags="-a -h -n"
alias add 'eval `/bin/athena/attach -Padd $add_flags \!:*`'

# source user's .cshrc if WHO variable is set
if ($?WHO) then
	if ( -r ~$WHO/.cshrc ) then
		source ~$WHO/.cshrc
	endif
endif
umask 022
