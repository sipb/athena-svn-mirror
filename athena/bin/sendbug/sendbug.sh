#!/bin/sh
# $Id: sendbug.sh,v 1.17 1997-04-30 08:32:56 ghudson Exp $

# save PATH so we can restore it for user's $EDITOR later
saved_path="$PATH"

# make sure stuff this script needs is up front
PATH=/srvd/patch:/usr/athena/bin:/bin/athena:/usr/bin/X11:/usr/ucb:/bin:/usr/bin:/usr/bsd:/usr/sbin
export PATH

bugs_address=bugs@MIT.EDU
sendmail="/usr/lib/sendmail -t -oi"
report_file=/tmp/bug$$.text
version_file=/etc/athena/version
if [ ! -r "$version_file" ]; then
	version="unknown version (no $version_file found)"
else
	version=`awk '
		/^Athena Workstation \([^\) ]*\) Version Update/ {
			update = 1;
		}
		/^Athena Workstation \([^\) ]*\) Version [0-9]/ {
			update = 0;
			mkserv_update = 0;
			mkserv = 0;
			if ($5 == version) {
				same++;
			} else {
				version = $5;
				same = 0;
			}
		}
		/^Athena Server \([^\) ]*\) Version Update/ {
			mkserv_update = 1;
		}
		/^Athena Server \([^\) ]*\) Version [0-9]/ {
			mkserv_update = 0;
			mkserv = 1;
		}
		END {
			printf "%s", version;
			if (same)
				printf " (%d update(s) to same version)", same;
			if (mkserv)
				printf " (with mkserv)";
			if (mkserv_update)
				printf " (plus partial mkserv)";
			if (update)
				printf " (plus partial update)";
		}' "$version_file"`
fi
short_version=`expr "$version (" : '\([^(]*[^( ]\) *(.*'`
machtype=`machtype`
cpu=`machtype -c`
hostname=`hostname`
dpy=`machtype -d`
fmt << EOF
Please enter the name of the program or locker with which you are
having problems. You may first want to check with the consultants to
see if there is a known workaround to this problem; hit ctrl-c now and
type 'olc' at your athena% prompt to enter a question.
EOF
echo -n ' --> '
read subject
cat > $report_file << EOF
To: $bugs_address
Subject: $machtype $short_version: $subject
-------
System name:		$hostname
Type and version:	$cpu $version
Display type:		$dpy

What were you trying to do?
	[Please replace this line with your information.]

What's wrong:
	[Please replace this line with your information.]

What should have happened:
	[Please replace this line with your information.]

Please describe any relevant documentation references:
	[Please replace this line with your information.]
EOF

fmt << EOF

Please fill in the specified fields of the bug report form, which will
be displayed momentarily.
Remember to save the file before exiting the editor.
EOF

if [ -r "${MH-$HOME/.mh_profile}" ]; then
	PATH="$saved_path" /usr/athena/bin/comp -form "$report_file"
	rm "$report_file"
	exit 0
fi
# not using MH; run the editor, and send, ourselves.
MH=/dev/null; export MH
if [ "${EDITOR}" = "" ]; then
	EDITOR=/usr/athena/bin/emacs ; export EDITOR
fi

PATH="$saved_path" exec whatnow -editor "$EDITOR" "$report_file"
