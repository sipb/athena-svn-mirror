#define NIDL_GENERATED
#define NIDL_SSTUB
#include "conv.h"
#ifndef IDL_BASE_SUPPORTS_V1
"The version of idl_base.h is not compatible with this stub/switch code."
#endif
#ifndef RPC_IDL_SUPPORTS_V1
"The version of rpc.idl is not compatible with this stub/switch code."
#endif
#ifndef NCASTAT_IDL_SUPPORTS_V1
"The version of ncastat.idl is not compatible with this stub/switch code."
#endif
#include <ppfm.h>

static void conv_$who_are_you_ssr
#ifdef __STDC__
(
 handle_t h,
 rpc_$ppkt_t *ins,
 ndr_$ulong_int ilen,
 rpc_$ppkt_t *outs,
 ndr_$ulong_int omax,
 rpc_$drep_t drep,
 rpc_$ppkt_t **routs,
 ndr_$ulong_int *olen,
 ndr_$boolean *free_outs,
 status_$t *st
)
#else
(
 h,
 ins,ilen,
 outs,omax,
 drep,
 routs,olen,
 free_outs,
 st)

handle_t h;
rpc_$ppkt_t *ins;
ndr_$ulong_int ilen;
rpc_$ppkt_t *outs;
ndr_$ulong_int omax;
rpc_$drep_t drep;
rpc_$ppkt_t **routs;
ndr_$ulong_int *olen;
ndr_$boolean *free_outs;
status_$t *st;
#endif

{

/* marshalling variables */
ndr_$ushort_int data_offset;
ndr_$ulong_int bound;
rpc_$mp_t mp;
rpc_$mp_t dbp;
ndr_$ushort_int count;

/* local variables */
status_$t st_;
ndr_$ulong_int seq_;
ndr_$ulong_int boot_time_;
uuid_$t actuid_;
ndr_$ushort_int xXx_163f_ /* actuid_host_cv_ */ ;
ndr_$byte *xXx_5b2_ /* actuid_host_epe_ */ ;
ndr_$ulong_int xXx_1352_ /* actuid_host_i_ */ ;

/* unmarshalling init */
data_offset=h->data_offset;
rpc_$init_mp(mp, dbp, ins, data_offset);
if (rpc_$equal_drep (drep, rpc_$local_drep)) {

/* unmarshalling */
rpc_$unmarshall_ulong_int(mp, actuid_.time_high);
rpc_$advance_mp(mp, 4);
rpc_$unmarshall_ushort_int(mp, actuid_.time_low);
rpc_$advance_mp(mp, 2);
rpc_$unmarshall_ushort_int(mp, actuid_.reserved);
rpc_$advance_mp(mp, 2);
rpc_$unmarshall_byte(mp, actuid_.family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_5b2_ = &actuid_.host[0];
rpc_$block_copy((rpc_$byte_p_t)mp, (rpc_$byte_p_t)xXx_5b2_, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_5b2_ = &actuid_.host[0];
for (xXx_1352_=7; xXx_1352_; xXx_1352_--){
rpc_$unmarshall_byte(mp, (*xXx_5b2_));
rpc_$advance_mp(mp, 1);
xXx_5b2_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$unmarshall_ulong_int(mp, boot_time_);
} else {
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, actuid_.time_high);
rpc_$advance_mp(mp, 4);
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, actuid_.time_low);
rpc_$advance_mp(mp, 2);
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, actuid_.reserved);
rpc_$advance_mp(mp, 2);
rpc_$convert_byte(drep, rpc_$local_drep, mp, actuid_.family);
rpc_$advance_mp(mp, 1);
xXx_5b2_ = &actuid_.host[0];
for (xXx_1352_=7; xXx_1352_; xXx_1352_--){
rpc_$convert_byte(drep, rpc_$local_drep, mp, (*xXx_5b2_));
rpc_$advance_mp(mp, 1);
xXx_5b2_++;
}
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, boot_time_);
}

/* server call */
conv_$who_are_you(h, &actuid_, boot_time_, &seq_, &st_);
bound=0;

/* bound calculations */
bound += 8;

/* buffer allocation */
if(*free_outs=(bound>omax))
    *routs=rpc_$alloc_pkt(bound);
else
    *routs=outs;
rpc_$init_mp(mp, dbp, *routs, data_offset);

/* marshalling */
rpc_$marshall_ulong_int(mp, seq_);
rpc_$advance_mp(mp, 4);
rpc_$marshall_long_int(mp, st_.all);
rpc_$advance_mp(mp, 4);

*olen=mp-dbp;

st->all=status_$ok;
}
globaldef conv_$epv_t conv_$manager_epv = {
conv_$who_are_you
};

static rpc_$server_stub_t conv_$server_epva[]={
(rpc_$server_stub_t)conv_$who_are_you_ssr
};
globaldef rpc_$epv_t conv_$server_epv=(rpc_$epv_t)conv_$server_epva;
