#define NIDL_GENERATED
#define NIDL_CSWTCH
#include "rrpc.h"

void rrpc_$are_you_there
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status_$t *st)
#else
 (h, st)
handle_t h;
status_$t *st;
#endif
{
(*rrpc_$client_epv.rrpc_$are_you_there)(h, st);
}

void rrpc_$inq_stats
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr_$ulong_int max_stats,
  /* [out] */rrpc_$stat_vec_t stats,
  /* [out] */ndr_$long_int *l_stat,
  /* [out] */status_$t *st)
#else
 (h, max_stats, stats, l_stat, st)
handle_t h;
ndr_$ulong_int max_stats;
rrpc_$stat_vec_t stats;
ndr_$long_int *l_stat;
status_$t *st;
#endif
{
(*rrpc_$client_epv.rrpc_$inq_stats)(h, max_stats, stats, l_stat, st);
}

void rrpc_$inq_interfaces
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr_$ulong_int max_ifs,
  /* [out] */rrpc_$interface_vec_t ifs,
  /* [out] */ndr_$long_int *l_if,
  /* [out] */status_$t *st)
#else
 (h, max_ifs, ifs, l_if, st)
handle_t h;
ndr_$ulong_int max_ifs;
rrpc_$interface_vec_t ifs;
ndr_$long_int *l_if;
status_$t *st;
#endif
{
(*rrpc_$client_epv.rrpc_$inq_interfaces)(h, max_ifs, ifs, l_if, st);
}

void rrpc_$shutdown
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status_$t *st)
#else
 (h, st)
handle_t h;
status_$t *st;
#endif
{
(*rrpc_$client_epv.rrpc_$shutdown)(h, st);
}
