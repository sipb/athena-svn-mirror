#
stty dec 	# sets DEC style control characters
# The PATH variable sets the directories searched when a command is executed
set path=(. $HOME/Bin /usr/athena /usr/new /usr/new/mh /usr/ucb /bin /usr/bin /usr/local /usr/games /usr/hosts)
setenv MAIL /usr/spool/mail/$USER	# mail input box
setenv VISUAL /usr/athena/emacs		# set default editor for various
setenv EDITOR /usr/athena/emacs		# commands that look in the environment
biff n					# don't be assaulted by incoming mail
setenv MORE -cs				# default MORE program behaviour
set ignoreeof				# ^D will not log you out
# the following obscurity gets terminal characteristics into the environment
set noglob; \
	eval `tset -s -Q -m 'switch>1200:vt100' -m 'switch<=1200:?vt100'`
msgs -q					# show system messages.
umask 66				# per privacy committee.
