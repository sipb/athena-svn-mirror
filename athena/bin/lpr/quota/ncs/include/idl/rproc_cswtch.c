#define NIDL_GENERATED
#define NIDL_CSWTCH
#include "rproc.h"

void rproc_$create_simple
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr_$char pname[256],
  /* [in] */ndr_$long_int argc,
  /* [in] */rproc_$args_t argv,
  /* [out] */rproc_$t *proc,
  /* [out] */status_$t *st)
#else
 (h, pname, argc, argv, proc, st)
handle_t h;
ndr_$char pname[256];
ndr_$long_int argc;
rproc_$args_t argv;
rproc_$t *proc;
status_$t *st;
#endif
{
(*rproc_$client_epv.rproc_$create_simple)(h, pname, argc, argv, proc, st);
}
