#ifndef lb__included
#define lb__included
#include "idl_base.h"
#include "nbase.h"
#include "glb.h"
extern  void lb_$register
#ifdef __STDC__
 (
  /* [in] */uuid_$t *object,
  /* [in] */uuid_$t *obj_type,
  /* [in] */uuid_$t *obj_interface,
  /* [in] */lb_$server_flag_t flags,
  /* [in] */ndr_$char annotation[64],
  /* [in] */socket_$addr_t *saddr,
  /* [in] */ndr_$ulong_int saddr_len,
  /* [out] */lb_$entry_t *xentry,
  /* [out] */status_$t *status);
#else
 ( );
#endif
extern  void lb_$unregister
#ifdef __STDC__
 (
  /* [in] */lb_$entry_t *xentry,
  /* [out] */status_$t *status);
#else
 ( );
#endif
extern  void lb_$lookup_range
#ifdef __STDC__
 (
  /* [in] */uuid_$t *object,
  /* [in] */uuid_$t *obj_type,
  /* [in] */uuid_$t *obj_interface,
  /* [in] */socket_$addr_t *location,
  /* [in] */ndr_$ulong_int location_len,
  /* [in, out] */lb_$lookup_handle_t *entry_handle,
  /* [in] */ndr_$ulong_int max_num_results,
  /* [out] */ndr_$ulong_int *num_results,
  /* [out] */lb_$entry_t result_entries[1],
  /* [out] */status_$t *status);
#else
 ( );
#endif
extern  void lb_$lookup_object
#ifdef __STDC__
 (
  /* [in] */uuid_$t *object,
  /* [in, out] */lb_$lookup_handle_t *entry_handle,
  /* [in] */ndr_$ulong_int max_num_results,
  /* [out] */ndr_$ulong_int *num_results,
  /* [out] */lb_$entry_t result_entries[1],
  /* [out] */status_$t *status);
#else
 ( );
#endif
extern  void lb_$lookup_object_local
#ifdef __STDC__
 (
  /* [in] */uuid_$t *object,
  /* [in] */socket_$addr_t *location,
  /* [in] */ndr_$ulong_int location_len,
  /* [in, out] */lb_$lookup_handle_t *entry_handle,
  /* [in] */ndr_$ulong_int max_num_results,
  /* [out] */ndr_$ulong_int *num_results,
  /* [out] */lb_$entry_t result_entries[1],
  /* [out] */status_$t *status);
#else
 ( );
#endif
extern  void lb_$lookup_type
#ifdef __STDC__
 (
  /* [in] */uuid_$t *obj_type,
  /* [in, out] */lb_$lookup_handle_t *entry_handle,
  /* [in] */ndr_$ulong_int max_num_results,
  /* [out] */ndr_$ulong_int *num_results,
  /* [out] */lb_$entry_t result_entries[1],
  /* [out] */status_$t *status);
#else
 ( );
#endif
extern  void lb_$lookup_interface
#ifdef __STDC__
 (
  /* [in] */uuid_$t *obj_interface,
  /* [in, out] */lb_$lookup_handle_t *entry_handle,
  /* [in] */ndr_$ulong_int max_num_results,
  /* [out] */ndr_$ulong_int *num_results,
  /* [out] */lb_$entry_t result_entries[1],
  /* [out] */status_$t *status);
#else
 ( );
#endif
#endif
