#!/bin/sh
# $Id: sendbug.sh,v 1.3 1989-12-04 15:22:54 raeburn Exp $
# make sure stuff this script needs is up front
eval `/etc/athena/sh_commands_path`
PATH=$PATH:/bin/athena
bugs_address=bugs@Athena.MIT.EDU
sendmail="/usr/lib/sendmail -t -oi"
report_file=/tmp/bug$$.text
version_file=/etc/version
if [ ! -r $version_file ]; then
	version="unknown version (no $version_file found)"
else
	awk_cmd='\
		{if ($5 == "Update") update++; \
		else if ($5 == version) { same++; update=0; } \
		else { version=$5; update=0; same=0; } } \
	END { printf "%s", version; \
		close=0; \
		if (update) { printf " (plus partial update"; close=1; }\
		else if (same) { \
			if (close) printf "; "; \
			else printf " ("; \
			printf "%d update(s) to same version", same; \
			close=1; } \
		if (close) printf ")"; \
	}'
	version=`awk "$awk_cmd" < $version_file`
fi
short_version=`expr "$version (" : '\([^(]*[^( ]\) *(.*'`
machtype=`machtype`
cpu=`machtype -c`
hostname=`hostname`
dpy=`machtype -d`
/usr/ucb/fmt << EOF
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
	[Please fill this in.]

What's wrong:
	[Please fill this in.]

What should have happened:
	[Please fill this in.]

Please describe any relevant documentation references:
	[Please fill this in.]
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
if [ "x$EDITOR" = "x" ]; then
	EDITOR=/usr/athena/emacs ; export EDITOR
fi

$EDITOR $report_file
exec whatnow $report_file
