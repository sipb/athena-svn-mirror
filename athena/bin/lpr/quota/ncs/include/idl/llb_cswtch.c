#define NIDL_GENERATED
#define NIDL_CSWTCH
#include "llb.h"

void llb_$insert
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
(*llb_$client_epv.llb_$insert)(h, xentry, status);
}

void llb_$delete
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
(*llb_$client_epv.llb_$delete)(h, xentry, status);
}

void llb_$lookup
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
 (h, object, obj_type, obj_interface, entry_handle, max_num_results, num_results, result_entries, status)
handle_t h;
uuid_$t *object;
uuid_$t *obj_type;
uuid_$t *obj_interface;
lb_$lookup_handle_t *entry_handle;
ndr_$ulong_int max_num_results;
ndr_$ulong_int *num_results;
lb_$entry_t result_entries[6];
status_$t *status;
#endif
{
(*llb_$client_epv.llb_$lookup)(h, object, obj_type, obj_interface, entry_handle, max_num_results, num_results, result_entries, status);
}
