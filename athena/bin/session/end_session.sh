#!/bin/sh
#
#  end_session - Sends hangup signal to all session_gate processes
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/session/end_session.sh,v $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/session/end_session.sh,v 1.3 1993-03-12 19:07:49 probe Exp $
#	$Author: probe $
#

MAX_PASSES=10
pass_num=0
DELAY_TIME=6

trap "exit 6" 2 3

cd /

#  Process the argument list.

force=0
while [ $# != 0 ]; do
	case $1 in
	"-f" | "-force" )
		force=1
		;;
	* )
		echo "usage: $0 [-force]"
		exit 1
		;;
	esac
	shift
done

#  Name of file containing process ID of the session_gate process.

## This is not needed, thanks to the T-shell (/bin/athena/tcsh) $uid variable.
#awkcmd='BEGIN {found=0;} ($1=="'$USER'" && found==0) {print $3; found=1;}'
#uid=`awk -F: "$awkcmd" < /etc/passwd`
#if [ x"$uid" = x"" ]; then
#	fmt << EOF
#************************************************************
#end_session:
#Cannot find you in this machine's password file; therefore
#your session_gate process id file cannot be found.  I suggest
#you reboot your workstation to fix this problem.
#************************************************************
#EOF
#	exit 5
#fi

uid=`/bin/athena/tcsh -fc 'echo $uid'`
pid_file=/tmp/session_gate_pid.$uid

if [ $force = 1 ]; then

    # If using standard startup, permit time for session_gate to start up.

    sleep 2

    #  Kill session_gate processes by brute force.

    pids="`/bin/ps uxc | /bin/awk '($10 == "session_gate") { print $2 }'`"

    if [ x"$pids" = x"" ]; then
	echo "************************************************************"
	echo "No session_gate processes are running -- you are probably"
	echo "using a customized session file.  End your session by"
	echo "terminating the last process you started in your session"
	echo "file."
	echo "************************************************************"
	exit 2
    else
	/bin/kill -HUP $pids
    fi

else

    #  Check for readability of the file containing the process ID of the
    #  session_gate process.

    # First, loop a while, until we can read the file, or until we time out.
    while [ ! -r $pid_file ] && [ `expr $pass_num '<' $MAX_PASSES` = 1 ]; do
	sleep $DELAY_TIME
	pass_num=`expr 1 + $pass_num`
    done
    sleep 2

    if [ ! -r $pid_file ]; then
	echo "************************************************************"
	echo "                Your session is still running."
	echo ""
	echo "end_session failed because:"
	echo "  The file $pid_file doesn't exist or is"
	echo "  not readable."
	echo ""
	echo "If you are running a .xsession file other than the system"
	echo "default, and that file does not invoke the program"
	echo "'session_gate', then end_session will not work.  You should"
	echo "end your session by terminating the last process you started"
	echo "in your .xsession file."
	echo ""
	echo "If you did run session_gate, then the file was somehow"
	echo "deleted.  Try typing 'end_session -force' (this time only)"
	echo "to end your session." 
	echo "************************************************************"
	exit 3
    fi

    #  Read the process ID and attempt to kill the process.

    pids="`/bin/cat $pid_file`"
    problem=0
    if [ x"$pids" = x"" ]; then
	problem=1
    fi
    for pid in $pids ; do
	case $pid in
	-* | 0 | 1 )
	    problem=1
	    ;;
	* )
	    /bin/kill -HUP $pid > /dev/null 2>&1
	    status=$?
	    case $status in
		0 | 1 )
		    # process killed or not found
		    ;;
		* )
		    problem=1
		    ;;
	    esac
	    ;;
	esac
    done

    if [ $problem = 1 ]; then
	echo "************************************************************"
	echo "end_session failed because:"
	echo "  The file $pid_file may have been modified."
	echo ""
	echo "Try typing 'end_session -force' (this time only) to end your"
	echo "session."
	echo "************************************************************"
	exit 4
    fi

fi
exit 0
