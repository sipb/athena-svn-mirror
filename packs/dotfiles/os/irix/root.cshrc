# $Id: root.cshrc,v 1.3 1999-04-09 19:14:13 ghudson Exp $

# For interactive shells, set the prompt to show the host name.
if ( (! $?ENVONLY) && $?prompt ) then
	set prompt="`uname -n`# "
	set nostat = (/afs/)
endif

set path=(/srvd/patch /usr/athena/bin /etc/athena /bin/athena /usr/sbin \
	  /usr/bsd /sbin /usr/bin /bin /etc /usr/athena/etc /usr/etc \
	  /usr/bin/X11)
setenv MANPATH /usr/athena/man:/usr/freeware/catman:/usr/share/catman:/usr/share/man:/usr/catman:/usr/man

# Default man pager is ul -b | more -s -f.  Remove the ul -b part
# to work around a bug where pager input is read in line mode.
setenv MANPAGER "more -s -f"

set add_flags="-a -h -n"
alias add 'eval `/bin/athena/attach -Padd $add_flags \!:*`'

# source user's .cshrc if WHO variable is set
if ($?WHO) then
	if ( -r ~$WHO/.cshrc ) then
		source ~$WHO/.cshrc
	endif
endif
umask 022
