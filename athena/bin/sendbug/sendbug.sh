#!/bin/sh
# $Id: sendbug.sh,v 1.9 1991-08-10 09:05:48 epeisach Exp $
# make sure stuff this script needs is up front
PATH=/srvd/patch:/usr/athena/bin:/bin/athena:/usr/bin/X11:/usr/ucb:/bin:/usr/bin
bugs_address=bugs@Athena.MIT.EDU
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
Please enter the subject for this bug report.  (Generally, this means the
name of the program or locker with which you are having problems.)
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

if [ -r $HOME/.mh_profile ]; then
	comp -form $report_file
	rm $report_file
	exit 0
fi
# not using MH; run the editor, and send, ourselves.
if [ "${EDITOR}" = "" ]; then
	EDITOR=emacs ; export EDITOR
fi

$EDITOR $report_file
exec whatnow $report_file
