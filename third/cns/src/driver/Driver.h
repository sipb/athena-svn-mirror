#define	dNeedLock		0x4000
#define dStatEnable		0x0800
#define dCtlEnable		0x0400

/* 
 	Driver error codes:
	Stolen from KrbDriver.h, KClient driver interface.
*/

enum {
	cKrbCorruptedFile = -1024,	/* couldn't find a needed resource */
	cKrbNoKillIO,				/* can't killIO because all calls sync */
	cKrbBadSelector,			/* csCode passed doesn't select a recognized function */
	cKrbCantClose,				/* we must always remain open */
	cKrbMapDoesntExist,			/* tried to access a map that doesn't exist (index too large,
									or criteria doesn't match anything) */
	cKrbSessDoesntExist,		/* tried to access a session that doesn't exist */
	cKrbCredsDontExist,			/* tried to access credentials that don't exist */
	cKrbTCPunavailable,			/* couldn't open MacTCP driver */
	cKrbUserCancelled,			/* user cancelled a log in operation */
	cKrbConfigurationErr,		/* Kerberos Preference file is not configured properly */
	cKrbServerRejected,			/* A server rejected our ticket */
	cKrbServerImposter,			/* Server appears to be a phoney */
	cKrbServerRespIncomplete,	/* Server response is not complete */
	cKrbNotLoggedIn,			/* Returned by cKrbGetUserName if user is not logged in */
	cKrbOldDriver,				/* old version of the driver */
	
	cKrbKerberosErrBlock = -20000	/* start of block of 256 kerberos errors */
	};
	
