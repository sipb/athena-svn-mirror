#!/bin/sh
# $Id: vacation.sh,v 1.1 1998-12-25 19:17:29 ghudson Exp $

echo 1>&2 "For most Athena users, incoming mail goes to the MIT mail hubs and"
echo 1>&2 "from there onto a post-office server machine.  Your home directory"
echo 1>&2 "is not consulted until you incorporate the mail.  The system"
echo 1>&2 "'vacation' program assumes more traditional Unix mail routing"
echo 1>&2 "controllable by .forward files, and thus will not work.  MIT's"
echo 1>&2 "central mail system does not support vacation functionality at"
echo 1>&2 "this time."
echo 1>&2 ""
echo 1>&2 "If you want to run the system vacation program anyway, you can"
echo 1>&2 "run it as 'vacation.real'."
