#ifndef TRANSARC_AFS_UTIL_ADMIN_H
#define TRANSARC_AFS_UTIL_ADMIN_H

/*
 * Copyright (C)  1998  Transarc Corporation.  All rights reserved.
 *
 * $Header: /afs/dev.mit.edu/source/repository/third/afsbin/arch/sgi_65/include/afs/afs_utilAdmin.h,v 1.1.1.1 1999-12-22 20:05:41 ghudson Exp $
 */

#include <afs/afs_Admin.h>
#include <afs/afs_AdminErrors.h>

#define UTIL_MAX_DATABASE_SERVER_NAME 64

typedef struct util_databaseServerEntry {
    int serverAddress;
    char serverName[ UTIL_MAX_DATABASE_SERVER_NAME ];
} util_databaseServerEntry_t, *util_databaseServerEntry_p;

extern int ADMINAPI util_AdminErrorCodeTranslate(
   afs_status_t errorCode,
   int langId,
   const char **errorTextP,
   afs_status_p st
);

extern int ADMINAPI util_DatabaseServerGetBegin(
  const char *cellName,
  void **iterationIdP,
  afs_status_p st
);

extern int ADMINAPI util_DatabaseServerGetNext(
  const void *iterationId,
  util_databaseServerEntry_p serverP,
  afs_status_p st
);

extern int ADMINAPI util_DatabaseServerGetDone(
  const void *iterationId,
  afs_status_p st
);

extern int ADMINAPI util_AdminServerAddressGetFromName(
  const char *serverName,
  int *serverAddress,
  afs_status_p st
);

extern int ADMINAPI CellHandleIsValid(
  const void *cellHandle,
  afs_status_p st
);

extern int ADMINAPI util_RPCStatsGetBegin(
  struct rx_connection *conn,
  int (*rpc)(),
  void **iterationIdP,
  afs_status_p st
);

extern int ADMINAPI util_RPCStatsGetNext(
  const void *iterationId,
  afs_RPCStats_p stats,
  afs_status_p st
);

extern int ADMINAPI util_RPCStatsGetDone(
  const void *iterationId,
  afs_status_p st
);

extern int ADMINAPI util_RPCStatsStateGet(
  struct rx_connection *conn,
  int (*rpc)(),
  afs_RPCStatsState_p state,
  afs_status_p st
);

extern int ADMINAPI util_RPCStatsStateEnable(
  struct rx_connection *conn,
  int (*rpc)(),
  afs_status_p st
);

extern int ADMINAPI util_RPCStatsStateDisable(
  struct rx_connection *conn,
  int (*rpc)(),
  afs_status_p st
);

extern int ADMINAPI util_RPCStatsClear(
  struct rx_connection *conn,
  int (*rpc)(),
  afs_RPCStatsClearFlag_t flag,
  afs_status_p st
);

extern int ADMINAPI util_RPCStatsVersionGet(
  struct rx_connection *conn,
  afs_RPCStatsVersion_p version,
  afs_status_p st
);

#endif /* TRANSARC_AFS_UTIL_ADMIN_H */
