#
# aliases
#
alias	cd	'set old=$cwd; chdir \!*'
alias	back	'set back=$old; set old=$cwd; cd $back; unset back; dirs'
alias	j	jobs -l
#
# set variables
#
setenv TZ US/Eastern
set history=40
set cdpath=(/)
set path=(/usr/athena/bin /etc/athena /usr/sbin /sbin /bin/athena /usr/bin /usr/ccs/bin /usr/ucb /usr/athena/etc /usr/openwin/bin /etc)
if ($?prompt) then
	set prompt="`uname -n`# "
endif
#
# source user's .cshrc if WHO variable is set
#
if ($?WHO) then
	if ( -r ~$WHO/.cshrc ) then
		source ~$WHO/.cshrc
	endif
endif
umask 2
