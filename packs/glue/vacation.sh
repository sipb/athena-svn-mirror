#!/bin/sh
# $Id: vacation.sh,v 1.3 2004-03-16 04:16:57 rbasch Exp $

echo 1>&2 "For most Athena users, incoming mail goes to the MIT mail hubs and"
echo 1>&2 "from there onto a post-office server machine.  Your home directory"
echo 1>&2 "is not consulted until you incorporate the mail.  The system"
echo 1>&2 "'vacation' program assumes more traditional Unix mail routing"
echo 1>&2 "controllable by .forward files, and thus will not work."
echo 1>&2 
case "$DISPLAY" in
  "")
    echo 1>&2 "To configure MIT's central auto-responder service via the web,"
    echo 1>&2 "please visit http://web.mit.edu/mail/autoresponder/"
    exit 0
    ;;
esac
echo 1>&2 "You may configure MIT's central auto-responder service via the web"
echo 1>&2 "browser that will appear momentarily."

exec htmlview -c http://web.mit.edu/mail/autoresponder/
