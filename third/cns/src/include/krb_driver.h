/*
 * KrbDriver.h -- Application interface for Kerberos Macintosh Client
 *                (A mac system extension)
 * 
 * This interface can be used as is or it can be accessed at a
 * higher level using a library interface (kerberos.h?).
 *
 * Set the compiler def KRB_DEFS if you are also including krb.h
 * in your code (this is only necessary for building the driver itself).
 * 
 * Copyright 1992 by Cornell University
 * FIXME:  What COPYRIGHT terms and conditions from Cornell?
 *
 * Initial coding 1/92 by Peter Bosanko.
 * Merged into multi-platform source tree by Julia Menapace and
 * John Gilmore, Cygnus Support, May 1994.
 */

#ifndef Krb_Driver_h
#define	Krb_Driver_h

/* Driver control block status/control bits */

#define	dNeedLock		0x4000
#define dStatEnable		0x0800
#define dCtlEnable		0x0400


/* csCodes for Control Calls */
enum cKrbcsCodes {
	cKrbKillIO = 1,

	/* Low level routines, here for compatability with Brown Driver */
	cKrbGetLocalRealm,
	cKrbSetLocalRealm,
	
	/* return the name of the Kerberos realm for the host.
	  -> host	Host name
	  -> uRealm	pointer to buffer that will receive realm name
	*/
       	cKrbGetRealm,
       	
	cKrbAddRealmMap,
	cKrbDeleteRealmMap,

	/* yields the Nth mapping of a net or host to a Kerberos realm 
 	  -> itemNumber 	which mapping, traditionally the first
 	  -> host	   		host or net
 	  -> uRealm    		pointer to buffer that will receive realm name
 	*/
	cKrbGetNthRealmMap,

	cKrbGetNthServer,
	cKrbAddServerMap,
	cKrbDeleteServerMap,
	cKrbGetNthServerMap,
	cKrbGetNumSessions,
	cKrbGetNthSession,
	cKrbDeleteSession,
	cKrbGetCredentials,
	cKrbAddCredentials,
	cKrbDeleteCredentials,
	cKrbGetNumCredentials,
	cKrbGetNthCredentials,
			
	/* High Level routines */
	cKrbDeleteAllSessions,
	/* Removes all credentials from storage.  The user will be asked to
	   enter user name and password the next time a ticket is requested */ 

	cKrbGetTicketForService,
	/* Gets a ticket and returns it to application in buf
	  -> service		Formal Kerberos name of service
	  -> buf		Buffer to receive ticket
	  -> checksum		checksum for this service
	 <-> buflen		length of ticket buffer (must be at least
				1258 bytes)
	 <-  sessionKey		for internal use
	 <-  schedule		for internal use */

	cKrbGetAuthForService,
	/* Similiar to cKrbGetTicketForService except it builds a kerberos
	   "SendAuth" style request (with SendAuth and application version
	   numbers preceeding the actual ticket)
	  -> service		Formal Kerberos name of service
	  -> buf		Buffer to receive ticket
	  -> checksum		checksum for this service
	  -> applicationVersion	version number of the application (8 byte
				string)
 	 <-> buflen		length of ticket buffer (must be at least 
				1258 bytes)
	 <-  sessionKey		for internal use
	 <-  schedule		for internal use */

	/* Use the same krbHiParmBlock for the routines below that you
	   used to get the ticket for the service.  That way the session
	   key and schedule will get passed back to the driver.  */
	
	cKrbCheckServiceResponse,
	/* Use the return code from this call to determine if the client
	   is properly authenticated
	  -> buf		points to the begining of the server response
	  -> buflen		length of the server response
	  -> sessionKey		this was returned from cKrbGetTicketForService
	  -> schedule		"	
	  -> checksum		left over from cKrbGetTicketForService call
	  -> lAddr		addresses used for service validation...
	  -> lPort		"
	  -> fAddr		"
	  -> fPort		" */

	cKrbEncrypt,
	/* Encrypt stream, High level version of cKrbMakePrivate
	  -> buf		points to the begining of stream buffer
	  -> buflen		length of the stream buffer
	  -> sessionKey		this was returned from cKrbGetTicketForService
	  -> schedule		"	
	  -> lAddr		server uses addresses to confirm who we are...
	  -> lPort		"
	  -> fAddr		"
	  -> fPort		"
	  -> encryptBuf		output buffer, allow 26 more bytes than
				input data			
	  <- encryptLength	actual length of output data */

	cKrbDecrypt,
	/* Decrypt stream, High level version of cKrbReadPrivate
	  -> buf		points to the begining of stream buffer
	  -> buflen		length of the stream buffer
	  -> sessionKey		this was returned from cKrbGetTicketForService
	  -> schedule		"	
	  -> lAddr		addresses used to confirm source of message...
	  -> lPort		"
	  -> fAddr		"
	  -> fPort		"
	  <- decryptOffset	offset in buf to beginning of application data
	  <- decryptLength	actual length of decrypted data */
		  
	cKrbCacheInitialTicket,
	/* Gets a ticket for the ticket granting service and optionally
   	   another service that you specify.
	   This call always prompts the user for a password.  The
	   ticket(s) are placed in the ticket cache but are not
	   returned.  Use cKrbGetTicketForService to receive the
	   ticket.  NOTE: This call is useful for getting a ticket for
	   the password changing service or any other service that
	   requires that the user be reauthenticated ( that needs an
	   initial ticket ).
	   
	  -> service		Formal Kerberos name of service
		  		( NULL service is OK if you just want a
				ticket granting ticket ) */
								
	cKrbGetUserName,
	/* Get the kerberos name of the user.  If the user is not
	   logged in, returns error cKrbNotLoggedIn.
	   
	  <- user		Name that user typed in login dialog */

	cKrbSetUserName,
	/* Set the kerberos name of the user.  If the user is logged
	in, cKrbSetUserName logged the user out.
	   
	  -> user		Name that will be used as default in
				login dialog */

	cKrbSetPassword,
	/* Sets the password which will be used the next time a
	   password is needed.  This can be used to bypass the login
	   dialog.  NOTE:  Password is cleared from memory after it is
	   used once or whenever a cKrbSetUserName or cKrbDeleteAllSessions
	   call is made.
	  -> user		contains password (of current user) */

	cKrbGetDesPointers,
	/* Returns a block of pointers to DES routines so the routines 
	   can be called directly. */
	      
	/* Various routines added by Cygnus Mac kerberos driver interface */
	
	cKrbKnameParse,
	/* 
	 takes a Kerberos name "fullname" of the form: 
	 username[.instance][@realm] and returns the three components 
	 ("name", "instance", and "realm" in the example above) in the 
	 given arguments. If successful, it returns KSUCCESS.  If there 
	 was an error, KNAME_FMT is returned. For proper operation, this 
	 routine requires that the arguments be initialized, either 
	 to null strings, or to default values of name, instance, 
	 and realm. Low Call.
		<- uName
		<- uInstance
		<- uRealm
		-> fullname
	*/
	
	cKrbGetErrText,
	/* 
	 Given an error number returns the error text associated with 
	 that error number.  Low Call.
		-> admin (the error number)
		<- uName (the error text)
	*/
	
	cKrbGetPwInTkt,
	/* 
	 Takes the name of the server for which the initial ticket is to 
	 be obtained, the name of the principal the ticket is for, the 
	 desired lifetime of the ticket, and the user's password.  It 
	 gets the ticket, decrypts it using the password provided,
 	 and stores it away for future use.  It requires the caller to
 	 supply a non-null password. Low Call. 	
	 	-> uName
		-> uInstance
		-> uRealm
		-> sName
		-> sInstance
		-> admin (ticket lifetime)
		-> fullname (password)
	*/
	
	cKrbGetTfFullname,
	/* 	
	 Given a ticket file name, return the principal's name, instance, 
	 and realm.  Currently there is only one tktfile/cache. Low Call.
		<- uName
		<- uInstance
		-> fullname	(ticket file/cache name)
	*/
	  
	cKrbLAST_ONE};

/* Password changing service */

#define KRB_PASSWORD_SERVICE  "changepw.kerberos"

/* Error codes */
enum cKrbErrs {
	cKrbCorruptedFile = -1024,	/* couldn't find a needed resource */
	cKrbNoKillIO,		/* can't killIO because all calls sync */
	cKrbBadSelector,	/* csCode passed doesn't select a
				   recognized function */
	cKrbCantClose,		/* we must always remain open */
	cKrbMapDoesntExist,	/* tried to access a map that doesn't exist
				   (index too large, or criteria doesn't
				   match anything) */
	cKrbSessDoesntExist,	/* tried to access session that doesn't exist */
	cKrbCredsDontExist,	/* tried to access creds that don't exist */
	cKrbTCPunavailable,	/* couldn't open MacTCP driver */
	cKrbUserCancelled,	/* user cancelled a log in operation */
	cKrbConfigurationErr,	/* Kerberos Preference file is not configured
				   properly */
	cKrbServerRejected,	/* A server rejected our ticket */
	cKrbServerImposter,	/* Server appears to be a phony */
	cKrbServerRespIncomplete,	/* Server response is not complete */
	cKrbNotLoggedIn,	/* Returned by cKrbGetUserName if user is
				   not logged in */
	cKrbOldDriver,		/* old version of the driver */
	
	cKrbKerberosErrBlock = -20000  /* start block of 256 kerberos errors */
};
	
/* Parameter block for high level calls */
struct krbHiParmBlock {
	/* full name -- combined service, instance, realm */
	char 		*service;
	char  		*buf;
	unsigned long  	buflen;
	long 		checksum;
	unsigned long	lAddr;
	unsigned short	lPort;
	unsigned long	fAddr;
	unsigned short	fPort;
	unsigned long	decryptOffset;
	unsigned long	decryptLength;
	char 		*encryptBuf;
	unsigned long	encryptLength;
	char	 	*applicationVersion;	/* Version string, must be
						   8 bytes long! */
	char	 	sessionKey[8];		/* for internal use */
	char		schedule[128];		/* for internal use */
	char 		*user;
};

typedef struct krbHiParmBlock krbHiParmBlock;
typedef krbHiParmBlock *KrbParmPtr;
typedef KrbParmPtr *KrbParmHandle;

/* ********************************************************* */
/* The rest of these defs are for low level calls
/* ********************************************************* */
#ifndef KRB_DEFS
/* First some kerberos defs */

typedef unsigned char des_cblock[8];	/* crypto-block size */

/* Key schedule */
typedef struct des_ks_struct { des_cblock _; } des_key_schedule[16];

#define C_Block des_cblock
#define Key_schedule des_key_schedule

/* The maximum sizes for aname, realm, sname, and instance +1 */
#define 	ANAME_SZ	40
#define		REALM_SZ	40
#define		SNAME_SZ	40
#define		INST_SZ		40

/* Definition of text structure used to pass text around */
#define		MAX_KTXT_LEN	1250

struct ktext {
    int     length;		/* Length of the text */
    unsigned char dat[MAX_KTXT_LEN];	/* The data itself */
    unsigned long mbz;		/* zero to catch runaway strings */
};

typedef struct ktext *KTEXT;
typedef struct ktext KTEXT_ST;

struct credentials {
    char    service[ANAME_SZ];	/* Service name */
    char    instance[INST_SZ];	/* Instance */
    char    realm[REALM_SZ];	/* Auth domain */
    C_Block session;			/* Session key */
    int     lifetime;			/* Lifetime */
    int     kvno;				/* Key version number */
    KTEXT_ST ticket_st;			/* The ticket itself */
    long    issue_date;			/* The issue time */
    char    pname[ANAME_SZ];	/* Principal's name */
    char    pinst[INST_SZ];		/* Principal's instance */
};

typedef struct credentials CREDENTIALS;

/* Structure definition for rd_private_msg and rd_safe_msg */

struct msg_dat {
    unsigned char *app_data;	/* pointer to appl data */
    unsigned long app_length;	/* length of appl data */
    unsigned long hash;		/* hash to lookup replay */
    int     swap;		/* swap bytes? */
    long    time_sec;		/* msg timestamp seconds */
    unsigned char time_5ms;	/* msg timestamp 5ms units */
};

typedef struct msg_dat MSG_DAT;

#endif


/* Parameter block for low level calls */		
struct krbParmBlock	{
	char	*uName;
	char	*uInstance;
	char	*uRealm;  /* also where local realm or mapping realm passed */
	char	*sName;
	char	*sInstance;
	char	*sRealm;
	char	*host;		/* also netorhost */
	int	admin;		/* isadmin, mustadmin */
	int	*itemNumber;
	int	*adminReturn;	/* when it needs to be passed back */
	CREDENTIALS *cred;
	char	*fullname;	/* for kname_parse */
	int 	result;		/* general purpose integer return value */
};
typedef struct krbParmBlock krbParmBlock;

/*
 * Mac_stubs.c exports a variable for communication with the Kerberos
 * driver, for hardy souls using the driver interface.
 */
extern short mac_stubs_kdriver;
#endif /* Krb_Driver_h */
