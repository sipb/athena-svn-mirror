#ifndef TRANSARC_AFS_CLIENT_ADMIN_H
#define TRANSARC_AFS_CLIENT_ADMIN_H

/*
 * Copyright (C)  1998  Transarc Corporation.  All rights reserved.
 *
 * $Header: /afs/dev.mit.edu/source/repository/third/afsbin/arch/sun4x_56/include/afs/afs_clientAdmin.h,v 1.1.1.1 1999-12-21 04:06:15 ghudson Exp $
 */

#include <afs/afs_Admin.h>

#ifdef DELETE
#undef DELETE
#endif

typedef enum {READ_ONLY, READ_WRITE} vol_type_t, *vol_type_p;
typedef enum {CHECK_VOLUME, DONT_CHECK_VOLUME} vol_check_t, *vol_check_p;

typedef enum {READ, NO_READ} acl_read_t, *acl_read_p;
typedef enum {WRITE, NO_WRITE} acl_write_t, *acl_write_p;
typedef enum {LOOKUP, NO_LOOKUP} acl_lookup_t, *acl_lookup_p;
typedef enum {DELETE, NO_DELETE} acl_delete_t, *acl_delete_p;
typedef enum {INSERT, NO_INSERT} acl_insert_t, *acl_insert_p;
typedef enum {LOCK, NO_LOCK} acl_lock_t, *acl_lock_p;
typedef enum {ADMIN, NO_ADMIN} acl_admin_t, *acl_admin_p;

typedef struct acl {
  acl_read_t read;
  acl_write_t write;
  acl_lookup_t lookup;
  acl_delete_t del;
  acl_insert_t insert;
  acl_lock_t lock;
  acl_admin_t admin;
} acl_t, *acl_p;

#define AFS_MAX_SERVER_NAME_LEN   128
#define AFS_MAX_SERVER_ADDRESS     16

typedef enum {
  DATABASE_SERVER = 0x1,
  FILE_SERVER = 0x2
} afs_server_type_t, *afs_server_type_p;

typedef enum {
  AFS_BOSSERVER,
  AFS_FILESERVER,
  AFS_KASERVER,
  AFS_PTSERVER,
  AFS_VOLSERVER,
  AFS_VLSERVER,
  AFS_CLIENT
} afs_stat_source_t, *afs_stat_source_p;

typedef struct {
  char serverName[ AFS_MAX_SERVER_NAME_LEN ];
  afs_server_type_t serverType;
  int serverAddress[ AFS_MAX_SERVER_ADDRESS ];
} afs_serverEntry_t, *afs_serverEntry_p;

extern int ADMINAPI afsclient_TokenGetExisting(
  const char *cellName,
  void **tokenHandle,
  afs_status_p st
);

extern int ADMINAPI afsclient_TokenSet(
  const void *tokenHandle,
  afs_status_p st
);

extern int ADMINAPI afsclient_TokenGetNew(
  const char *cellName,
  const char *principal,
  const char *password,
  void **tokenHandle,
  afs_status_p st
);

extern int ADMINAPI afsclient_TokenClose(
  const void *tokenHandle,
  afs_status_p st
);

extern int ADMINAPI afsclient_TokenQuery(
  void *tokenHandle,
  unsigned long *expirationDateP,
  char *principal,
  char *instance,
  char *cell,
  int *hasKasTokens,
  afs_status_p st
);

extern int ADMINAPI afsclient_CellOpen(
  const char *cellName,
  const void *tokenHandle,
  void **cellHandleP,
  afs_status_p st
);

extern int ADMINAPI afsclient_NullCellOpen(
  void **cellHandleP,
  afs_status_p st
);
 
extern int ADMINAPI afsclient_CellClose(
  const void *cellHandle,
  afs_status_p st
);

extern int ADMINAPI afsclient_CellNameGet(
  const void *cellHandle,
  const char **cellNameP,
  afs_status_p st
);
 
extern int ADMINAPI afsclient_LocalCellGet(
  char *cellName,
  afs_status_p st
);

extern int ADMINAPI afsclient_MountPointCreate(
  const void *cellHandle,
  const char *directory,
  const char *volumeName,
  vol_type_t volType,
  vol_check_t volCheck,
  afs_status_p st
);

extern int ADMINAPI afsclient_ACLEntryAdd(
  const char *directory,
  const char *user,
  const acl_p acl,
  afs_status_p st
);

extern int ADMINAPI afsclient_Init(
  afs_status_p st
);

extern int ADMINAPI afsclient_AFSServerGet(
  const void *cellHandle,
  const char *serverName,
  afs_serverEntry_p serverEntryP,
  afs_status_p st
);
 
extern int ADMINAPI afsclient_AFSServerGetBegin(
  const void *cellHandle,
  void **iterationIdP,
  afs_status_p st
);
 
extern int ADMINAPI afsclient_AFSServerGetNext(
  void *iterationId,
  afs_serverEntry_p serverEntryP,
  afs_status_p st
);
 
extern int ADMINAPI afsclient_AFSServerGetDone(
  void *iterationId,
  afs_status_p st
);

extern int ADMINAPI afsclient_RPCStatOpen(
  const void *cellHandle,
  const char *serverName,
  afs_stat_source_t type,
  struct rx_connection **rpcStatHandleP,
  afs_status_p st
);

extern int ADMINAPI afsclient_RPCStatClose(
  struct rx_connection *rpcStatHandle,
  afs_status_p st
);

#endif /* TRANSARC_AFS_CLIENT_ADMIN_H */