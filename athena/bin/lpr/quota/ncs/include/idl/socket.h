#ifndef socket__included
#define socket__included
#include "idl_base.h"
#include "nbase.h"
#define socket_$wk_fwd ((ndr_$ushort_int) 0x0)
typedef ndr_$ushort_int socket_$wk_ports_t;
typedef ndr_$char socket_$string_t[100];
typedef socket_$addr_t socket_$addr_list_t[1];
typedef ndr_$ulong_int socket_$len_list_t[1];
typedef ndr_$char socket_$local_sockaddr_t[50];
#define socket_$eq_hostid 1
#define socket_$eq_netaddr 2
#define socket_$eq_port 4
#define socket_$eq_network 8
#define socket_$addr_module_code 268566528
#define socket_$buff_too_large 268566529
#define socket_$buff_too_small 268566530
#define socket_$bad_numeric_name 268566531
#define socket_$cant_find_name 268566532
#define socket_$cant_cvrt_addr_to_name 268566533
#define socket_$cant_get_local_name 268566534
#define socket_$cant_create_socket 268566535
#define socket_$cant_get_if_config 268566536
#define socket_$internal_error 268566537
#define socket_$family_not_valid 268566538
#define socket_$invalid_name_format 268566539
extern  ndr_$boolean socket_$valid_family
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int family,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$valid_families
#ifdef __STDC__
 (
  /* [in, out] */ndr_$ulong_int *num,
  /* [out] */socket_$addr_family_t families[1],
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  ndr_$ulong_int socket_$inq_port
#ifdef __STDC__
 (
  /* [in] */socket_$addr_t *saddr,
  /* [in] */ndr_$ulong_int slen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$set_port
#ifdef __STDC__
 (
  /* [in, out] */socket_$addr_t *saddr,
  /* [in, out] */ndr_$ulong_int *slen,
  /* [in] */ndr_$ulong_int port,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$set_wk_port
#ifdef __STDC__
 (
  /* [in, out] */socket_$addr_t *saddr,
  /* [in, out] */ndr_$ulong_int *slen,
  /* [in] */ndr_$ulong_int port,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  ndr_$boolean socket_$equal
#ifdef __STDC__
 (
  /* [in] */socket_$addr_t *saddr1,
  /* [in] */ndr_$ulong_int slen1,
  /* [in] */socket_$addr_t *saddr2,
  /* [in] */ndr_$ulong_int slen2,
  /* [in] */ndr_$ulong_int flags,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$from_name
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int family,
  /* [in] */socket_$string_t name,
  /* [in] */ndr_$ulong_int namelen,
  /* [in] */ndr_$ulong_int port,
  /* [out] */socket_$addr_t *saddr,
  /* [in, out] */ndr_$ulong_int *slen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  ndr_$ulong_int socket_$family_from_name
#ifdef __STDC__
 (
  /* [in] */socket_$string_t name,
  /* [in] */ndr_$ulong_int namelen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$family_to_name
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int family,
  /* [out] */socket_$string_t name,
  /* [in, out] */ndr_$ulong_int *namelen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$to_name
#ifdef __STDC__
 (
  /* [in] */socket_$addr_t *saddr,
  /* [in] */ndr_$ulong_int slen,
  /* [out] */socket_$string_t name,
  /* [in, out] */ndr_$ulong_int *namelen,
  /* [out] */ndr_$ulong_int *port,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$to_numeric_name
#ifdef __STDC__
 (
  /* [in] */socket_$addr_t *saddr,
  /* [in] */ndr_$ulong_int slen,
  /* [out] */socket_$string_t name,
  /* [in, out] */ndr_$ulong_int *namelen,
  /* [out] */ndr_$ulong_int *port,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$set_broadcast
#ifdef __STDC__
 (
  /* [in, out] */socket_$addr_t *saddr,
  /* [in, out] */ndr_$ulong_int *slen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  ndr_$ulong_int socket_$max_pkt_size
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int family,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$inq_my_netaddr
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int family,
  /* [out] */socket_$net_addr_t *naddr,
  /* [in, out] */ndr_$ulong_int *nlen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$inq_netaddr
#ifdef __STDC__
 (
  /* [in] */socket_$addr_t *saddr,
  /* [in] */ndr_$ulong_int slen,
  /* [out] */socket_$net_addr_t *naddr,
  /* [in, out] */ndr_$ulong_int *nlen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$set_netaddr
#ifdef __STDC__
 (
  /* [in, out] */socket_$addr_t *saddr,
  /* [in, out] */ndr_$ulong_int *slen,
  /* [in] */socket_$net_addr_t *naddr,
  /* [in] */ndr_$ulong_int nlen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$inq_hostid
#ifdef __STDC__
 (
  /* [in] */socket_$addr_t *saddr,
  /* [in] */ndr_$ulong_int slen,
  /* [out] */socket_$host_id_t *hid,
  /* [in, out] */ndr_$ulong_int *hlen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$set_hostid
#ifdef __STDC__
 (
  /* [in, out] */socket_$addr_t *saddr,
  /* [in, out] */ndr_$ulong_int *slen,
  /* [in] */socket_$host_id_t *hid,
  /* [in] */ndr_$ulong_int hlen,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$inq_broad_addrs
#ifdef __STDC__
 (
  /* [in] */ndr_$ulong_int family,
  /* [in] */ndr_$ulong_int port,
  /* [out] */socket_$addr_list_t brd_addrs,
  /* [out] */socket_$len_list_t brd_lens,
  /* [in, out] */ndr_$ulong_int *len,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$to_local_rep
#ifdef __STDC__
 (
  /* [in] */socket_$addr_t *saddr,
  /* [in, out] */socket_$local_sockaddr_t lcl_saddr,
  /* [out] */status_$t *st);
#else
 ( );
#endif
extern  void socket_$from_local_rep
#ifdef __STDC__
 (
  /* [in, out] */socket_$addr_t *saddr,
  /* [in] */socket_$local_sockaddr_t lcl_saddr,
  /* [out] */status_$t *st);
#else
 ( );
#endif
#endif
