/*
 * (c) Copyright 1992 Hewlett-Packard Company.  All Rights Reserved.
 *
 * This source is for demonstration purposes only.
 *
 * Since this source has not been debugged for all situations, it will not be
 * supported by Hewlett-Packard in any manner.
 *
 * THIS MATERIAL IS PROVIDED "AS IS".  HEWLETT-PACKARD MAKES NO WARRANTY OF
 * ANY KIND WITH REGARD TO THIS MATERIAL.  HEWLETT-PACKARD SHALL NOT BE LIABLE
 * FOR ERRORS CONTAINED HEREIN OR FOR DIRECT, INDIRECT, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES IN CONNECTION WITH THE FURNISHING, PERFORMANCE, OR
 * USE OF THIS MATERIAL.  HEWLETT-PACKARD ASSUMES NO RESPONSIBILITY FOR THE
 * USE OR RELIABILITY OF THIS SOFTWARE.
 */

#ifndef lint
static char Productid[] = "@(#) $Revision: 1.1 $";
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/transcript-v2.1/hpquery.c,v 1.1 1993-10-12 05:28:10 probe Exp $";
#endif

/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/transcript-v2.1/hpquery.c,v $
 * $Revision: 1.1 $
 * $State: Exp $ $Locker:  $
 * $Date: 1993-10-12 05:28:10 $
 *
 * hpnpadmin.c - send SNMP GetRequests and SetRequests to a network
 *               peripheral using the CMU SNMP library routines.
 *               This code should be compiled and then linked with libsnmp.a
 *		 from the 1.0 release of the CMU SNMP distribution.
 *
 *
 * HPNP_XSTAT	The HPNP_XSTAT compile flag, when defined, enables extended
 *		status and statistics information that is not present in the
 *		standard version of hpnpadmin shipped with the host software
 *		distributions.  This extended status and statistics
 *		information is primarily experimental and probably of little
 *		value to the user.
 *
 * HPUX7	The HPUX7 compile flag, when defined, includes code for
 *		a string compare routine needed for HP-UX 7.0
 *
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>

#include "snmp.h"
#include "snmp_impl.h"
#include "asn1.h"
#include "snmp_api.h"
#include "snmp_client.h"


/*
 *	Structure for passing SNMP variable values
 */

typedef union SnmpVal {
	long            *integer;
	u_char          *string;
	oid             *objid;
} snmp_val;

/*
 *	Additional ASN types
 */

#define ASN_U_INTEGER   (0x07) /* not a true asn1 type */
#define ASN_IPADDRESS   (ASN_APPLICATION | 0)
#define ASN_COUNTER	(ASN_APPLICATION | 1)
#define ASN_GAUGE	(ASN_APPLICATION | 2)
#define ASN_TIMETICKS   (ASN_APPLICATION | 3)
#define ASN_OPAQUE	(ASN_APPLICATION | 4)

/*
 *	Flag to enable or disable SNMP PDU dumps
 */

int snmp_dump_packet = 0;

/*
 *	Misc externs
 */

extern int snmp_errno;
char *print_response();
void print_stat();
char *get_hpnpsnmp_passwd();
char *get_hpnpsnmp_passwd_default();

/*
 *    The system object class identifiers.
 */

#define MAXOBJLEN  25

struct object {
	int           name_length;
	oid           name[MAXOBJLEN];
} ;

/* MIB-II objects */
struct object sysObjectID       = {9,  {1, 3, 6, 1, 2, 1, 1, 2, 0 }};
struct object sysUpTime         = {9,  {1, 3, 6, 1, 2, 1, 1, 3, 0 }};
struct object sysContact        = {9,  {1, 3, 6, 1, 2, 1, 1, 4, 0 }};
struct object sysName           = {9,  {1, 3, 6, 1, 2, 1, 1, 5, 0 }};
struct object sysLocation       = {9,  {1, 3, 6, 1, 2, 1, 1, 6, 0 }};
struct object ipInReceives      = {9,  {1, 3, 6, 1, 2, 1, 4, 3, 0 }};
struct object ipInHdrErrors     = {9,  {1, 3, 6, 1, 2, 1, 4, 4, 0 }};
struct object ipInAddrErrors    = {9,  {1, 3, 6, 1, 2, 1, 4, 5, 0 }};
struct object ipInUnknownProtos = {9,  {1, 3, 6, 1, 2, 1, 4, 7, 0 }};
struct object ipOutRequests     = {9,  {1, 3, 6, 1, 2, 1, 4,10, 0 }};
struct object ipOutDiscards     = {9,  {1, 3, 6, 1, 2, 1, 4,11, 0 }};
struct object ipOutNoRoutes     = {9,  {1, 3, 6, 1, 2, 1, 4,12, 0 }};
struct object ipAdEntAddr       = {10, {1, 3, 6, 1, 2, 1, 4,20, 1, 1 }};
struct object ipAdEntNetMask    = {10, {1, 3, 6, 1, 2, 1, 4,20, 1, 3 }};
struct object ipRouteNextHop    = {14, {1, 3, 6, 1, 2, 1, 4,21, 1, 7, 0,0,0,0}};
struct object ipRoutingDiscards = {9,  {1, 3, 6, 1, 2, 1, 4,23, 0 }};
struct object icmpInMsgs        = {9,  {1, 3, 6, 1, 2, 1, 5, 1, 0 }};
struct object icmpInErrors      = {9,  {1, 3, 6, 1, 2, 1, 5, 2, 0 }};
struct object icmpInDestUnreachs= {9,  {1, 3, 6, 1, 2, 1, 5, 3, 0 }};
struct object icmpInTimeExcds   = {9,  {1, 3, 6, 1, 2, 1, 5, 4, 0 }};
struct object icmpInSrcQuenchs  = {9,  {1, 3, 6, 1, 2, 1, 5, 6, 0 }};
struct object icmpInRedirects   = {9,  {1, 3, 6, 1, 2, 1, 5, 7, 0 }};
struct object icmpInEchos       = {9,  {1, 3, 6, 1, 2, 1, 5, 8, 0 }};
struct object icmpOutMsgs       = {9,  {1, 3, 6, 1, 2, 1, 5,14, 0 }};
struct object icmpOutDestUnreach= {9,  {1, 3, 6, 1, 2, 1, 5,16, 0 }};
struct object icmpOutTimeExcds  = {9,  {1, 3, 6, 1, 2, 1, 5,17, 0 }};
struct object icmpOutEchoReps   = {9,  {1, 3, 6, 1, 2, 1, 5,22, 0 }};
struct object tcpInSegs         = {9,  {1, 3, 6, 1, 2, 1, 6,10, 0 }};
struct object tcpInErrs         = {9,  {1, 3, 6, 1, 2, 1, 6,14, 0 }};
struct object tcpOutSegs        = {9,  {1, 3, 6, 1, 2, 1, 6,11, 0 }};
struct object udpInDatagrams    = {9,  {1, 3, 6, 1, 2, 1, 7, 1, 0 }};
struct object udpInNoPorts      = {9,  {1, 3, 6, 1, 2, 1, 7, 2, 0 }};
struct object udpInErrors       = {9,  {1, 3, 6, 1, 2, 1, 7, 3, 0 }};
struct object udpOutDatagrams   = {9,  {1, 3, 6, 1, 2, 1, 7, 4, 0 }};
struct object snmpInPkts        = {9,  {1, 3, 6, 1, 2, 1,11, 1, 0 }};
struct object snmpOutPkts       = {9,  {1, 3, 6, 1, 2, 1,11, 2, 0 }};
struct object snmpInBadCmnty    = {9,  {1, 3, 6, 1, 2, 1,11, 4, 0 }};
struct object snmpInBadCmntyUses= {9,  {1, 3, 6, 1, 2, 1,11, 5, 0 }};
struct object snmpInGetRequests = {9,  {1, 3, 6, 1, 2, 1,11,15, 0 }};
struct object snmpInGetNexts    = {9,  {1, 3, 6, 1, 2, 1,11,16, 0 }};
struct object snmpInSetRequests = {9,  {1, 3, 6, 1, 2, 1,11,17, 0 }};
struct object snmpOutTraps      = {9,  {1, 3, 6, 1, 2, 1,11,29, 0 }};
struct object snmpEnableAuth    = {9,  {1, 3, 6, 1, 2, 1,11,30, 0 }};

/* enterprise objects */
struct object hpnpadmin_obj     = {10, {1, 3, 6, 1, 4, 1,11, 2, 3, 9}};

/* npSys group */
struct object npSysState                = {13, {1, 3, 6, 1,4,1,11,2,4,3,1,1,0}};
struct object npSysStatusMessage        = {13, {1, 3, 6, 1,4,1,11,2,4,3,1,2,0}};
struct object npSysPeripheralFatalError = {13, {1, 3, 6, 1,4,1,11,2,4,3,1,3,0}};
struct object npSysCardFatalError       = {13, {1, 3, 6, 1,4,1,11,2,4,3,1,4,0}};
struct object npSysMaximumWriteBuffers  = {13, {1, 3, 6, 1,4,1,11,2,4,3,1,5,0}};
struct object npSysMaximumReadBuffers   = {13, {1, 3, 6, 1,4,1,11,2,4,3,1,6,0}};
struct object npSysTotalBytesRecvs      = {13, {1, 3, 6, 1,4,1,11,2,4,3,1,7,0}};
struct object npSysTotalBytesSent       = {13, {1, 3, 6, 1,4,1,11,2,4,3,1,8,0}};
struct object npSysCurrReadReq          = {13, {1, 3, 6, 1,4,1,11,2,4,3,1,9,0}};
/* npConns group */
struct object npConnsAccepts    = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 4, 1, 0}};
struct object npConnsDenys      = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 4, 3, 0}};
struct object npConnsDenysIP    = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 4, 4, 0}};
struct object npConnsAborts     = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 4, 5, 0}};
struct object npConnsAbortReason = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 4, 6,0}};
struct object npConnsAbortIP    = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 4, 7, 0}};
struct object npConnsAbortPort  = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 4, 8, 0}};
struct object npConnsAbortTime  = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 4, 9, 0}};
struct object npConnsIP         = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 4,11, 0}};
struct object npConnsPort       = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 4,12, 0}};
struct object npConnsIdleTimeouts = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3,4,14,0}};
struct object npConnsNmClose    = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 4,15, 0}};
struct object npConnsBytesRecvd = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 4,16, 0}};
struct object npConnsBytesSent  = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 4,17, 0}};
/* npCfg group */
struct object npCfgSource       = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 5, 1, 0}};
struct object npCfgSiaddr       = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 5, 3, 0}};
struct object npCfgLogServer    = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 5, 5, 0}};
struct object npCfgSyslogFac    = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 5, 6, 0}};
struct object npCfgAccessState  = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 5, 7, 0}};
struct object npCfgAccess       = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 5, 9, 1}};
struct object npCfgAccessListAddress = {14, {1, 3, 6, 1, 4,1,11,2,4,3,5,9,1,2}};
struct object npCfgAccessListAddrMask = {14, {1, 3, 6, 1,4,1,11,2,4,3,5,9,1,3}};
struct object npCfgIdleTimeout  = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 5,10, 0}};
struct object npCfgLocalSub     = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 5,11, 0}};
/* npTcp group */
struct object npTcpInSegInOrd   = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 6, 1, 0}};
struct object npTcpInSegOutOf   = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 6, 2, 0}};
struct object npTcpInSegZero    = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 6, 3, 0}};
struct object npTcpInDiscards   = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 6, 4, 0}};
/* npCtl group */
struct object npCtlReconfigIP   = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 7, 1, 0}};
struct object npCtlReconfigTime = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 7, 3, 0}};
struct object npCtlCloseIP      = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 7, 4, 0}};

/* Setable objects */
struct object npCtlImageDump    = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 7, 6, 0}};
struct object npCtlCloseConnection = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3,7,7,0}};
struct object npCtlReconfig     = {13, {1, 3, 6, 1, 4, 1,11, 2, 4, 3, 7, 8, 0}};

/* trap destination list */
struct object trapDestination   = {13, {1, 3, 6, 1, 4, 1,11, 2,13, 1, 2, 1, 1}};

/* gdStatus group */
struct object gdStatusDisplay  = {14, {1, 3, 6, 1, 4, 1, 11, 2, 3, 9, 1,1,3,0}};
struct object gdStatusJobName  = {14, {1, 3, 6, 1, 4, 1, 11, 2, 3, 9, 1,1,4,0}};

/* gdStatus Entry group */
struct object gdStatusLineState = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,1,0}};
struct object gdStatusPaperState = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,2,0}};
struct object gdStatusInterventionState = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,3,0}};
struct object gdStatusNewMode = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,4,0}};
struct object gdStatusConnectionTerminationAck = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,5,0}};
struct object gdStatusPeripheralError = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,6,0}};
struct object gdStatusIoChannelResert = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,7,0}};
struct object gdStatusPaperOut = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,8,0}};
struct object gdStatusPaperJam = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,9,0}};
struct object gdStatusTonerLow = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,10,0}};
struct object gdStatusPagePunt = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,11,0}};
struct object gdStatusMemoryOut = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,12,0}};
struct object gdStatusIoActive = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,13,0}};
struct object gdStatusBusy = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,14,0}};
struct object gdStatusWait = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,15,0}};
struct object gdStatusInitialize = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,16,0}};
struct object gdStatusDoorOpen = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,17,0}};
struct object gdStatusPrinting = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,18,0}};
struct object gdStatusPaperOutput = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,19,0}};
struct object gdStatusReserved = {15, {1, 3, 6, 1, 4, 1,11,2,3,9,1,1,2,20,0}};

/* npi peripheral attributes */
struct object npNpiPeripheralAttributeCount = {13,{1,3,6,1,4,1,11,2,4,3,2,2,0}};
struct object npNpiPaeLinkDirection  = {14, {1, 3, 6, 1, 4,1,11,2,4,3,2,3,1,0}};
struct object npNpiPaeClass          = {14, {1, 3, 6, 1, 4,1,11,2,4,3,2,3,2,0}};
struct object npNpiPaeIdentification = {14, {1, 3, 6, 1, 4,1,11,2,4,3,2,3,3,0}};
struct object npNpiPaeRevision       = {14, {1, 3, 6, 1, 4,1,11,2,4,3,2,3,4,0}};
struct object npNpiPaeAppleTalk      = {14, {1, 3, 6, 1, 4,1,11,2,4,3,2,3,5,0}};
struct object npNpiPaeMessage        = {14, {1, 3, 6, 1, 4,1,11,2,4,3,2,3,6,0}};
struct object npNpiPaeVideoFlag      = {14, {1, 3, 6, 1, 4,1,11,2,4,3,2,3,7,0}};


/*
 *	Misc Global Variables
 */

int NoResponse = 0;
#define NORESPONSEMAX 2

query(peripheral, message)
char	*peripheral;
char	*message;
{
	char	*community = NULL;
   int	exit_val;
   struct snmp_session  *session ;
   struct snmp_session  session_var;
   struct hostent  *hp = NULL;

	if((inet_addr(peripheral) == -1) && 
				((hp = gethostbyname(peripheral)) == NULL)){
	    (void) fprintf(stderr, "%s: Unknown peripheral\n", peripheral);
	    return(-1);
	}

	/* default to public */
	if (community == NULL)
		community = "public";

#ifdef DEBUG
	fprintf(stdout, "cmnty lookup peripheral=(%s) h_name=(%s) cmnty=(%s)\n",
		peripheral, 
		((hp == NULL) ? "hp NULL" : hp->h_name),
		community);
#endif

	bzero((char *)&session_var, sizeof(struct snmp_session));
	session_var.peername = peripheral;
	session_var.community = (u_char *)community;
	session_var.community_len = strlen((char *)community);
	session_var.retries = 2;                        /* 2 retries */
	session_var.timeout = SNMP_DEFAULT_TIMEOUT;
	session_var.authenticator = NULL;
	snmp_synch_setup(&session_var);
	session = snmp_open(&session_var);
	if ( session == NULL ) {
	    (void) fprintf(stderr, "Unable to initialize SNMP session.\n");
	    return(-1);
	}

	if(!check_peripheral(session))
		(void) fprintf(stderr, 
			"WARNING: %s is not a network peripheral!\n", peripheral);

	exit_val = sys_state(session, message);
	if (exit_val) goto doit;

	exit_val = jam_stats(session, message);
	if (exit_val) goto doit;
	
	exit_val = paper_stats(session, message);
	if (exit_val) goto doit;

doit:
	snmp_close (session);
	return(exit_val);
}

/**********************************************
 * get() - get the value of the specified object
 **********************************************/

struct snmp_pdu *
get(session, obj, getnext)
struct snmp_session	*session ;
struct object   *obj;
int 		getnext;
{
    struct snmp_pdu	*request, *response;
    int 	status;
    int		count = 0;
    struct variable_list *vars;

#ifdef DEBUG
    printf("get: ");
    print_objid(obj->name, obj->name_length);
    printf("\n");
#endif

    if(getnext)
	request = snmp_pdu_create(GETNEXT_REQ_MSG);
    else
	request = snmp_pdu_create(GET_REQ_MSG);
    if ( request == NULL ) {
	perror("get: Unable to create PDU");
	return(NULL);
    }

    snmp_add_null_var(request, obj->name, obj->name_length);
    if (0) {	/* call does not return error nor set snmp_errno */
	perror ("get: Couldn't add variable");
	snmp_free_pdu(request);
	return(NULL);
    }

    response = NULL;
    status = snmp_synch_response(session, request, &response);
    snmp_free_pdu(request);
    if (status == STAT_SUCCESS){
	return(response);
    } else if (status == STAT_TIMEOUT){
	snmp_free_pdu(request);
	NoResponse++;
	(void) fprintf(stderr, "Error sending SNMP request.\n");
	(void) fprintf(stderr, "    Either the host can not be reached or the community name does not match\n");
	if(NoResponse == NORESPONSEMAX)
	    exit(-1);
	if (response)
	    snmp_free_pdu(response);
	return(NULL);
    } else {    /* status == STAT_ERROR [snmp_synch_rsp frees pdu] */
	snmp_free_pdu(request);
	(void) fprintf (stderr, "Error sending SNMP request.\n");
	/* todo - what about api_errstring() ?? */
	exit(-1);
    }
}

/**********************************************
 * set() - set the value of the specified object
 **********************************************/

struct snmp_pdu *
set(session, obj, obj_value)
struct snmp_session	*session ;
struct object   *obj;
int 		obj_value;
{
    struct snmp_pdu *request, *response;
    int 	    status;
    snmp_val	    value;

#ifdef DEBUG
    printf("set: ");
    print_objid(obj->name, obj->name_length);
    printf("\n");
#endif

    value.integer = (long *)&obj_value;

    request = snmp_pdu_create(SET_REQ_MSG);
    if ( request == NULL ) {
	perror("set: Unable to create PDU");
	return(NULL);
    }

    snmp_add_typed_var(request, obj->name, obj->name_length,
		     (u_char) ASN_INTEGER, (snmp_val *) &value, 1);
    if (0) {	/* call does not return error nor set snmp_errno */
	perror ("set: Couldn't add variable");
	snmp_free_pdu(request);
	return(NULL);
    }

    response = NULL;
    status = snmp_synch_response(session, request, &response);
    snmp_free_pdu(request);
    if (status == STAT_SUCCESS){
	return(response);
    } else if (status == STAT_TIMEOUT){
	snmp_free_pdu(request);
	NoResponse++;
	(void) fprintf(stderr, "Error sending SNMP request.\n");
	(void) fprintf(stderr, "    Either the host can not be reached or the community name does not match\n");
	if(NoResponse == NORESPONSEMAX)
	    exit(-1);
	if (response)
	    snmp_free_pdu(response);
	return(NULL);
    } else {    /* status == STAT_ERROR [snmp_synch_rsp frees pdu] */
	snmp_free_pdu(request);
	(void) fprintf (stderr, "Error sending SNMP request.\n");
	/* todo - what about api_errstring() ?? */
	exit(-1);
    }
}

/**********************************************
 * Add a typed variable with the requested name to the end of the list of
 * variables for this pdu.  Currently only ASN_INTEGER type is supported.
 **********************************************/

snmp_add_typed_var(pdu, name, name_length, type, value, flag)
    struct snmp_pdu *pdu;
    oid *name;
    int name_length;
    u_char type;
    snmp_val *value;
    int flag;
{
    struct variable_list *vars;

    if (pdu->variables == NULL){
	pdu->variables = vars = (struct variable_list *)malloc(sizeof(struct variable_list));
    } else {
	for(vars = pdu->variables; vars->next_variable; vars = vars->next_variable)
	    ;
	vars->next_variable = (struct variable_list *)malloc(sizeof(struct variable_list));
	vars = vars->next_variable;
    }

    vars->next_variable = NULL;
    vars->name = (oid *)malloc(name_length * sizeof(oid));
    bcopy((char *)name, (char *)vars->name, name_length * sizeof(oid));
    vars->name_length = name_length;
    vars->type = type;
    switch (type) {
	case ASN_INTEGER:
	    vars->val.integer = (long *)malloc(sizeof(long));
	    *(vars->val.integer) = *(value->integer);
	    vars->val_len = sizeof(long);
	    break;
	case GAUGE:
	case COUNTER:
	case TIMETICKS:
	case ASN_OCTET_STR:
	case IPADDRESS:
	case OPAQUE:
	case ASN_OBJECT_ID:
	case ASN_NULL:
	default:
	    fprintf(stderr,"snmp_add_typed_var: wrong or unsupported type: %d\n",
		    type);
	    exit(-1);
	    break;
    }
}

/**********************************************
 * access_test() - show whether host is allowed access to card
 **********************************************/

access_test(session, peripheral)
struct snmp_session	*session ;
char *peripheral;
{
	struct snmp_pdu *response;
	char hostname[256];

	hostname[0] = '\0';
	(void) gethostname(hostname, sizeof(hostname));
	response = get(session, &npCfgAccessState, 0);
	if(response != NULL){
	  if (response->errstat != SNMP_ERR_NOERROR) {
	    (void) printf("Access Check: %s\n", 
				snmp_errstring (response->errstat));
	    }
	  if (response->errstat != SNMP_ERR_NOERROR) {
	    (void) printf("Access Check: %s\n", 
				snmp_errstring (response->errstat));
	    return(0);
	  }
	  if(response->variables->type != ASN_INTEGER){
	    (void) fprintf(stderr, "unexpected variable type - %d\n", 
					response->variables->type);
	    return(0);
	  }
	  if(response->variables->val.integer[0] == 1){
	    (void) printf("%s is allowed access to %s\n", hostname, peripheral);
	    return(0);
	  } else if(response->variables->val.integer[0] == 2){
	    (void) printf("%s is denied access to %s\n", hostname, peripheral);
	    return(1);
	  } else {
	    (void) fprintf(stderr, "unexpected value - %d\n", 
					response->variables->val.integer[0]);
	    /* something went wrong ... */
	    return(0);
	  }
	}
      return(0);
}

/**********************************************
 * netperiph_test() - show whether the node is a NPI network peripheral
 **********************************************/

netperiph_test(session, peripheral)
struct snmp_session	*session ;
char *peripheral;
{
	if (check_peripheral(session)){
	    (void) printf("%s is a network peripheral\n", peripheral);
	    return(0);
	}else{
	    return(1);
	}
}

/**********************************************
 * check_peripheral() - return 1 if node is a network peripheral,
 *			0 if it is not.
 **********************************************/

int
check_peripheral(session)
struct snmp_session	*session ;
{
	struct snmp_pdu *response;
	int hpnetperiph, j;

	response = get(session, &sysObjectID, 0);
	hpnetperiph = 1;
	if(response != NULL){
	    /*
	     * The response will be 1 id larger than the id we are
	     * checking.  This allows for .1 to be printer, .2 to be
	     * a plotter, .3 to be an instrument, etc
	     */
	    if(response->variables->val_len <= hpnpadmin_obj.name_length){
		hpnetperiph = 0;
	    } else {
	      for(j = 0; (j < hpnpadmin_obj.name_length) && hpnetperiph; j++){
	        if(hpnpadmin_obj.name[j] != response->variables->val.objid[j])
		    hpnetperiph = 0;
	      }
	    }
	} else {
	    /*
	     * If no response, exit.
	     */
	    return(1);
	}

	return(hpnetperiph);
}

/**********************************************
 * sys_state
 *      Current System State 
 **********************************************/
sys_state(session, message)
struct snmp_session	*session;
char						*message;
{
	struct snmp_pdu *response, *response1, *response2;
	char *value;
	char buf[512];
	int config = 0;
	int ivalue;
	int exit_val;

	response = get(session, &npSysState, 0);
	if(response != NULL) {
		ivalue = response->variables->val.integer[0];

		switch(ivalue) {
			case 0:
			case 3:
			case 4:
				strcpy(message,"Printer is Online");
				exit_val = 0;
				break;
			case 1:
				strcpy(message,"Printer is OffLine");
				exit_val = 1;
				break;
			case 2:
				strcpy(message,"Printer is Initializing");
				exit_val = 0;
				break;
			case 5:
				strcpy(message,"Unknown Printer Error (Code NP)");
				exit_val = 5;
				break;
			default:
				strcpy(message,"Unknown Printer Error (Code NPD)");
				exit_val = 6;
				break;
		}
	}

	response = get(session, &npSysStatusMessage, 0);
	if(response != NULL) {
	    if (response->errstat != SNMP_ERR_NOERROR) {
	        (void) printf("%s\n", snmp_errstring(response->errstat));
			  exit_val = 9;
	        return(1);
	    }
	    sprintf_resp(buf, sizeof(buf), response->variables);

	    /** It is NOT READY OR Printing **/
		if((strncmp(buf, "ready", 5) == 0) ||
			(strncmp(buf, "print", 5) == 0) ||
			(strncmp(buf, "toner", 5) == 0)) {		
			/* do nothing */
		}
		else {
			strcpy(message,buf);
			exit_val=7;
	    }
	}

	return(exit_val);
}

/**********************************************
 * jam_status
 *		 Jam Status
 **********************************************/
jam_stats(session, message)
struct snmp_session	*session;
char						*message;
{
	struct snmp_pdu *response, *response1, *response2;
	char *value;
	int config = 0;
	int ivalue;
	int exit_val;
	int done, i, printheader, trapdests;
	unsigned long uptime, reconfigtime;
	struct object last_addr, last_mask;

	response = get(session, &gdStatusPaperJam, 0);
	if(response != NULL) {
		ivalue = response->variables->val.integer[0];

		switch(ivalue) {
			case 0:
				strcpy(message,"No Jam Detected");
				exit_val = 0;
				break;
			case 1:
				strcpy(message,"Paper Path is Jammed");
				exit_val = 1;
				break;
			default:
				strcpy(message,"Check Paper Path");
				exit_val = 2;
				break;
		}
	}

	return(exit_val);
}

/**********************************************
 * paper_status
 *		  Paper Status
 **********************************************/
paper_stats(session, message)
struct snmp_session	*session;
char						*message;
{
	struct snmp_pdu *response, *response1, *response2;
	char *value;
	int config = 0;
	int ivalue;
	int exit_val;
	int done, i, printheader, trapdests;
	unsigned long uptime, reconfigtime;
	struct object last_addr, last_mask;

   response = get(session, &gdStatusPaperOut, 0);
	if(response != NULL) {
		ivalue = response->variables->val.integer[0];

		switch(ivalue) {
			case 0:
				strcpy(message,"OK");
				exit_val = 0;
				break;
			case 1:
				strcpy(message,"Out of Paper or No Paper Cassette");
				exit_val = 1;
				break;
			case 2:
				strcpy(message,"Manual Feed Required");
				exit_val = 2;
				break;
			default:
				strcpy(message,"Check Paper Path");
				exit_val = 3;
				break;
		}
	}

	return(exit_val);
}

/**********************************************
 * print_response() - print label, object value from SNMP response, and suffix
 *		      if lookup_addr is 1, do a hostname lookup on IP addr
 *		      then free SNMP response PDU
 **********************************************/

char *
print_response(label, response, lookup_addr, suffix)
char *label;
struct snmp_pdu *response;
int lookup_addr;
char *suffix;
{
	static char buf[512];
	struct hostent *hp;
	long addr;

	(void) printf("%-20s: ", label);

	if (response->errstat != SNMP_ERR_NOERROR) {
	/* need to fix this up */
		(void) printf("%s\n", snmp_errstring (response->errstat));
		return (NULL);
	}

	sprintf_resp(buf, sizeof(buf), response->variables);
	if(lookup_addr){
		addr = (long) response->variables->val.integer[0];
		if(addr == (long)0)
		    (void) printf("not specified");
		else if(addr == (long) -1)
		    (void) printf("255.255.255.255");
		else {
		    hp = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
		    if(hp == NULL)
			(void) printf("??? (%s)", buf);
		    else
			(void) printf("%s (%s)", hp->h_name, buf);
		}
	} else {
		if(buf[0] == '\0')
		    (void) printf("not specified");
		else
		    (void) printf("%s", buf);
	}

	if(suffix == NULL)
		(void) printf("\n");
	else
		(void) printf(" %s\n", suffix);
	snmp_free_pdu (response) ;
	return(buf);
}

/**********************************************
 * print_stat() - print label and object value from SNMP response
 *		  then free SNMP pdu
 **********************************************/

void
print_stat(label, response, indent)
char *label;
struct snmp_pdu *response;
int indent;
{
	char buf[512];

	if (response->errstat != SNMP_ERR_NOERROR) {
		(void) printf("%s\n", snmp_errstring (response->errstat));
		return;
	}

	sprintf_resp(buf, sizeof(buf), response->variables);
	if(indent)
	    (void) printf("    ");
	(void) printf("        %s %s\n", buf, label);

	snmp_free_pdu (response) ;
}

/**********************************************
 * sprintf_resp() - print object value from SNMP variables list into buffer
 **********************************************/

sprintf_resp(buf, bufsiz, vars)
char *buf;
int bufsiz;
struct variable_list *vars;
{
	unsigned long timeticks, seconds, minutes, hours, days;
	int size;

	switch(vars->type){
	    case ASN_INTEGER:
		(void) sprintf(buf, "%d", *vars->val.integer);
		break;
	    case ASN_U_INTEGER:
		(void) sprintf(buf, "%u", *vars->val.integer);
		break;
	    case ASN_NULL:
		(void) sprintf(buf, "NULL");
		break;
	    case ASN_OCTET_STR:
		size = vars->val_len;
		if (size > (bufsiz-1))
		    size = bufsiz-1;
		if(size > 0)
		    (void) strncpy(buf, (char *)vars->val.string, size);
		buf[size] = '\0';
		break;
	    case ASN_IPADDRESS:
		(void) sprintf(buf, "%d.%d.%d.%d", vars->val.string[0],
					    vars->val.string[1],
					    vars->val.string[2],
					    vars->val.string[3]);
		break;
	    case ASN_COUNTER:
	    case ASN_GAUGE:
		(void) sprintf(buf, "%lu", *vars->val.integer);
		break;
	    case ASN_TIMETICKS:
		timeticks = *vars->val.integer;
		if(timeticks == (unsigned long) 0){
		    (void) sprintf(buf, "not available");
		    break;
		}
		timeticks /= 100;
		days = timeticks / 86400;
		timeticks = timeticks % 86400;
		hours = timeticks / 3600;
		timeticks %= 3600;
		minutes = timeticks / 60;
		seconds = timeticks % 60;
		if (days == 0)
		    (void) sprintf(buf, "%d:%02d:%02d", 
						hours, minutes, seconds);
		else if (days == 1)
		    (void) sprintf(buf, "1 day, %d:%02d:%02d", 
						hours, minutes, seconds);
		else 
		    (void) sprintf(buf, "%d days, %d:%02d:%02d",
					days, hours, minutes, seconds);
		break;
	    default:
		(void) fprintf(stderr,"unsupported SNMP response type %d\n",
				vars->type);
	}
}


/**********************************************
 * hpnpsnmp community name file lookup routines
 **********************************************/

#define DEF_NODE_PASSWD_FILE 	"/usr/lib/hpnp/hpnpsnmp"
#define DEF_NODE_PASSWD		"public"
#define SPACES			" \t\n"

static FILE *pw_fp = NULL;
static char buf[BUFSIZ];

/* -------------------------------------------------------------- */

static char *
getnextline(fp)
FILE *fp;
{
	char *cp;
	char *bufcp = buf, *first = NULL;
	int size = BUFSIZ;
	
nextline:
	while( fgets(bufcp, size, fp) != NULL ) {
	    for( cp = bufcp; *cp != '\0'; cp++) {
		if ( *cp == '#') {			/* Comment */
			if ( first == NULL)
				goto nextline;
			*cp = '\0';
			goto done;
		}
		if ( *cp == '\\' && *(cp+1) == '\n' ) {	/* Continuation */
			*cp = '\0';
			size = size - (cp - bufcp);
			bufcp = cp;
			goto nextline;
		}
		if ( first == NULL && !isspace(*cp))	/* First non-blank */
			first = cp;
	    }
	    if (first != NULL)
		break;
	}
done:
	return(first);
}

/* -------------------------------------------------------------- */

#ifdef HPUX7
int 
strcasecmp(s,t)
char *s,*t;
{
    while((*s != '\0') && (*t != '\0')){
	if(tolower(*s) != tolower(*t))
	  return(1);
	s++; t++;
    }
    return(*s != *t);
}
#endif

/* -------------------------------------------------------------- */

char *
parse_hpnpsnmp_passwd(nodename, line)
char *nodename;
char *line;
{
    char *name, *passwd, *proxy;

    if ( (name = strtok(line, SPACES)) == NULL) {
#ifdef DEBUG
	fprintf(stderr,"ERROR: Could not find name, discarding \"%s\"\n", line);
#endif
	return(NULL);
    }

    if ( (passwd = strtok(NULL, SPACES)) == NULL) {
#ifdef DEBUG
	fprintf(stderr,"ERROR: No community name for %s, discarding\n", name);
#endif
	return(NULL);
    }

    proxy = strtok(NULL, SPACES); /* proxy name is optional */

    if ( !strcasecmp(name, nodename) )
	return(passwd);
    else
	return(NULL);
}

/* -------------------------------------------------------------- */

char *
get_hpnpsnmp_passwd(nodename)
char *nodename;
{
	char *line, *passwd;

	/* Open passwd file  */
	if ( (pw_fp = fopen(DEF_NODE_PASSWD_FILE, "r")) == NULL)
		return(NULL);

	/* Search thru file for nodename. If found, return cmnty name */
	while ( (line = getnextline(pw_fp)) != NULL) {
		if ( (passwd = parse_hpnpsnmp_passwd(nodename, line)) != NULL) {
			fclose(pw_fp);
			return (passwd);
		}
	}

	/* Nodename was not found in file */
	fclose(pw_fp);
	return(NULL);
}

/* -------------------------------------------------------------- */

char *
get_hpnpsnmp_passwd_default(nodename, Default)
char *nodename;
char *Default;
{
    char *passwd;

    if ( (passwd = get_hpnpsnmp_passwd(nodename)) == NULL &&
         (passwd = Default) == NULL &&
         (passwd = get_hpnpsnmp_passwd("default")) == NULL )
                passwd = DEF_NODE_PASSWD;
    return(passwd);
}
