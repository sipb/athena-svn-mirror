# Prototype file for setting up course-specific environment

echo "Attaching $SUBJECT ... "
setup_dir=`/bin/attach -p $SUBJECT`

if [ $? == 0 ]; then

	# Record information to remove setup later
	setup_filsys=$SUBJECT

	# Set prompt to reflect changed environment
	PS1="${SUBJECT}\$ "

	if [ -r $setup_dir/.attachrc.bash ]; then
		unset SUBJECT	# to prevent infinite loops
		echo "Running commands in $setup_dir/.attachrc.bash ... "
		source $setup_dir/.attachrc.bash
        else
	    if [ -r $setup_dir/.attachrc ]; then
		echo "This locker is not yet configured for the 'setup' command."
		echo "Contact the locker maintainer for more information."
	    fi
	    # Do minimal environment setup
	    add $setup_filsys

	fi
else
	echo ""
        echo "The $SUBJECT filesystem could not be attached."
	filsys=(`hesinfo $SUBJECT filsys`)
	if [ ${#filsys} -lt 3 ]; then
		echo "Check your spelling of the filesystem name."
	else
		echo -n "The ${filsys[0]} fileserver"
		if [ ${filsys[0]} != "AFS" ]; then 
		    echo -n " named ${filsys[2]}"
		fi
		echo " may be down."
	fi
	if [ -n "$XSESSION" ]; then
	    echo "Type exit to get rid of this window."
	else
	    kill -HUP $$	# cause shell to exit
	fi
fi
unset SUBJECT	# to prevent infinite loops
