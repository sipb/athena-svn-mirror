# Prototype file sourced by "remove" alias

if [ -n "$setup_dir" -a -n "$setup_filsys" ]; then
	if [ -r $setup_dir/.detachrc.bash ]; then
	    source $setup_dir/.detachrc.bash
	fi
	cd /			# get out of locker
	/bin/detach $setup_filsys
	kill -HUP $$ 		# cause shell to exit
fi

echo "Remove only works from the same shell in which you typed setup."
