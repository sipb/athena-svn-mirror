/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/rx/RCS/rx_globals.h,v 2.77 1996/04/24 17:02:36 zumach Exp $ */
/* $Source: /afs/transarc.com/project/fs/dev/afs/rcs/rx/RCS/rx_globals.h,v $ */

/*
****************************************************************************
*        Copyright IBM Corporation 1988, 1989 - All Rights Reserved        *
*                                                                          *
* Permission to use, copy, modify, and distribute this software and its    *
* documentation for any purpose and without fee is hereby granted,         *
* provided that the above copyright notice appear in all copies and        *
* that both that copyright notice and this permission notice appear in     *
* supporting documentation, and that the name of IBM not be used in        *
* advertising or publicity pertaining to distribution of the software      *
* without specific, written prior permission.                              *
*                                                                          *
* IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL IBM *
* BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY      *
* DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER  *
* IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING   *
* OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.    *
****************************************************************************
*/

/* RX:  Globals for internal use, basically */

#ifdef	KERNEL
#include "../rx/rx.h"
#else /* KERNEL */
# include "rx.h"
#endif /* KERNEL */

#ifndef INIT
#define INIT(x)
#define	EXT extern
#endif /* INIT */

/* The array of installed services.  Null terminated. */
EXT struct rx_service *rx_services[RX_MAX_SERVICES+1];
#ifdef RX_ENABLE_LOCKS
/* Protects nRequestsRunning as well as pool allocation variables. */
EXT kmutex_t rx_serverPool_lock;
#endif /* RX_ENABLE_LOCKS */

/* Incoming calls wait on this queue when there are no available server processes */
EXT struct rx_queue rx_incomingCallQueue;

/* Server processes wait on this queue when there are no appropriate calls to process */
EXT struct rx_queue rx_idleServerQueue;

/* Constant delay time before sending an acknowledge of the last packet received.  This is to avoid sending an extra acknowledge when the client is about to make another call, anyway, or the server is about to respond. */
EXT struct clock rx_lastAckDelay;

/* Variable to allow introduction of network unreliability */
#ifdef RXDEBUG
EXT int rx_intentionallyDroppedPacketsPer100 INIT(0);	/* Dropped on Send */
#endif

EXT int rx_extraQuota INIT(0);		/* extra packets to add to the quota */
EXT int rx_extraPackets INIT(32);	/* extra packets to alloc (2 windows by deflt) */

EXT int rx_stackSize INIT(RX_DEFAULT_STACK_SIZE);

EXT int	rx_connDeadTime	INIT(12);	    /* Time until an unresponsive connection is declared dead */
EXT int rx_idleConnectionTime INIT(700);    /* Time until we toss an idle connection */
EXT int	rx_idlePeerTime	INIT(60);	    /* Time until we toss a peer structure, after all connections using it have disappeared */
EXT int	rx_tranquil	INIT(0);    	/* The file server is temporarily salvaging : dhruba */

/* These definitions should be in one place */
#ifdef	AFS_SUN5_ENV
#define	RX_CBUF_TIME	180	/* Check for cbuf deficit */
#define	RX_REAP_TIME	90	    /* Check for tossable connections every 90 seconds */
#else
#define	RX_CBUF_TIME	120	/* Check for cbuf deficit */
#define	RX_REAP_TIME	60	    /* Check for tossable connections every 60 seconds */
#endif

#define RX_STANDARD_ACK_RATE 3  /* old */
#define RX_FAST_ACK_RATE 1      /* as of 3.4, ask for an ack every 
				   other packet */

EXT int rx_minWindow INIT(1); 
EXT int rx_initReceiveWindow INIT(16);  /* how much to accept */
EXT int rx_maxReceiveWindow INIT(16);  /* how much to accept */
EXT int rx_initSendWindow INIT(8); 
EXT int rx_maxSendWindow INIT(16); 
#define RX_MAX_FRAGS 4
EXT int rxi_nSendFrags INIT(RX_MAX_FRAGS);  /* max fragments in a datagram */
EXT int rxi_nRecvFrags INIT(RX_MAX_FRAGS);
EXT int rxi_OrphanFragSize INIT(512);
/* allow n packets between soft acks - must be power of 2 -1, else change
 * macro below */
EXT int rxi_SoftAckRate INIT(RX_FAST_ACK_RATE);  
/* consume n packets before sending hard ack, should be larger than above,
   but not absolutely necessary.  If it's smaller, than fast receivers will
   send a soft ack, immediately followed by a hard ack. */
EXT int rxi_HardAckRate INIT(RX_FAST_ACK_RATE+1);

/* EXT int rx_maxWindow INIT(15);   Temporary HACK:  transmit/receive window */

/* LWSXXX if window sizes become very variable (in terms of #packets), be
 * sure that the sender can get back a hard acks without having to wait for
 * some kind of timer event first (like a keep-alive, for instance).
 * It might be kind of tricky, so it might be better to shrink the
 * window size by reducing the packet size below the "natural" MTU. */

#define	ACKHACK(p,r) { if (((p)->header.seq & (rxi_SoftAckRate))==0) (p)->header.flags |= RX_REQUEST_ACK; } 

EXT int rx_nPackets INIT(100);	/* obsolete; use rx_extraPackets now */

/* List of free packets */
EXT struct rx_queue rx_freePacketQueue;
EXT struct rx_queue rx_freeCbufQueue;
#ifdef RX_ENABLE_LOCKS
EXT kmutex_t rx_freePktQ_lock;
#endif

/* Number of free packets */
EXT int rx_nFreePackets INIT(0);
EXT int rx_nFreeCbufs INIT(0);
EXT int rx_nCbufs INIT(0);
EXT int rxi_NeedMoreCbufs INIT(0);
EXT int rx_nWaiting INIT(0);

/* largest packet which we can safely receive, initialized to AFS 3.2 value
 * This is provided for backward compatibility with peers which may be unable
 * to swallow anything larger. THIS MUST NEVER DECREASE WHILE AN APPLICATION
 * IS RUNNING! */
EXT u_int32 rx_maxReceiveSize INIT(OLD_MAX_PACKET_SIZE*RX_MAX_FRAGS + UDP_HDR_SIZE*(RX_MAX_FRAGS-1));

#if (defined(AFS_SUN5_ENV) || defined(AFS_AOS_ENV)) && defined(KERNEL)
EXT u_int32 rx_MyMaxSendSize INIT(OLD_MAX_PACKET_SIZE - RX_HEADER_SIZE);
#else
EXT u_int32 rx_MyMaxSendSize INIT(8192);
#endif
/* need this to permit progs to run on AIX systems */
EXT int32 (*rxi_syscallp) () INIT(0); 

/* List of free queue entries */
EXT struct rx_serverQueueEntry *rx_FreeSQEList INIT(0);
#ifdef	RX_ENABLE_LOCKS
EXT kmutex_t freeSQEList_lock;
#endif

/* List of free call structures */
EXT struct rx_queue rx_freeCallQueue;
#ifdef	RX_ENABLE_LOCKS
EXT kmutex_t rx_freeCallQueue_lock;
#endif
EXT int32 rxi_nCalls INIT(0);

/* Basic socket for client requests; other sockets (for receiving server requests) are in the service structures */
EXT osi_socket rx_socket;

/* Port requested at rx_Init.  If this is zero, the actual port used will be different--but it will only be used for client operations.  If non-zero, server provided services may use the same port. */
EXT u_short rx_port;

/* 32-bit select Mask for rx_Listener.  We use 32 bits because IOMGR_Select only supports 32 */
EXT u_int32 rx_selectMask;
EXT int rx_maxSocketNumber;	    /* Maximum socket number represented in the select mask */

/* This is actually the minimum number of packets that must remain free,
    overall, immediately after a packet of the requested class has been
    allocated.  *WARNING* These must be assigned with a great deal of care.
    In order, these are receive quota, send quota and special quota */
#define	RX_PACKET_QUOTAS {1, 10, 0}
/* value large enough to guarantee that no allocation fails due to RX_PACKET_QUOTAS.
   Make it a little bigger, just for fun */
#define	RX_MAX_QUOTA	15	/* part of min packet computation */
EXT int rx_packetQuota[RX_N_PACKET_CLASSES] INIT(RX_PACKET_QUOTAS);

EXT int rx_nextCid;	    /* Next connection call id */
EXT int rx_epoch;	    /* Initialization time of rx */
#ifdef	RX_ENABLE_LOCKS
EXT kcondvar_t rx_waitingForPackets_cv;
#endif
EXT char rx_waitingForPackets; /* Processes set and wait on this variable when waiting for packet buffers */

EXT struct rx_stats rx_stats;

EXT struct rx_peer **rx_peerHashTable;
EXT struct rx_connection **rx_connHashTable;
EXT u_int32 rx_hashTableSize INIT(256);	/* Power of 2 */
EXT u_int32 rx_hashTableMask INIT(255);	/* One less than rx_hashTableSize */
#ifdef RX_ENABLE_LOCKS
EXT kmutex_t rx_peerHashTable_lock;
EXT kmutex_t rx_connHashTable_lock;
#endif /* RX_ENABLE_LOCKS */

#define CONN_HASH(host, port, cid, epoch, type) ((((cid)>>RX_CIDSHIFT)&rx_hashTableMask))

#define PEER_HASH(host, port)  ((host ^ port) & rx_hashTableMask)

#ifdef	notdef /* Use a func for now to measure allocated structs */
#define	rxi_Free(addr, size)	osi_Free(addr, size)
#endif /* notdef */


/* Forward definitions of internal procedures */
struct rx_packet *rxi_AllocPacket();
struct rx_packet *rxi_AllocSendPacket();
char *rxi_Alloc();
struct rx_peer *rxi_FindPeer();
struct rx_call *rxi_NewCall();
void rxi_FreeCall();
struct rx_call *rxi_FindCall();
void rxi_Listener();
int rxi_ReadPacket();
struct rx_packet *rxi_ReceivePacket();
struct rx_packet *rxi_ReceiveDataPacket();
struct rx_packet *rxi_ReceiveAckPacket();
struct rx_packet *rxi_ReceiveResponsePacket();
struct rx_packet *rxi_ReceiveChallengePacket();
void rx_ServerProc();
void rxi_AttachServerProc();
void rxi_ChallengeOn();
#define	rxi_ChallengeOff(conn)	rxevent_Cancel((conn)->challengeEvent, (struct rx_call*)0, 0);
void rxi_ChallengeEvent();
struct rx_packet *rxi_SendAck();
void rxi_ClearTransmitQueue();
void rxi_ClearReceiveQueue();
void rxi_InitCall();
void rxi_ResetCall();
void rxi_CallError();
void rxi_ConnectionError();
void rxi_QueuePackets();
void rxi_Start();
void rxi_CallIsIdle();
void rxi_CallTimedOut();
void rxi_ComputeRoundTripTime();
void rxi_ScheduleKeepAliveEvent();
void rxi_KeepAliveEvent();
void rxi_KeepAliveOn();
#define rxi_KeepAliveOff(call) rxevent_Cancel((call)->keepAliveEvent, call, RX_CALL_REFCOUNT_ALIVE)
void rxi_AckAll();
void rxi_SendDelayedAck();
struct rx_packet *rxi_SendSpecial();
struct rx_packet *rxi_SendCallAbort();
struct rx_packet *rxi_SendConnectionAbort();
void rxi_ScheduleDecongestionEvent();
void rxi_CongestionWait();
void rxi_ReapConnections();
void rxi_EncodePacketHeader();
void rxi_DecodePacketHeader();
void rxi_DebugPrint();
void rxi_SendDelayedAck();
void rxi_PrepareSendPacket();
void rxi_MoreCbufs();


#define rxi_AllocSecurityObject() (struct rx_securityClass *) rxi_Alloc(sizeof(struct rx_securityClass))
#define	rxi_FreeSecurityObject(obj) rxi_Free(obj, sizeof(struct rx_securityClass))
#define	rxi_AllocService()	(struct rx_service *) rxi_Alloc(sizeof(struct rx_service))
#define	rxi_FreeService(obj)	rxi_Free(obj, sizeof(struct rx_service))
#define	rxi_AllocPeer()		(struct rx_peer *) rxi_Alloc(sizeof(struct rx_peer))
#define	rxi_FreePeer(peer)	rxi_Free(peer, sizeof(struct rx_peer))
#define	rxi_AllocConnection()	(struct rx_connection *) rxi_Alloc(sizeof(struct rx_connection))
#define rxi_FreeConnection(conn) (rxi_Free(conn, sizeof(struct rx_connection)))

#ifdef RXDEBUG
/* Some debugging stuff */
EXT FILE *rx_debugFile;	/* Set by the user to a stdio file for debugging output */

#define Log rx_debugFile
#define dpf(args) if (rx_debugFile) rxi_DebugPrint args; else

EXT char *rx_packetTypes[RX_N_PACKET_TYPES] INIT(RX_PACKET_TYPES); /* Strings defined in rx.h */

#else
#define dpf(args) 
#endif /* RXDEBUG */
