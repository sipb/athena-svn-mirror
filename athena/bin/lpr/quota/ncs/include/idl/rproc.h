#ifndef rproc__included
#define rproc__included
#include "idl_base.h"
#include "rpc.h"
#include "nbase.h"
static rpc_$if_spec_t rproc_$if_spec = {
  1,
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  1,
  {
  0x33c38289,
  0x8000,
  0,
  0xd,
  {0x0, 0x0, 0x80, 0x9c, 0x0, 0x0, 0x0}
  }
};
#define rproc_$cant_run_prog 470155265
#define rproc_$cant_create_proc 470155266
#define rproc_$internal_error 470155267
#define rproc_$too_many_args 470155268
#define rproc_$not_allowed 470155269
#define rproc_$cant_set_id 470155270
typedef ndr_$long_int rproc_$t;
typedef ndr_$char rproc_$arg_t[128];
typedef rproc_$arg_t rproc_$args_t[1];
extern  void rproc_$create_simple
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr_$char pname[256],
  /* [in] */ndr_$long_int argc,
  /* [in] */rproc_$args_t argv,
  /* [out] */rproc_$t *proc,
  /* [out] */status_$t *st);
#else
 ( );
#endif
typedef struct rproc_$epv_t {
void (*rproc_$create_simple)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr_$char pname[256],
  /* [in] */ndr_$long_int argc,
  /* [in] */rproc_$args_t argv,
  /* [out] */rproc_$t *proc,
  /* [out] */status_$t *st)
#else
()
#endif
;
} rproc_$epv_t;
globalref rproc_$epv_t rproc_$client_epv;
globalref rproc_$epv_t rproc_$manager_epv;
globalref rpc_$epv_t rproc_$server_epv;
#endif
