
#
# Copyright (c) 1985,1987,1991,1992 Adobe Systems Incorporated. All rights
# reserved. 
# GOVERNMENT END USERS: See notice of rights in Notice file in release
# directory. 
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/mkprinter.sh,v 1.1.1.1 1996-10-07 20:25:22 ghudson Exp $
#
# usage: mkprinter PRINTER TTY COMM

# mkprinter takes PRINTER TTY COMM
# 	where PRINTER is the name by which you want a printer, 
#	TTY is the actual device (/dev/TTY) it is hooked up to, and 
#	COMM is the desired communications program for this printer
#	(one of pscomm, qmscomm, lpcomm, fpcomm, or ascomm)

if test $# != 3 ; then
	echo usage: mkprinter PRINTER TTY COMM
	exit 1
fi

PRINTER=$1
TTY=$2
COMM=$3
export PRINTER TTY COMM

if  [ -r ./config ]
then
    . ./config
else
    echo "config file missing!"
    exit 1
fi	

if [ -r ./printer ] 
then
    . ./printer
else
    echo "printer file missing!"
    exit 1
fi

if [ $COMM = "pscomm" ] 
then
    if [ $OPSYS = "SYSV" ]
    then
	echo "Here is the /etc/inittab line for $TTY"
        echo "You may need to edit it or use the ttymgmt menu of sysadm"
        fgrep "${TTY}" /etc/inittab
    else
        echo "Here is the line from /etc/ttys for $TTY:"
        echo "You may need to edit it and do a 'kill -HUP 1' to disable login/getty"
        fgrep "${TTY}" /etc/ttys
    fi
fi

# set up the device itself
rm -f /dev/${PRINTER}
# some of the new communications modules don't need a device, so
# null is specified.  But we don't want it to actually be /dev/null,
# because that would serialize multiple printers using it, so we create an
# empty file.
if [ ${TTY} = "null" ]
then
    echo >/dev/${PRINTER}
else
    ln /dev/${TTY} /dev/${PRINTER}
fi

if [ $OPSYS = "SYSV" ]
then
    if [ $COMM = "pscomm" ]
    then
        # find out what stty string to use
        CODE=`(stty cs8 9600 cread -clocal -ignbrk brkint -parmrk \
	       inpck -istrip -inlcr -igncr -icrnl -iuclc ixon -ixany ixoff \
	       -opost -isig -icanon -xcase \
               -echo -echoe -echok -echonl min \^a time \^d 
	        stty -g ) </dev/${PRINTER}`
        export CODE
    fi

    # create a transcript directory log files
    PDIR=${SPOOLDIR}/transcript
    export PDIR
    if test ! -d $PDIR ; then
	    mkdir $PDIR
    fi
    rm -rf ${PDIR}/${PRINTER}-log*

    cp /dev/null ${PDIR}/${PRINTER}-log
    cp /dev/null ${PDIR}/${PRINTER}.opt
    echo "PSCOMM=\$PSLIBDIR/$COMM" >>${PDIR}/${PRINTER}.opt
    echo "export PSCOMM" >>${PDIR}/${PRINTER}.opt
    chown $SPOOLUSER $PDIR $PDIR/$PRINTER-log ${PDIR}/${PRINTER}.opt
    chgrp $SPOOLGROUP $PDIR $PDIR/$PRINTER-log ${PDIR}/${PRINTER}.opt
    chmod 775 $PDIR
    chmod 664 $PDIR/$PRINTER-log ${PDIR}/${PRINTER}.opt



    # create the interface spec for this printer
    sed	-e s,XCODEX,${CODE},g \
        -e s,XSPOOLDIRX,${SPOOLDIR},g \
	-e s,XPSCOMMX,${COMM},g \
	    $BUILDDIR/lib/psinterface >psinterface

    if [ $SOLARIS = "FALSE" ]
    then
      /usr/lib/lpshut
    fi
    /usr/lib/lpadmin -x$PRINTER
    if [ $SOLARIS = "TRUE" ]
    then
         /usr/lib/lpadmin -p$PRINTER -cPostScript -Iany -h -ipsinterface \
	                  -v${PDIR}/${PRINTER}-log
    else
         /usr/lib/lpadmin -p$PRINTER -cPostScript -h -ipsinterface \
	                  -v${PDIR}/${PRINTER}-log
    fi
    /usr/lib/lpsched
    /usr/lib/accept $PRINTER PostScript
    enable $PRINTER

    # report what we have done
    echo "Here are the goods on ${PRINTER}:"
    ls -lF /dev/${TTY} /dev/${PRINTER}
    ls -ldF ${PDIR}
    ls -lF ${PDIR}/${PRINTER}-log ${PDIR}/${PRINTER}.opt
    lpstat -t
else
    # create a spooling directory and log and accounting files
    rm -rf ${LOGDIR}/${PRINTER}-log* ${ACCTDIR}/${PRINTER}.acct*
    rm -rf ${SPOOLDIR}/${PRINTER}
    mkdir ${SPOOLDIR}/${PRINTER}
    if test ! -d ${LOGDIR} ; then
	    mkdir ${LOGDIR}
    fi
    if test ! -d ${ACCTDIR} ; then
	    mkdir ${ACCTDIR}
    fi	
    cp /dev/null $LOGDIR/${PRINTER}-log
    cp /dev/null $ACCTDIR/${PRINTER}.acct
    chown $SPOOLUSER $SPOOLDIR/$PRINTER $LOGDIR/$PRINTER-log \
	    $ACCTDIR/$PRINTER.acct
    chgrp $SPOOLGROUP $SPOOLDIR/$PRINTER $LOGDIR/$PRINTER-log \
	    $ACCTDIR/$PRINTER.acct
    chmod 775 ${SPOOLDIR}/${PRINTER}
    chmod 664 $LOGDIR/$PRINTER-log $ACCTDIR/$PRINTER.acct

    cp /dev/null ${SPOOLDIR}/${PRINTER}/.options
    echo "PSCOMM=\$PSLIBDIR/$COMM" >>${SPOOLDIR}/${PRINTER}/.options
    echo "export PSCOMM" >>${SPOOLDIR}/${PRINTER}/.options
    chown $SPOOLUSER ${SPOOLDIR}/${PRINTER}/.options
    chgrp $SPOOLGROUP ${SPOOLDIR}/${PRINTER}/.options
    chmod 775 ${SPOOLDIR}/${PRINTER}/.options

    # report what we have done
    echo "Here are the goods on ${PRINTER}:"
    ls -lgF /dev/${TTY} /dev/${PRINTER}
    ls -lgdF ${SPOOLDIR}/${PRINTER}
    ls -lgF ${LOGDIR}/${PRINTER}-log
    ls -lgF ${ACCTDIR}/${PRINTER}.acct
    echo " "

    # create a printcap entry for it
    sed	-e s,PSLIBDIR,${PSLIBDIR},g \
	    -e s,PRINTER,${PRINTER},g \
	    -e s,SPOOLDIR,${SPOOLDIR},g \
	    -e s,LOGDIR,${LOGDIR},g \
	    -e s,ACCTDIR,${ACCTDIR},g \
	    $SRCDIR/etc/printcap.proto >printcap.new

    if fgrep -s /dev/${PRINTER} /etc/printcap ; then
	    echo "There seems to be an existing printcap entry for $PRINTER."
    fi
    echo "Examine printcap.new and edit /etc/printcap to include it."
fi
