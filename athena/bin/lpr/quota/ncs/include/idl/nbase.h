#ifndef nbase__included
#define nbase__included
#include "idl_base.h"
typedef ndr_$short_int binteger;
typedef ndr_$short_int pinteger;
typedef ndr_$long_int linteger;
typedef struct status_$t status_$t;
struct status_$t {
ndr_$long_int all;
};
#define status_$ok 0
typedef struct uuid_$t uuid_$t;
struct uuid_$t {
ndr_$ulong_int time_high;
ndr_$ushort_int time_low;
ndr_$ushort_int reserved;
ndr_$byte family;
ndr_$byte host[7];
};
#ifdef __STDC__
handle_t uuid_$t_bind(uuid_$t h);
void uuid_$t_unbind(uuid_$t uh, handle_t h);
#else
handle_t uuid_$t_bind();
void uuid_$t_unbind();
#endif
#define socket_$unspec_port 0
#define socket_$unspec ((ndr_$ushort_int) 0x0)
#define socket_$unix ((ndr_$ushort_int) 0x1)
#define socket_$internet ((ndr_$ushort_int) 0x2)
#define socket_$implink ((ndr_$ushort_int) 0x3)
#define socket_$pup ((ndr_$ushort_int) 0x4)
#define socket_$chaos ((ndr_$ushort_int) 0x5)
#define socket_$ns ((ndr_$ushort_int) 0x6)
#define socket_$nbs ((ndr_$ushort_int) 0x7)
#define socket_$ecma ((ndr_$ushort_int) 0x8)
#define socket_$datakit ((ndr_$ushort_int) 0x9)
#define socket_$ccitt ((ndr_$ushort_int) 0xa)
#define socket_$sna ((ndr_$ushort_int) 0xb)
#define socket_$unspec2 ((ndr_$ushort_int) 0xc)
#define socket_$dds ((ndr_$ushort_int) 0xd)
typedef ndr_$ushort_int socket_$addr_family_t;
#define socket_$num_families 32
#define socket_$sizeof_family 2
#define socket_$sizeof_data 14
#define socket_$sizeof_ndata 12
#define socket_$sizeof_hdata 12
typedef struct socket_$addr_t socket_$addr_t;
struct socket_$addr_t {
socket_$addr_family_t family;
ndr_$byte data[14];
};
typedef struct socket_$net_addr_t socket_$net_addr_t;
struct socket_$net_addr_t {
socket_$addr_family_t family;
ndr_$byte data[12];
};
typedef struct socket_$host_id_t socket_$host_id_t;
struct socket_$host_id_t {
socket_$addr_family_t family;
ndr_$byte data[12];
};
#endif
