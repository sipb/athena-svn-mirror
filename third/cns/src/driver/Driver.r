#include "Types.r"
#include "MPWTypes.r"
type 'DRVR' as 'DRVW';
#define DriverID 1

resource 'DRVR' (DriverID, ".kerberos", SYSHEAP, LOCKED) {
	needLock,			/* lock drvr in memory	*/
	dontNeedTime,		/* for periodic action	*/
	dontNeedGoodbye,	/* call before heap reinit*/
	statusEnable,		/* responds to status	*/
	ctlEnable,			/* responds to control	*/
	writeEnable,		/* responds to write	*/
	readEnable,			/* responds to read 	*/
	0,					/* driver delay (ticks) */
	everyEvent,			/* desk acc event mask	*/
	0,					/* driver menu ID		*/
	".kerberos",				/* driver name */
	$$resource("DriverShell Driver.DRVW",'DRVW',0) 
};