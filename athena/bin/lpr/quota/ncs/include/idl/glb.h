#ifndef glb__included
#define glb__included
#include "idl_base.h"
#include "rpc.h"
#include "nbase.h"
static rpc_$if_spec_t glb_$if_spec = {
  4,
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  4,
  {
  0x333b2e69,
  0x0000,
  0,
  0xd,
  {0x0, 0x0, 0x87, 0x84, 0x0, 0x0, 0x0}
  }
};
#define lb_$mod 469893120
#define lb_$database_invalid 469893121
#define lb_$database_busy 469893122
#define lb_$not_registered 469893123
#define lb_$update_failed 469893124
#define lb_$cant_access 469893125
#define lb_$server_unavailable 469893126
#define lb_$bad_entry 469893127
typedef ndr_$ulong_int lb_$server_flag_t;
#define lb_$server_flag_local 1
#define lb_$server_flag_reserved_02 2
#define lb_$server_flag_reserved_04 4
#define lb_$server_flag_reserved_08 8
#define lb_$server_flag_reserved_10 16
#define lb_$server_flag_reserved_20 32
#define lb_$server_flag_reserved_40 64
#define lb_$server_flag_reserved_80 128
#define lb_$server_flag_reserved_0100 256
#define lb_$server_flag_reserved_0200 512
#define lb_$server_flag_reserved_0400 1024
#define lb_$server_flag_reserved_0800 2048
#define lb_$server_flag_reserved_1000 4096
#define lb_$server_flag_reserved_2000 8192
#define lb_$server_flag_reserved_4000 16384
#define lb_$server_flag_reserved_8000 32768
#define lb_$server_flag_reserved_10000 65536
#define lb_$server_flag_reserved_20000 131072
#define lb_$server_flag_reserved_40000 262144
#define lb_$server_flag_reserved_80000 524288
typedef ndr_$ulong_int lb_$lookup_handle_t;
#define lb_$default_lookup_handle 0
typedef struct lb_$entry_t lb_$entry_t;
struct lb_$entry_t {
uuid_$t object;
uuid_$t obj_type;
uuid_$t obj_interface;
lb_$server_flag_t flags;
ndr_$char annotation[64];
ndr_$ulong_int saddr_len;
socket_$addr_t saddr;
};
extern  void glb_$insert
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb_$entry_t *xentry,
  /* [out] */status_$t *status);
#else
 ( );
#endif
extern  void glb_$delete
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb_$entry_t *xentry,
  /* [out] */status_$t *status);
#else
 ( );
#endif
#define glb_$max_lookup_results 400
extern  void glb_$lookup
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */uuid_$t *object,
  /* [in] */uuid_$t *obj_type,
  /* [in] */uuid_$t *obj_interface,
  /* [in, out] */lb_$lookup_handle_t *entry_handle,
  /* [in] */ndr_$ulong_int max_num_results,
  /* [out] */ndr_$ulong_int *num_results,
  /* [out] */lb_$entry_t result_entries[1],
  /* [out] */status_$t *status);
#else
 ( );
#endif
extern  void glb_$find_server
#ifdef __STDC__
 (
  /* [in] */handle_t h);
#else
 ( );
#endif
typedef struct glb_$epv_t {
void (*glb_$insert)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb_$entry_t *xentry,
  /* [out] */status_$t *status)
#else
()
#endif
;
void (*glb_$delete)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb_$entry_t *xentry,
  /* [out] */status_$t *status)
#else
()
#endif
;
void (*glb_$lookup)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */uuid_$t *object,
  /* [in] */uuid_$t *obj_type,
  /* [in] */uuid_$t *obj_interface,
  /* [in, out] */lb_$lookup_handle_t *entry_handle,
  /* [in] */ndr_$ulong_int max_num_results,
  /* [out] */ndr_$ulong_int *num_results,
  /* [out] */lb_$entry_t result_entries[1],
  /* [out] */status_$t *status)
#else
()
#endif
;
void (*glb_$find_server)
#ifdef __STDC__
 (
  /* [in] */handle_t h)
#else
()
#endif
;
} glb_$epv_t;
globalref glb_$epv_t glb_$client_epv;
globalref glb_$epv_t glb_$manager_epv;
globalref rpc_$epv_t glb_$server_epv;
#endif
