#ifndef TRANSARC_AFS_ADMIN_H
#define TRANSARC_AFS_ADMIN_H

/*
 * Copyright (C)  1998  Transarc Corporation.  All rights reserved.
 *
 * $Header: /afs/dev.mit.edu/source/repository/third/afsbin/arch/i386_linux22/include/afs/afs_Admin.h,v 1.1.1.1 1999-12-22 20:45:34 ghudson Exp $
 */

#include <afs/param.h>
#include <rx/rx.h>

#ifdef AFS_NT40_ENV
/* NT definitions */
#define ADMINAPI __cdecl

#ifndef ADMINEXPORT
#define ADMINEXPORT  __declspec(dllimport)
#endif

#else
/* Unix definitions */
#define ADMINAPI
#define ADMINEXPORT
#endif /* AFS_NT40_ENV */


typedef unsigned int afs_status_t, *afs_status_p;

typedef enum {
  AFS_RPC_STATS_DISABLED,
  AFS_RPC_STATS_ENABLED
} afs_RPCStatsState_t, *afs_RPCStatsState_p;

typedef u_int32 afs_RPCStatsVersion_t, *afs_RPCStatsVersion_p;

/*
 * The following is not an enum because AIX forces enum's to be
 * unsigned ints and rejects the all flag
 */

#define AFS_RX_STATS_CLEAR_ALL 			 	0xffffffff
#define AFS_RX_STATS_CLEAR_INVOCATIONS 			0x1
#define AFS_RX_STATS_CLEAR_TIME_SUM 			0x2
#define AFS_RX_STATS_CLEAR_TIME_SQUARE 			0x4
#define AFS_RX_STATS_CLEAR_TIME_MIN 			0x8
#define AFS_RX_STATS_CLEAR_TIME_MAX 			0x10

typedef u_int32 afs_RPCStatsClearFlag_t, *afs_RPCStatsClearFlag_p;

typedef struct afs_RPCStats {
  u_int32 clientVersion;
  u_int32 serverVersion;
  u_int32 statCount;
  union {
      rx_function_entry_v1_t stats_v1;
      /* add new stat structures here when required */
  } s;
} afs_RPCStats_t, *afs_RPCStats_p;

#define AFS_STATUS_OK 0

#endif /* TRANSARC_AFS_ADMIN_H */
