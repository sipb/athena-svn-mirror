alias mail Mail		# Both mail and Mail are obsolete, but Mail is better
set history=40 savehist=50	# number of history events saved
set cdpath=(~)		# change directory path
set mail=(120 /usr/spool/mail/$USER)	# set mail parameters
# conveniant aliases for common commands
# the cd and back commands are defined so that cd saves the last directory
# you were in before the cd; then back can return you to the previous directory
alias	cd	'set old=$cwd; chdir \!*'
alias	back	'set back=$old; set old=$cwd; cd $back; unset back; dirs'
alias	term	'setenv TERM `tset -Q - \!*`'
if ($?prompt) then	# set prompt to a value depending on host name
	set prompt="`hostname | sed s/mit-//`% "
endif

set msg = "Sourcing .cshrc and .login..."
