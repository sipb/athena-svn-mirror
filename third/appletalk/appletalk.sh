#!/bin/sh

case "$CPUTYPE" in
	SPARC/Classic)
		;;
	SPARC/*)
		echo "Adding AppleTalk devices... \c"

		if /os/usr/sbin/add_drv -b "$ROOT/" -m '* 0666 root sys' \
			atalk; then
			echo "done."
		else
			echo "failed to add AppleTalk devices."
			exit 1
		fi
		;;
	*)
		;;
esac
