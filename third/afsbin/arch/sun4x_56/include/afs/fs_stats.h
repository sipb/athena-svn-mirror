/*
 * (C) Copyright Transarc Corporation 1991
 * Licensed Materials - Property of Transarc
 * All Rights Reserved.
 */

#ifndef __fs_stats_h
#define	__fs_stats_h  1

/*------------------------------------------------------------------------
 * fs_stats.h
 *
 * Definitions supporting call and performance information gathering for
 * the FileServer xstat (extended statistics) interface.
 *
 * Author:
 *	Ed. Zayas
 *	Transarc Corporation
 *------------------------------------------------------------------------*/


#include <afs/param.h>		/*System configuration info*/

/*
 * Decide if we're keeping detailed File Server stats.
 */
#define FS_STATS_DETAILED 1

/*
 * Performance numbers.
 */
struct afs_PerfStats {
    int32 numPerfCalls;		/*# of performance calls rcvd*/

    /*
     * Vnode cache section.
     */
    int32 vcache_L_Entries;	/*Num entries in LARGE vnode cache*/
    int32 vcache_L_Allocs;	/*# allocs, large*/
    int32 vcache_L_Gets;		/*# gets, large*/
    int32 vcache_L_Reads;	/*# reads, large*/
    int32 vcache_L_Writes;	/*# writes, large*/

    int32 vcache_S_Entries;	/*Num entries in SMALL vnode cache*/
    int32 vcache_S_Allocs;	/*# allocs, small*/
    int32 vcache_S_Gets;		/*# gets, small*/
    int32 vcache_S_Reads;	/*# reads, small*/
    int32 vcache_S_Writes;	/*# writes, small*/

    int32 vcache_H_Entries;	/*Num entries in HEADER vnode cache */
    int32 vcache_H_Gets;		/*# gets*/
    int32 vcache_H_Replacements;	/*# replacements*/

    /*
     * Directory package section.
     */
    int32 dir_Buffers;		/*Num buffers in use*/
    int32 dir_Calls;		/*Num read calls made*/
    int32 dir_IOs;		/*I/O ops performed*/

    /*
     * Rx section.  These numbers represent the contents of the
     * rx_stats structure maintained by Rx.  All other Rx info,
     * including struct rx_debug and connection stuff is available
     * via the rxdebug interface.
     */
    int32 rx_packetRequests;		/*Packet alloc requests*/
    int32 rx_noPackets_RcvClass;		/*Failed pkt requests, receives*/
    int32 rx_noPackets_SendClass;	/*Ditto, sends*/
    int32 rx_noPackets_SpecialClass;	/*Ditto, specials*/
    int32 rx_socketGreedy;		/*Did SO_GREEDY succeed?*/
    int32 rx_bogusPacketOnRead;		/*Short pkts rcvd*/
    int32 rx_bogusHost;			/*Host addr from bogus pkts*/
    int32 rx_noPacketOnRead;		/*Read pkts w/no packet there*/
    int32 rx_noPacketBuffersOnRead;	/*Pkts dropped from buff shortage*/
    int32 rx_selects;			/*Selects waiting on pkt or timeout*/
    int32 rx_sendSelects;		/*Selects forced upon sends*/
    int32 rx_packetsRead_RcvClass;	/*Packets read, rcv class*/
    int32 rx_packetsRead_SendClass;	/*Packets read, send class*/
    int32 rx_packetsRead_SpecialClass;	/*Packets read, special class*/
    int32 rx_dataPacketsRead;		/*Uniq data packets read off wire*/
    int32 rx_ackPacketsRead;		/*Ack packets read*/
    int32 rx_dupPacketsRead;		/*Duplicate data packets read*/
    int32 rx_spuriousPacketsRead;	/*Inappropriate packets read*/
    int32 rx_packetsSent_RcvClass;	/*Packets sent, rcv class*/
    int32 rx_packetsSent_SendClass;	/*Packets sent, send class*/
    int32 rx_packetsSent_SpecialClass;	/*Packets sent, special class*/
    int32 rx_ackPacketsSent;		/*Ack packets sent*/
    int32 rx_pingPacketsSent;		/*Ping packets sent*/
    int32 rx_abortPacketsSent;		/*Abort packets sent*/
    int32 rx_busyPacketsSent;		/*Busy packets sent*/
    int32 rx_dataPacketsSent;		/*Unique data packets sent*/
    int32 rx_dataPacketsReSent;		/*Retransmissions sent*/
    int32 rx_dataPacketsPushed;		/*Retransmissions pushed by NACK*/
    int32 rx_ignoreAckedPacket;		/*Packets w/acked flag on rxi_Start*/
    int32 rx_totalRtt_Sec;		/*Ttl round trip time, secs*/
    int32 rx_totalRtt_Usec;		/*Ttl round trip time, usecs*/
    int32 rx_minRtt_Sec;			/*Min round trip time, secs*/
    int32 rx_minRtt_Usec;		/*Min round trip time, usecs*/
    int32 rx_maxRtt_Sec;			/*Max round trip time, secs*/
    int32 rx_maxRtt_Usec;		/*Max round trip time, usecs*/
    int32 rx_nRttSamples;		/*Round trip samples*/
    int32 rx_nServerConns;		/*Ttl server connections*/
    int32 rx_nClientConns;		/*Ttl client connections*/
    int32 rx_nPeerStructs;		/*Ttl peer structures*/
    int32 rx_nCallStructs;		/*Ttl call structures*/
    int32 rx_nFreeCallStructs;		/*Ttl free call structures*/

    /*
     * Host module fields.
     */
    int32 host_NumHostEntries;		/*Number of host entries*/
    int32 host_HostBlocks;		/*Blocks in use for hosts*/
    int32 host_NonDeletedHosts;		/*Non-deleted hosts*/
    int32 host_HostsInSameNetOrSubnet;	/*" in same [sub]net as server*/
    int32 host_HostsInDiffSubnet;	/*" in different subnet as server*/
    int32 host_HostsInDiffNetwork;	/*" in different network as server*/
    int32 host_NumClients;		/*Number of client entries*/
    int32 host_ClientBlocks;		/*Blocks in use for clients*/

    /*
     * Host systype
     */
    int32 sysname_ID;			/*Unique hardware/OS identifier*/

    /*
     * Spares
     */
    int32 spare[31];
};

#if FS_STATS_DETAILED
/*
 * Assign each of the File Server's RPC interface routines an index.
 */
#define FS_STATS_RPCIDX_FETCHDATA	 0
#define FS_STATS_RPCIDX_FETCHACL	 1
#define FS_STATS_RPCIDX_FETCHSTATUS	 2
#define FS_STATS_RPCIDX_STOREDATA	 3
#define FS_STATS_RPCIDX_STOREACL	 4
#define FS_STATS_RPCIDX_STORESTATUS	 5
#define FS_STATS_RPCIDX_REMOVEFILE	 6
#define FS_STATS_RPCIDX_CREATEFILE	 7
#define FS_STATS_RPCIDX_RENAME		 8
#define FS_STATS_RPCIDX_SYMLINK		 9
#define FS_STATS_RPCIDX_LINK		10
#define FS_STATS_RPCIDX_MAKEDIR		11
#define FS_STATS_RPCIDX_REMOVEDIR	12
#define FS_STATS_RPCIDX_SETLOCK		13
#define FS_STATS_RPCIDX_EXTENDLOCK	14
#define FS_STATS_RPCIDX_RELEASELOCK	15
#define FS_STATS_RPCIDX_GETSTATISTICS	16
#define FS_STATS_RPCIDX_GIVEUPCALLBACKS	17
#define FS_STATS_RPCIDX_GETVOLUMEINFO	18
#define FS_STATS_RPCIDX_GETVOLUMESTATUS	19
#define FS_STATS_RPCIDX_SETVOLUMESTATUS	20
#define FS_STATS_RPCIDX_GETROOTVOLUME	21
#define FS_STATS_RPCIDX_CHECKTOKEN	22
#define FS_STATS_RPCIDX_GETTIME		23
#define FS_STATS_RPCIDX_NGETVOLUMEINFO	24
#define FS_STATS_RPCIDX_BULKSTATUS	25
#define FS_STATS_RPCIDX_XSTATSVERSION	26
#define FS_STATS_RPCIDX_GETXSTATS	27

#define FS_STATS_NUM_RPC_OPS		28

/*
 * Assign an index to each of the File Server's RPC interface routines
 * that transfer any data to speak of.
 */
#define FS_STATS_XFERIDX_FETCHDATA	 0
#define FS_STATS_XFERIDX_STOREDATA	 1

#define FS_STATS_NUM_XFER_OPS		 2

/*
 * Record to track timing numbers for each File Server RPC operation.
 */
struct fs_stats_opTimingData {
    int32 numOps;			/*Number of operations executed*/
    int32 numSuccesses;			/*Number of successful ops*/
    struct timeval sumTime;		/*Sum of sample timings*/
    struct timeval sqrTime;             /*Sum of squares of sample timings */
    struct timeval minTime;		/*Minimum timing value observed*/
    struct timeval maxTime;		/*Minimum timing value observed*/
};

/*
 * We discriminate byte size transfers into this many buckets.
 */
#define FS_STATS_NUM_XFER_BUCKETS       9

#define FS_STATS_MAXBYTES_BUCKET0     128
#define FS_STATS_MAXBYTES_BUCKET1    1024
#define FS_STATS_MAXBYTES_BUCKET2    8192
#define FS_STATS_MAXBYTES_BUCKET3   16384
#define FS_STATS_MAXBYTES_BUCKET4   32768
#define FS_STATS_MAXBYTES_BUCKET5  131072
#define FS_STATS_MAXBYTES_BUCKET6  524288
#define FS_STATS_MAXBYTES_BUCKET7 1048576


/*
 * Record to track timings and byte sizes for data transfers.
 */
struct fs_stats_xferData {
    int32 numXfers;			   /*Number of xfers*/
    int32 numSuccesses;			   /*Number of successful xfers*/
    struct timeval sumTime;		   /*Sum of timing values*/
    struct timeval sqrTime;                /*Sum of squares of timing values */
    struct timeval minTime;		   /*Minimum xfer time recorded*/
    struct timeval maxTime;		   /*Maximum xfer time recorded*/
    int32 sumBytes;			   /*Sum of bytes transferred*/
    int32 minBytes;			   /*Minimum value observed*/
    int32 maxBytes;			   /*Maximum value observed*/
    int32 count[FS_STATS_NUM_XFER_BUCKETS]; /*Tally for each range of bytes*/
};

/*
 * Macros to operate on time values.
 *
 * fs_stats_TimeLessThan(t1, t2)     Non-zero if t1 is less than t2
 * fs_stats_TimeGreaterThan(t1, t2)  Non-zero if t1 is greater than t2
 * fs_stats_GetDiff(t3, t1, t2)      Set t3 to the difference between
 *					t1 and t2 (t1 is less than or
 *					equal to t2).
 * fs_stats_AddTo(t1, t2)            Add t2 to t1
 * fs_stats_TimeAssign(t1, t2)	     Assign time t2 to t1
 * afs_stats_SquareAddTo(t1,t2)      Add square of t2 to t1
 */
#define fs_stats_TimeLessThan(t1, t2)        \
            ((t1.tv_sec  < t2.tv_sec)  ? 1 : \
	     (t1.tv_sec  > t2.tv_sec)  ? 0 : \
	     (t1.tv_usec < t2.tv_usec) ? 1 : \
	     0)

#define fs_stats_TimeGreaterThan(t1, t2)     \
            ((t1.tv_sec  > t2.tv_sec)  ? 1 : \
	     (t1.tv_sec  < t2.tv_sec)  ? 0 : \
	     (t1.tv_usec > t2.tv_usec) ? 1 : \
	     0)

#define fs_stats_GetDiff(t3, t1, t2)				\
{								\
    /*								\
     * If the microseconds of the later time are smaller than	\
     * the earlier time, set up for proper subtraction (doing	\
     * the carry).						\
     */								\
    if (t2.tv_usec < t1.tv_usec) {				\
	t2.tv_usec += 1000000;					\
	t2.tv_sec -= 1;						\
    }								\
    t3.tv_sec  = t2.tv_sec  - t1.tv_sec;			\
    t3.tv_usec = t2.tv_usec - t1.tv_usec;			\
}

#define fs_stats_AddTo(t1, t2)    \
{                                 \
    t1.tv_sec  += t2.tv_sec;      \
    t1.tv_usec += t2.tv_usec;     \
    if (t1.tv_usec > 1000000) {   \
	t1.tv_usec -= 1000000;    \
	t1.tv_sec++;              \
    }                             \
}

#define fs_stats_TimeAssign(t1, t2)	\
{					\
    t1.tv_sec = t2.tv_sec;		\
    t1.tv_usec = t2.tv_usec;		\
}

#define fs_stats_SquareAddTo(t1, t2)                         \
{                                                            \
    if (t2.tv_sec > 0)                                       \
      {                                                      \
       t1.tv_sec += t2.tv_sec * t2.tv_sec                    \
                    + (0.000002 * t2.tv_sec) * t2.tv_usec ;  \
       t1.tv_usec += (2 * t2.tv_sec * t2.tv_usec) % 1000000  \
                    + (0.000001 * t2.tv_usec) * t2.tv_usec;  \
      }                                                      \
    else                                                     \
      {                                                      \
       t1.tv_usec += (0.000001 * t2.tv_usec) * t2.tv_usec;   \
      }                                                      \
    if (t1.tv_usec > 1000000) {                              \
        t1.tv_usec -= 1000000;                               \
        t1.tv_sec++;                                         \
    }                                                        \
}

/*
 * This is the detailed performance data collection for the File Server.
 */
struct fs_stats_DetailedStats {
    struct timeval epoch;		    /*Time when data collection began*/
    struct fs_stats_opTimingData
	rpcOpTimes[FS_STATS_NUM_RPC_OPS];   /*Individual RPC operation timings*/
    struct fs_stats_xferData
	xferOpTimes[FS_STATS_NUM_XFER_OPS]; /*Byte info for certain ops*/
};

/*
 * This is all of the performance data, both overall and detailed.
 */
struct fs_stats_FullPerfStats {
    struct afs_PerfStats overall;
    struct fs_stats_DetailedStats det;
};

/*
 * This is the structure accessible by specifying the
 * AFS_XSTATSCOLL_FULL_PERF_INFO collection to the xstat package.
 */
extern struct fs_stats_FullPerfStats afs_FullPerfStats;
#endif /* FS_STATS_DETAILED */


/*
 * This is the structure accessible by specifying the
 * AFS_XSTATSCOLL_PERF_INFO collection to the xstat package.
 */
extern struct afs_PerfStats afs_perfstats;

/*
  * FileServer's name and IP address, both network byte order and
  * host byte order.
  */
extern char FS_HostName[];
extern u_int32 FS_HostAddr_NBO;
extern u_int32 FS_HostAddr_HBO;

#endif /* __fs_stats_h */
