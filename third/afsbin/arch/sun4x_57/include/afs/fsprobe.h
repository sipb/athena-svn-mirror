/*
 * (C) Copyright Transarc Corporation 1989
 * Licensed Materials - Property of Transarc
 * All Rights Reserved.
 */

#ifndef __fsprobe_h
#define	__fsprobe_h  1

/*------------------------------------------------------------------------
 * fsprobe.h
 *
 * Interface to the AFS FileServer probe facility.  With the routines
 * defined here, the importer can gather statistics from the given set
 * of FileServers at regular intervals, or force immediate collection.
 *
 * Author:
 *	Ed. Zayas
 *	Transarc Corporation
 *------------------------------------------------------------------------*/

#include <sys/types.h>			/*Basic system types*/
#include <netinet/in.h>			/*Internet definitions*/
#include <netdb.h>			/*Network database library*/
#include <sys/socket.h>			/*Socket definitions*/
#include <rx/rx.h>			/*Rx definitions*/
#include <afs/afsint.h>			/*AFS FileServer interface*/
#include <afs/volser.h>
#include <afs/volint.h>

struct ProbeViceStatistics {
    u_int32 CurrentMsgNumber;
    u_int32 OldestMsgNumber;
    u_int32 CurrentTime;
    u_int32 BootTime;
    u_int32 StartTime;
    int32	  CurrentConnections;
    u_int32 TotalViceCalls;
    u_int32 TotalFetchs;
    u_int32 FetchDatas;
    u_int32 FetchedBytes;
    int32	  FetchDataRate;
    u_int32 TotalStores;
    u_int32 StoreDatas;
    u_int32 StoredBytes;
    int32	  StoreDataRate;
    u_int32 TotalRPCBytesSent;
    u_int32 TotalRPCBytesReceived;
    u_int32 TotalRPCPacketsSent;
    u_int32 TotalRPCPacketsReceived;
    u_int32 TotalRPCPacketsLost;
    u_int32       TotalRPCBogusPackets;
    int32	  SystemCPU;
    int32	  UserCPU;
    int32	  NiceCPU;
    int32	  IdleCPU;
    int32	  TotalIO;
    int32	  ActiveVM;
    int32	  TotalVM;
    int32	  EtherNetTotalErrors;
    int32	  EtherNetTotalWrites;
    int32	  EtherNetTotalInterupts;
    int32	  EtherNetGoodReads;
    int32	  EtherNetTotalBytesWritten;
    int32	  EtherNetTotalBytesRead;
    int32	  ProcessSize;
    int32	  WorkStations;
    int32	  ActiveWorkStations;
    int32	  Spare1;
    int32	  Spare2;
    int32	  Spare3;
    int32	  Spare4;
    int32	  Spare5;
    int32	  Spare6;
    int32	  Spare7;
    int32	  Spare8;
    ViceDisk	  Disk[VOLMAXPARTS];
};


/*
  * Connection information per FileServer host being probed.
  */
struct fsprobe_ConnectionInfo {
    struct sockaddr_in skt;		/*Socket info*/
    struct rx_connection *rxconn;	/*Rx connection*/
    struct rx_connection *rxVolconn;	/*Rx connection to Vol server*/
    struct partList partList;		/*Server part list*/
    int32 partCnt;      			/*# of parts */
    char hostName[256];			/*Computed hostname*/
};

/*
  * The results of a probe of the given set of FileServers.  The ith
  * entry in the stats array corresponds to the ith connected server.
  */
struct fsprobe_ProbeResults {
    int probeNum;			/*Probe number*/
    int32 probeTime;			/*Time probe initiated*/
    struct ProbeViceStatistics *stats;	/*Ptr to stats array for servers*/
    int *probeOK;			/*Array: was latest probe successful?*/
};

extern int fsprobe_numServers;				/*# servers connected*/
extern struct fsprobe_ConnectionInfo *fsprobe_ConnInfo;	/*Ptr to connections*/
extern int numCollections;				/*Num data collections*/
extern struct fsprobe_ProbeResults fsprobe_Results;	/*Latest probe results*/

extern int fsprobe_Init();
    /*
     * Summary:
     *    Initialize the fsprobe module: set up Rx connections to the
     *	  given set of servers, start up the probe and callback LWPs,
     *	  and associate the routine to be called when a probe completes.
     *
     * Args:
     *	  int a_numServers		    : Num. servers to connect to.
     *	  struct sockaddr_in *a_socketArray : Array of server sockets.
     *	  int a_ProbeFreqInSecs		    : Probe frequency in seconds.
     *	  int (*a_ProbeHandler)()	    : Ptr to probe handler fcn.
     *	  int a_debug			    : Turn debugging output on?
     *
     * Returns:
     *	  0 on success,
     *	  Error value otherwise.
     */

extern int fsprobe_ForceProbeNow();
    /*
     * Summary:
     *    Force an immediate probe to the connected FileServers.
     *
     * Args:
     *	  None.
     *
     * Returns:
     *	  0 on success,
     *	  Error value otherwise.
     */

extern int fsprobe_Cleanup();
    /*
     * Summary:
     *    Clean up our memory and connection state.
     *
     * Args:
     *	  int a_releaseMem : Should we free up malloc'ed areas?
     *
     * Returns:
     *	  0 on total success,
     *	  -1 if the module was never initialized, or there was a problem
     *		with the fsprobe connection array.
     */

#endif /* __fsprobe_h */
