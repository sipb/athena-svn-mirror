#!/bin/sh
# $Id: sendbug.sh,v 1.16 1996-08-19 21:14:55 ghudson Exp $

# save PATH so we can restore it for user's $EDITOR later
saved_path="$PATH"

# make sure stuff this script needs is up front
PATH=/srvd/patch:/usr/athena/bin:/bin/athena:/usr/bin/X11:/usr/ucb:/bin:/usr/bin:/usr/bsd:/usr/sbin
export PATH

bugs_address=bugs@MIT.EDU
sendmail="/usr/lib/sendmail -t -oi"
report_file=/tmp/bug$$.text
if [ ! -r /etc/athena/version -a -r /etc/version ]; then
      version_file=/etc/version
else
      version_file=/etc/athena/version
fi
if [ ! -r $version_file ]; then
	version="unknown version (no $version_file found)"
else
	awk_cmd='\
		{if ($5 == "Update") update++; \
		else if ($5 == version) { same++; update=0; } \
		else { version=$5; update=0; same=0; } } ; \
	END { printf "%s", version; \
		close1=0; \
		if (update) { printf " (plus partial update"; close1=1; }\
		else if (same) { \
			if (close1) printf "; "; \
			else printf " ("; \
			printf "%d update(s) to same version", same; \
			close1=1; } \
		if (close1) printf ")"; \
	}'
	version=`awk "$awk_cmd" < $version_file`
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
