#ifndef rrpc__included
#define rrpc__included
#include "idl_base.h"
#include "rpc.h"
#include "nbase.h"
#include "rpc.h"
static rpc_$if_spec_t rrpc_$if_spec = {
  0,
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  4,
  {
  0x36ce399d,
  0x4000,
  0,
  0xd,
  {0x0, 0x0, 0xc3, 0x66, 0x0, 0x0, 0x0}
  }
};
#define rrpc_$mod 470024192
#define rrpc_$shutdown_not_allowed 470024193
typedef rpc_$if_spec_t rrpc_$interface_vec_t[1];
typedef ndr_$long_int rrpc_$stat_vec_t[1];
#define rrpc_$sv_calls_in 0
#define rrpc_$sv_rcvd 1
#define rrpc_$sv_sent 2
#define rrpc_$sv_calls_out 3
#define rrpc_$sv_frag_resends 4
#define rrpc_$sv_dup_frags_rcvd 5
#define rrpc_$sv_n_calls rrpc_$sv_calls_in
#define rrpc_$sv_n_pkts_rcvd rrpc_$sv_rcvd
#define rrpc_$sv_n_pkts_sent rrpc_$sv_sent
extern  void rrpc_$are_you_there
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rrpc_$inq_stats
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr_$ulong_int max_stats,
  /* [out] */rrpc_$stat_vec_t stats,
  /* [out] */ndr_$long_int *l_stat,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rrpc_$inq_interfaces
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr_$ulong_int max_ifs,
  /* [out] */rrpc_$interface_vec_t ifs,
  /* [out] */ndr_$long_int *l_if,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rrpc_$shutdown
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status_$t *st);
#else
 ( );
#endif
typedef struct rrpc_$epv_t {
void (*rrpc_$are_you_there)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status_$t *st)
#else
()
#endif
;
void (*rrpc_$inq_stats)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr_$ulong_int max_stats,
  /* [out] */rrpc_$stat_vec_t stats,
  /* [out] */ndr_$long_int *l_stat,
  /* [out] */status_$t *st)
#else
()
#endif
;
void (*rrpc_$inq_interfaces)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr_$ulong_int max_ifs,
  /* [out] */rrpc_$interface_vec_t ifs,
  /* [out] */ndr_$long_int *l_if,
  /* [out] */status_$t *st)
#else
()
#endif
;
void (*rrpc_$shutdown)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status_$t *st)
#else
()
#endif
;
} rrpc_$epv_t;
globalref rrpc_$epv_t rrpc_$client_epv;
globalref rrpc_$epv_t rrpc_$manager_epv;
globalref rpc_$epv_t rrpc_$server_epv;
#endif
