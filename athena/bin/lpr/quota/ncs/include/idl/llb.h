#ifndef llb__included
#define llb__included
#include "idl_base.h"
#include "rpc.h"
#include "nbase.h"
#include "glb.h"
static rpc_$if_spec_t llb_$if_spec = {
  4,
  {0, 0, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  3,
  {
  0x333b33c3,
  0x0000,
  0,
  0xd,
  {0x0, 0x0, 0x87, 0x84, 0x0, 0x0, 0x0}
  }
};
extern  void llb_$insert
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb_$entry_t *xentry,
  /* [out] */status_$t *status);
#else
 ( );
#endif
extern  void llb_$delete
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb_$entry_t *xentry,
  /* [out] */status_$t *status);
#else
 ( );
#endif
#define llb_$max_lookup_results 6
extern  void llb_$lookup
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */uuid_$t *object,
  /* [in] */uuid_$t *obj_type,
  /* [in] */uuid_$t *obj_interface,
  /* [in, out] */lb_$lookup_handle_t *entry_handle,
  /* [in] */ndr_$ulong_int max_num_results,
  /* [out] */ndr_$ulong_int *num_results,
  /* [out] */lb_$entry_t result_entries[6],
  /* [out] */status_$t *status);
#else
 ( );
#endif
typedef struct llb_$epv_t {
void (*llb_$insert)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb_$entry_t *xentry,
  /* [out] */status_$t *status)
#else
()
#endif
;
void (*llb_$delete)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb_$entry_t *xentry,
  /* [out] */status_$t *status)
#else
()
#endif
;
void (*llb_$lookup)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */uuid_$t *object,
  /* [in] */uuid_$t *obj_type,
  /* [in] */uuid_$t *obj_interface,
  /* [in, out] */lb_$lookup_handle_t *entry_handle,
  /* [in] */ndr_$ulong_int max_num_results,
  /* [out] */ndr_$ulong_int *num_results,
  /* [out] */lb_$entry_t result_entries[6],
  /* [out] */status_$t *status)
#else
()
#endif
;
} llb_$epv_t;
globalref llb_$epv_t llb_$client_epv;
globalref llb_$epv_t llb_$manager_epv;
globalref rpc_$epv_t llb_$server_epv;
#endif
