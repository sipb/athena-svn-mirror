#define NIDL_GENERATED
#define NIDL_CSWTCH
#include "glb.h"

void glb_$insert
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb_$entry_t *xentry,
  /* [out] */status_$t *status)
#else
 (h, xentry, status)
handle_t h;
lb_$entry_t *xentry;
status_$t *status;
#endif
{
(*glb_$client_epv.glb_$insert)(h, xentry, status);
}

void glb_$delete
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */lb_$entry_t *xentry,
  /* [out] */status_$t *status)
#else
 (h, xentry, status)
handle_t h;
lb_$entry_t *xentry;
status_$t *status;
#endif
{
(*glb_$client_epv.glb_$delete)(h, xentry, status);
}

void glb_$lookup
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
 (h, object, obj_type, obj_interface, entry_handle, max_num_results, num_results, result_entries, status)
handle_t h;
uuid_$t *object;
uuid_$t *obj_type;
uuid_$t *obj_interface;
lb_$lookup_handle_t *entry_handle;
ndr_$ulong_int max_num_results;
ndr_$ulong_int *num_results;
lb_$entry_t result_entries[1];
status_$t *status;
#endif
{
(*glb_$client_epv.glb_$lookup)(h, object, obj_type, obj_interface, entry_handle, max_num_results, num_results, result_entries, status);
}

void glb_$find_server
#ifdef __STDC__
 (
  /* [in] */handle_t h)
#else
 (h)
handle_t h;
#endif
{
(*glb_$client_epv.glb_$find_server)(h);
}
