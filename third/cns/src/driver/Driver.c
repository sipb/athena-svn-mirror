/*
	main.c 
	Mac System Extension for the Kerberos Library
	
	Copyright 1992 by Cornell University
	
	Initial coding 1/92 by Peter Bosanko.
*/

/* The mac headers needed to compile this file */
#include <Files.h>
#include <Devices.h>
#include <Errors.h>
#include <OSUtils.h>

#include "krb.h"			/* This must precede krb_driver.h */
#include "krb_driver.h"

#include "kerberos.h"		
#include "des.h"			
#include "memcache.h"

/*
 * The headers that will be used when the driver guts plug in...
 */

#if 0
#include <stdarg.h>
#include "Machine Dependencies.h"
#include "UDPobj.h"
#include "debug.h"
#include "krb_Store.h"
#include "password.h"
#endif

#define		IORES	(pb->ioResult)
#define		PARM	((krbParmBlock *) *((long *) &(pb->csParam)) )
#define		HIPARM	((krbHiParmBlock *) *((long *) &(pb->csParam)) )
#define		kRdCmd  2
#define		kWrCmd  3

/* THINK C calls the driver's main() routine with a third parameter which
   is a bare number that defines which driver entry point was called by the
   Device Manager.  We decided all those bare numbers were hard to fathom,
   so here is an enum that THINK should have provided and documented.  This
   came from page 273 of the THINK C/Symantec C++ User's Guide, version 6, section
   "How to write main() for a driver".  */
   
enum DriverCodes {
	DriverOpenCode = 0,
	DriverPrimeCode = 1,
	DriverControlCode = 2,
	DriverStatusCode = 3,
	DriverCloseCode = 4
};


/* 
FIXME jcm - want this global but no globals in MPW driver.
try allowing some global data in Think
*/

int padData = -1;
int padDatae = -1;

/* IMPORTANT THINK C stuff....
 * "main" is in DCOD segment 1 which is loaded when the driver "open" is called.
 * This happens from the INIT that loaded the driver.
 * The second DCOD segment contains the Krb_Store stuff.  By creating the object during
 * the driver "open" the second segment also is loaded.  SEGMENTS NOT LOADED DURING THE
 * OPEN CALL WILL NEVER BE LOADED.
*/ 

extern int gettimeofdaynet (struct timeval *tp, struct timezone *tz);

/*
FIXME jcm - stub out these hooks for driver functionality.

extern int Encrypt(char *buf, unsigned long buflen,unsigned char *sessionKey, 
		unsigned long fAddr, unsigned short fPort,
		unsigned long lAddr, unsigned short lPort,
		char *outBuf, unsigned long *outlen, Key_schedule schedule);
extern int Decrypt(char *buf, unsigned long buflen,unsigned char *sessionKey, 
		unsigned long fAddr, unsigned short fPort,
		unsigned long lAddr, unsigned short lPort, 
		unsigned long *dataOffset, unsigned long *dataLength, Key_schedule schedule);
*/

OSErr
main (CntrlParam *pb,DCtlPtr devCtlEnt, short n)
{
	OSErr err;
	int numSessions;
	THz saveZone,sysHeap;

/* 
	Ensure that any calls to C library memory allocation routines 
	from the ported/able krb and des libraries allocate their memory 
	in the system heap and not in the heap of the application making
	calls to the Kerberos driver.  The application may quit while
	the memory it allocates (eg. the ticket cache) is still in use 
	by the kerberos driver and other applications.  
*/
	
	saveZone = GetZone();
	sysHeap = SystemZone();
	SetZone(sysHeap);

	switch (n) {
		case DriverOpenCode:
			devCtlEnt->dCtlFlags |= dCtlEnable | dStatEnable | dNeedLock;
			
			if(devCtlEnt->dCtlStorage == 0)
				return cKrbCorruptedFile;
				
			/* Comments claim that we must call in each seg. during the open.
			   Indeed, we seem to get "System Error 15 ("Segment Loader error:
			   A GetResource call to read a segment into memory failed.")
			   later, if we don't.  So let's try.  */
			{	
				static char astr[] = "****";
				static char bstr[] = "yyyy";
				static unsigned KRB_INT32 toss;
				static const char *string;
		
				/* Call in the ANSI library segment */
				strcpy(astr,bstr);
			
				/* Call in the KrbLib segment */
				string = krb_get_err_text (KSUCCESS);
				
				/* Call in the DesLib segment */
				toss = TIME_GMT_UNIXSEC;
			}
						
			return IORES = 0;
			break;
		case DriverPrimeCode:
				switch (pb->ioTrap & 0x00FF){
				case kRdCmd :
					return (IORES = readErr);
				case kWrCmd :
					return (IORES = writErr);
				default :
					break;
			}
			return(0);
			break;
		case DriverControlCode:
			switch (pb->csCode) {

				case cKrbGetLocalRealm:
					err = krb_get_lrealm (PARM->uRealm, 1);
					break;
							
				case cKrbSetLocalRealm:
					err = SetLocalRealm(PARM->uRealm);
					break;

				case cKrbGetRealm:
					{
					char* uRealmPtr;
					uRealmPtr = krb_realmofhost(PARM->host);
					if (!uRealmPtr)
						err = KFAILURE;
					else {
						err = KSUCCESS;
						strcpy(PARM->uRealm, uRealmPtr);
					}
					}
					break;
					
				case cKrbKnameParse:
					err = kname_parse (PARM->uName, PARM->uInstance, PARM->uRealm,
										PARM->fullname);
					break;
													
				case cKrbGetTfFullname:
					err = krb_get_tf_fullname (PARM->fullname, PARM->uName, 
										PARM->uInstance, PARM->uRealm);
					break;

				case cKrbGetUserName:
					{
						char *temp;
						
						err = KSUCCESS;
						temp = krb_get_default_user();
						if (temp)
							strcpy(HIPARM->user, temp);
						else
							err = KFAILURE;
					}
					break;
					
				case cKrbSetUserName:
					err = krb_set_default_user(HIPARM->user);
					break;
			
				case cKrbGetErrText:
					PARM->uName = (char *) krb_get_err_text(PARM->admin);
					err = KSUCCESS;
					break;
				
				case cKrbCacheInitialTicket:
					err =  CacheInitialTicket(HIPARM->service);
					break;
		
				case cKrbGetPwInTkt:
					err = krb_get_pw_in_tkt(PARM->uName, PARM->uInstance, PARM->uRealm,
						PARM->sName, PARM->sInstance, PARM->admin, PARM->fullname);
					break;
							
				case cKrbGetTicketForService:
					err = krb_get_ticket_for_service(HIPARM->service, HIPARM->buf,
					 &(HIPARM->buflen), HIPARM->checksum,
					 /* FIXME */ (unsigned char *)HIPARM->sessionKey,
					 *((Key_schedule *) HIPARM->schedule ), "", false);
					break;
					
				case cKrbGetAuthForService:
					err =  krb_get_ticket_for_service(HIPARM->service, HIPARM->buf,
					 &(HIPARM->buflen),HIPARM->checksum, 
					 /* FIXME */ (unsigned char *)HIPARM->sessionKey,
					 *((Key_schedule *) HIPARM->schedule ),
					 HIPARM->applicationVersion , true);
					break;

				case cKrbGetCredentials:
					err = krb_get_cred (PARM->cred->service, PARM->cred->instance, 
										PARM->cred->realm, PARM->cred);
					break;
					
				case cKrbAddCredentials:
					err = krb_save_credentials(PARM->cred->service, PARM->cred->instance, 
							PARM->cred->realm, PARM->cred->session, 
							PARM->cred->lifetime, PARM->cred->kvno,
							&(PARM->cred->ticket_st), PARM->cred->issue_date);
					
					break;
						
				case cKrbDeleteCredentials:
					err = krb_delete_cred (PARM->sName, PARM->sInstance, PARM->sRealm);
					break;
				
				case cKrbGetNthCredentials:
					err = krb_get_nth_cred( PARM->sName, PARM->sInstance, PARM->sRealm, 
											*(PARM->itemNumber));
					break;	
						
				case cKrbGetNumCredentials:
					*(PARM->itemNumber) = krb_get_num_cred ();
					if (*(PARM->itemNumber) < 0)
						err = KFAILURE;
					else
						err = KSUCCESS;
					break;
					
				case cKrbAddRealmMap:
 					err = AddRealmMap(PARM->host, PARM->uRealm);
 					break;
 					
 				case cKrbDeleteRealmMap:
 					err = DeleteRealmMap(PARM->host) ;
 					break;
 					
 				case cKrbGetNthRealmMap:
 					err = GetNthRealmMap(*(PARM->itemNumber), PARM->host,
 							 PARM->uRealm) ;
 					break;
 					
 				case cKrbAddServerMap:
 					err = AddServerMap(PARM->uRealm, PARM->host, PARM->admin) ;
 					break;
 					
 				case cKrbDeleteServerMap:
 					err = DeleteServerMap(PARM->uRealm, PARM->host) ;
 					break;
					
 				case cKrbGetNthServerMap:
 					err = GetNthServerMap(*(PARM->itemNumber), PARM->uRealm,
 											PARM->host, PARM->adminReturn) ;
 					break;
 				
 				case cKrbGetNumSessions:
					err = GetNumSessions(PARM->itemNumber) ;
					break;
							
				case cKrbGetNthSession:
					err = GetNthSession(*(PARM->itemNumber), PARM->uName, 
											PARM->uInstance, PARM->uRealm) ;
					break;

				case cKrbDeleteSession:
					err = DeleteSession(PARM->uName, PARM->uInstance, PARM->uRealm);
					break;
		
 				case cKrbDeleteAllSessions:
 					{
					err = dest_all_tkts();
					/*
					FIXME jcm - once SetPassword is in our tree re-enable call
					SetPassword("");
					*/
					break;	
					}
					
				case cKrbSetPassword:
					/* FIXME jcm - once SetPassword is in our tree re-enable call
					SetPassword(HIPARM->user); 
					*/
					err = 0;
					break;
						
 				case cKrbGetDesPointers:
					{
					    long *lp, a4l, np;
					    
						lp = (long *)((long *)pb->csParam)[0]; /* Pointer to return block */
						np = ((long *)pb->csParam)[1]; 	/* Number of pointers requested */
						
						/* If user is requesting more than we know about,
						    she must want a newer driver
						 */
						if (np > 10) {
							err = cKrbOldDriver;
							break;
						}

						asm {					/* Driver's A4 (globals) */
							move.l a4, a4l
						}
						if (np >= 1)
							*lp++ = a4l;
						if (np >= 2)
							*lp++ = (long)des_new_random_key;
						if (np >= 3)
							*lp++ = (long)des_ecb_encrypt;
						if (np >= 4)
							*lp++ = (long)des_set_random_generator_seed;
						if (np >= 5)
							*lp++ = (long)des_key_sched;
						if (np >= 6)
							*lp++ = (long)des_init_random_number_generator;
						if (np >= 7)
							*lp++ = (long)des_pcbc_encrypt;
						if (np >= 8)
							*lp++ = (long)des_string_to_key;
						if (np >= 9)
							*lp++ = (long)des_quad_cksum;
						if (np >= 10)
							*lp++ = (long)gettimeofdaynet; 
							
						/* 
							FIXME jcm - return gettimeofdaynet when it's ported.
							
							*lp++ = (long) 0;
						*/
							
					}
					err = 0;
					break;
				
				case cKrbKillIO:
					err = cKrbNoKillIO;			/* all sync for now */
					break;
				
#if 0
								
				case cKrbCheckServiceResponse:
					err = (short) CheckServiceResponse(HIPARM->buf,
					 &(HIPARM->buflen), HIPARM->sessionKey,
					 HIPARM->fAddr,HIPARM->fPort,HIPARM->lAddr,HIPARM->lPort,
					 HIPARM->checksum, HIPARM->schedule );
					break;
					
				case cKrbDecrypt:
					err = (short) Decrypt(HIPARM->buf,HIPARM->buflen, 
					 HIPARM->sessionKey, 
			 		 HIPARM->fAddr,HIPARM->fPort,HIPARM->lAddr,HIPARM->lPort,
					 &(HIPARM->decryptOffset), &(HIPARM->decryptLength),
					 HIPARM->schedule );
			  		break;		
			  				
			  	case cKrbEncrypt:
					err = (short) Encrypt(HIPARM->buf,HIPARM->buflen,
					 HIPARM->sessionKey, 
			 		 HIPARM->fAddr,HIPARM->fPort,HIPARM->lAddr,HIPARM->lPort,
					 HIPARM->encryptBuf, &(HIPARM->encryptLength),
					 HIPARM->schedule );
			  		break;
			  																														
#endif 
				default :
					err = cKrbBadSelector;
					break;
	
			}
			if (err > 0) 
			/* positive return code means IO pending, 
			   so we change Kerberos return codes to be negative
			*/
				err = cKrbKerberosErrBlock - err;
			/*
			make sure name resolver DNR is released...
			FIXME jcm - no calls to external routines in driver.
			CloseResolver();		
			*/
			return (IORES = err);
			break;
		case DriverStatusCode:
			return IORES = 0;
			break;
		case DriverCloseCode:
			/* See page 278, "How to return from a driver", in Think C 6.0
			   User's Guide */
			return(-24 /* closeErr but I can't find where it's defined */);
			break;
	}
	/* restore the current heap zone to be the one the driver was entered with */
    SetZone(saveZone);
}


/*
 * Glue routines to hook up the MPW DRVRRuntime.o with code written for the THINK C
 * driver regime (main gets third argument saying which driver entry point was called).
 */
pascal OSErr
drvrclose (CntrlParam *pb,DCtlPtr devCtlEnt)
{
    return main (pb, devCtlEnt, DriverCloseCode);
}

pascal OSErr
drvropen (CntrlParam *pb,DCtlPtr devCtlEnt)
{
    return main (pb, devCtlEnt, DriverOpenCode);
}

pascal OSErr
drvrprime (CntrlParam *pb,DCtlPtr devCtlEnt)
{
    return main (pb, devCtlEnt, DriverPrimeCode);
}

pascal OSErr
drvrstatus (CntrlParam *pb,DCtlPtr devCtlEnt)
{
    return main (pb, devCtlEnt, DriverStatusCode);
}

pascal OSErr
drvrcontrol (CntrlParam *pb,DCtlPtr devCtlEnt)
{
    return main (pb, devCtlEnt, DriverControlCode);
}

