#ifndef conv__included
#define conv__included
#include "idl_base.h"
#include "rpc.h"
#include "nbase.h"
static rpc_$if_spec_t conv_$if_spec = {
  3,
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  1,
  {
  0x333a2276,
  0x0000,
  0,
  0xd,
  {0x0, 0x0, 0x80, 0x9c, 0x0, 0x0, 0x0}
  }
};
extern  void conv_$who_are_you
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */uuid_$t *actuid,
  /* [in] */ndr_$ulong_int boot_time,
  /* [out] */ndr_$ulong_int *seq,
  /* [out] */status_$t *st);
#else
 ( );
#endif
typedef struct conv_$epv_t {
void (*conv_$who_are_you)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */uuid_$t *actuid,
  /* [in] */ndr_$ulong_int boot_time,
  /* [out] */ndr_$ulong_int *seq,
  /* [out] */status_$t *st)
#else
()
#endif
;
} conv_$epv_t;
globalref conv_$epv_t conv_$client_epv;
globalref conv_$epv_t conv_$manager_epv;
globalref rpc_$epv_t conv_$server_epv;
#endif
