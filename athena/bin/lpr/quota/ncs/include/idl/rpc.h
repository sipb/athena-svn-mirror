#ifndef rpc__included
#define rpc__included
#include "idl_base.h"
#include "nbase.h"
#include "ncastat.h"
#define RPC_IDL_SUPPORTS_V1 ndr_$true
#define rpc_$unbound_port socket_$unspec_port
#define rpc_$mod 469827584
#define rpc_$comm_failure nca_status_$comm_failure
#define rpc_$op_rng_error nca_status_$op_rng_error
#define rpc_$unk_if nca_status_$unk_if
#define rpc_$cant_create_sock 469827588
#define rpc_$cant_bind_sock 469827589
#define rpc_$wrong_boot_time nca_status_$wrong_boot_time
#define rpc_$too_many_ifs 469827591
#define rpc_$not_in_call 469827592
#define rpc_$you_crashed nca_status_$you_crashed
#define rpc_$no_port 469827594
#define rpc_$proto_error nca_status_$proto_error
#define rpc_$too_many_sockets 469827596
#define rpc_$illegal_register 469827597
#define rpc_$cant_recv 469827598
#define rpc_$bad_pkt 469827599
#define rpc_$unbound_handle 469827600
#define rpc_$addr_in_use 469827601
#define rpc_$in_args_too_big 469827602
#define rpc_$out_args_too_big nca_status_$out_args_too_big
#define rpc_$server_too_busy nca_status_$server_too_busy
#define rpc_$string_too_long nca_status_$string_too_long
#define rpc_$too_many_objects 469827606
#define rpc_$invalid_handle 469827607
#define rpc_$not_authenticated 469827608
#define rpc_$invalid_auth_type 469827609
#define rpc_$cant_malloc 469827610
#define rpc_$cant_nmalloc 469827611
typedef ndr_$ulong_int rpc_$sar_opts_t;
#define rpc_$brdcst 1
#define rpc_$idempotent 2
#define rpc_$maybe 4
#define rpc_$drep_int_big_endian 0
#define rpc_$drep_int_little_endian 1
#define rpc_$drep_float_ieee 0
#define rpc_$drep_float_vax 1
#define rpc_$drep_float_cray 2
#define rpc_$drep_float_ibm 3
#define rpc_$drep_char_ascii 0
#define rpc_$drep_char_ebcdic 1
typedef ndr_$short_float *rpc_$short_float_p_t;
typedef ndr_$long_float *rpc_$long_float_p_t;
typedef rpc_$short_float_p_t rpc_$short_float_p;
typedef rpc_$long_float_p_t rpc_$long_float_p;
typedef ndr_$char *rpc_$char_p_t;
typedef ndr_$byte *rpc_$byte_p_t;
#define rpc_$max_alignment 8
#define rpc_$mispacked_hdr 0
#define rpc_$max_pkt_size 1024
#define rpc_$max_pkt_size_8 128
typedef struct rpc_$ppkt_t rpc_$ppkt_t;
struct rpc_$ppkt_t {
ndr_$long_float d[128];
};
typedef rpc_$ppkt_t *rpc_$ppkt_p_t;
typedef void (*rpc_$server_stub_t)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */rpc_$ppkt_p_t ins,
  /* [in] */ndr_$ulong_int ilen,
  /* [in] */rpc_$ppkt_p_t outs,
  /* [in] */ndr_$ulong_int omax,
  /* [in] */rpc_$drep_t drep,
  /* [out] */rpc_$ppkt_p_t *routs,
  /* [out] */ndr_$ulong_int *olen,
  /* [out] */ndr_$boolean *must_free,
  /* [out] */status_$t *st)
#else
()
#endif
;
typedef rpc_$server_stub_t *rpc_$epv_t;
typedef void (*rpc_$mgr_proc_t)
#ifdef __STDC__
 (
  /* [in] */handle_t h)
#else
()
#endif
;
typedef rpc_$mgr_proc_t *rpc_$mgr_epv_t;
typedef void (*rpc_$generic_server_stub_t)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */rpc_$ppkt_p_t ins,
  /* [in] */ndr_$ulong_int ilen,
  /* [in] */rpc_$ppkt_p_t outs,
  /* [in] */ndr_$ulong_int omax,
  /* [in] */rpc_$drep_t drep,
  /* [in] */rpc_$mgr_epv_t epv,
  /* [out] */rpc_$ppkt_p_t *routs,
  /* [out] */ndr_$ulong_int *olen,
  /* [out] */ndr_$boolean *must_free,
  /* [out] */status_$t *st)
#else
()
#endif
;
typedef rpc_$generic_server_stub_t *rpc_$generic_epv_t;
typedef struct rpc_$if_spec_t rpc_$if_spec_t;
struct rpc_$if_spec_t {
ndr_$ulong_int vers;
ndr_$ushort_int port[32];
ndr_$ushort_int opcnt;
uuid_$t id;
};
typedef ndr_$boolean (*rpc_$shut_check_fn_t)
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status_$t *st)
#else
()
#endif
;
typedef void (*rpc_$auth_log_fn_t)
#ifdef __STDC__
 (
  /* [in] */status_$t st,
  /* [in] */socket_$addr_t *addr,
  /* [in] */ndr_$ulong_int addrlen)
#else
()
#endif
;
typedef ndr_$char *rpc_$cksum_t;
typedef ndr_$char rpc_$string_t[256];
extern  void rpc_$use_family
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int family,
  /* [out] */socket_$addr_t *saddr,
  /* [out] */ndr_$ulong_int *slen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$use_family_wk
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int family,
  /* [in] */rpc_$if_spec_t *ifspec,
  /* [out] */socket_$addr_t *saddr,
  /* [out] */ndr_$ulong_int *slen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$register
#ifdef __STDC__
 (
  /* [in] */rpc_$if_spec_t *ifspec,
  /* [in] */rpc_$epv_t epv,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$unregister
#ifdef __STDC__
 (
  /* [in] */rpc_$if_spec_t *ifspec,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$register_mgr
#ifdef __STDC__
 (
  /* [in] */uuid_$t *typ,
  /* [in] */rpc_$if_spec_t *ifspec,
  /* [in] */rpc_$generic_epv_t sepv,
  /* [in] */rpc_$mgr_epv_t mepv,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$register_object
#ifdef __STDC__
 (
  /* [in] */uuid_$t *obj,
  /* [in] */uuid_$t *typ,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  handle_t rpc_$get_handle
#ifdef __STDC__
 (
  /* [in] */uuid_$t *actuid,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  handle_t rpc_$alloc_handle
#ifdef __STDC__
 (
  /* [in] */uuid_$t *obj,
  /* [in] */ndr_$ulong_int family,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$set_binding
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */socket_$addr_t *saddr,
  /* [in] */ndr_$ulong_int slen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$inq_binding
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */socket_$addr_t *saddr,
  /* [out] */ndr_$ulong_int *slen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$clear_server_binding
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$clear_binding
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  handle_t rpc_$bind
#ifdef __STDC__
 (
  /* [in] */uuid_$t *obj,
  /* [in] */socket_$addr_t *saddr,
  /* [in] */ndr_$ulong_int slen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$free_handle
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  handle_t rpc_$dup_handle
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$listen
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int max_calls,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$listen_dispatch
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int sock,
  /* [in] */rpc_$ppkt_t *pkt,
  /* [in] */rpc_$cksum_t cksum,
  /* [in] */socket_$addr_t *from,
  /* [in] */ndr_$ulong_int from_len,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$listen_recv
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int sock,
  /* [out] */rpc_$ppkt_t *pkt,
  /* [out] */rpc_$cksum_t *cksum,
  /* [out] */socket_$addr_t *from,
  /* [out] */ndr_$ulong_int *from_len,
  /* [out] */ndr_$ulong_int *ptype,
  /* [out] */uuid_$t *obj,
  /* [out] */uuid_$t *if_id,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$forward
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int sock,
  /* [in] */socket_$addr_t *from,
  /* [in] */ndr_$ulong_int from_len,
  /* [in] */socket_$addr_t *taddr,
  /* [in] */ndr_$ulong_int to_len,
  /* [in] */rpc_$ppkt_t *pkt,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$name_to_sockaddr
#ifdef __STDC__
 (
  /* [in] */rpc_$string_t name,
  /* [in] */ndr_$ulong_int namelen,
  /* [in] */ndr_$ulong_int port,
  /* [in] */ndr_$ulong_int family,
  /* [out] */socket_$addr_t *saddr,
  /* [out] */ndr_$ulong_int *slen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$sockaddr_to_name
#ifdef __STDC__
 (
  /* [in] */socket_$addr_t *saddr,
  /* [in] */ndr_$ulong_int slen,
  /* [out] */rpc_$string_t name,
  /* [in, out] */ndr_$ulong_int *namelen,
  /* [out] */ndr_$ulong_int *port,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$inq_object
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [out] */uuid_$t *obj,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$shutdown
#ifdef __STDC__
 (
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$allow_remote_shutdown
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int allow,
  /* [in] */rpc_$shut_check_fn_t cproc,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$set_auth_logger
#ifdef __STDC__
 (
  /* [in] */rpc_$auth_log_fn_t lproc);
#else
 ( );
#endif
extern  ndr_$ulong_int rpc_$set_short_timeout
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */ndr_$ulong_int on,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  ndr_$ulong_int rpc_$set_fault_mode
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int on);
#else
 ( );
#endif
extern  void rpc_$sar
#ifdef __STDC__
 (
  /* [in] */handle_t hp,
  /* [in] */rpc_$sar_opts_t opts,
  /* [in] */rpc_$if_spec_t *ifspec,
  /* [in] */ndr_$ulong_int opn,
  /* [in] */rpc_$ppkt_t *ins,
  /* [in] */ndr_$ulong_int ilen,
  /* [in] */rpc_$ppkt_t *outs,
  /* [in] */ndr_$ulong_int omax,
  /* [out] */rpc_$ppkt_p_t *routs,
  /* [out] */ndr_$ulong_int *olen,
  /* [out] */rpc_$drep_t *drep,
  /* [out] */ndr_$boolean *must_free,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void rpc_$cvt_short_float
#ifdef __STDC__
 (
  /* [in] */rpc_$drep_t src_drep,
  /* [in] */rpc_$drep_t dst_drep,
  /* [in] */rpc_$short_float_p_t src,
  /* [in] */rpc_$short_float_p_t dst);
#else
 ( );
#endif
extern  void rpc_$cvt_long_float
#ifdef __STDC__
 (
  /* [in] */rpc_$drep_t src_drep,
  /* [in] */rpc_$drep_t dst_drep,
  /* [in] */rpc_$long_float_p_t src,
  /* [in] */rpc_$long_float_p_t dst);
#else
 ( );
#endif
extern  rpc_$ppkt_p_t rpc_$alloc_pkt
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int len);
#else
 ( );
#endif
extern  void rpc_$free_pkt
#ifdef __STDC__
 (
  /* [in] */rpc_$ppkt_p_t p);
#else
 ( );
#endif
extern  void rpc_$cvt_string
#ifdef __STDC__
 (
  /* [in] */rpc_$drep_t src_drep,
  /* [in] */rpc_$drep_t dst_drep,
  /* [in] */rpc_$char_p_t src,
  /* [in] */rpc_$char_p_t dst);
#else
 ( );
#endif
extern  void rpc_$block_copy
#ifdef __STDC__
 (
  /* [in] */rpc_$byte_p_t src,
  /* [in] */rpc_$byte_p_t dst,
  /* [in] */ndr_$ulong_int count);
#else
 ( );
#endif
#endif
