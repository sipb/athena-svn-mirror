
# Copyright (c) 1985,1987,1991 Adobe Systems Incorporated. All rights reserved.
# GOVERNMENT END USERS: See notice of rights in Notice file in release directory.
# PostScript is a registered trademark of Adobe Systems Incorporated
# TranScript is a registered trademark of Adobe Systems Incorporated
# RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/rmprinter.sh,v 1.1.1.1 1996-10-07 20:25:23 ghudson Exp $
#
# usage: rmprinter PRINTER

if [ -r ./config ] 
then
    . ./config
else
    echo "config file missing"
    exit 1
fi

if [ -r ./printer ]
then 
    . ./printer
else
    echo "printer file missing"
    exit 1
fi

if [ $OPSYS = "BSD" ]
then
    PATH=${PATH}:/etc:/usr/etc
    export PATH
fi

PRINTER=$1
export PRINTER

# shut down the printer
if [ $OPSYS = "BSD" ]
then
    lpc stop $PRINTER
    lpc disable $PRINTER
    lpc abort $PRINTER
    lprm -P$PRINTER -
else
    /usr/lib/reject -r"printer being removed" $PRINTER
    disable -c -r"printer begin removed" $PRINTER
    cancel $PRINTER
    if [ $SOLARIS = "FALSE" ]
    then
      /usr/lib/lpshut
    fi
    /usr/lib/lpadmin -x$PRINTER
    /usr/lib/lpsched
fi

# remove the device
rm -f /dev/${PRINTER}

# remove spool, log, options, accounting files
if [ $OPSYS = "BSD" ]
then
    rm -rf ${LOGDIR}/${PRINTER}-log* ${ACCTDIR}/${PRINTER}.acct*
    rm -rf ${SPOOLDIR}/${PRINTER}
else
    PDIR=${SPOOLDIR}/transcript
    export PDIR
    rm -rf ${PDIR}/${PRINTER}-log* ${PDIR}/${PRINTER}.opt
fi

# finish up
if [ $OPSYS = "BSD" ]
then
    echo "remove the entry for $PRINTER from /etc/printcap"
else
    echo "$PRINTER removed - lp status is now:"
    lpstat -t
fi

exit 0
