#define NIDL_GENERATED
#define NIDL_CSTUB
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

static void conv_$who_are_you_csr
#ifdef __STDC__
 (
  /* [in] */handle_t h_,
  /* [in] */uuid_$t *actuid_,
  /* [in] */ndr_$ulong_int boot_time_,
  /* [out] */ndr_$ulong_int *seq_,
  /* [out] */status_$t *st_)
#else
 (h_, actuid_, boot_time_, seq_, st_)
handle_t h_;
uuid_$t *actuid_;
ndr_$ulong_int boot_time_;
ndr_$ulong_int *seq_;
status_$t *st_;
#endif
{

/* rpc_$sar arguments */
volatile rpc_$ppkt_t *ip;
ndr_$ulong_int ilen;
rpc_$ppkt_t *op;
rpc_$ppkt_t *routs;
ndr_$ulong_int olen;
rpc_$drep_t drep;
ndr_$boolean free_outs;
status_$t st;

/* other client side local variables */
rpc_$ppkt_t ins;
rpc_$ppkt_t outs;
pfm_$cleanup_rec cleanup_rec;
status_$t cleanup_status;
ndr_$ushort_int data_offset;
ndr_$ulong_int bound;
rpc_$mp_t mp;
rpc_$mp_t dbp;
ndr_$ushort_int count;
volatile ndr_$boolean free_ins;

/* local variables */
ndr_$ushort_int xXx_163f_ /* actuid_host_cv_ */ ;
ndr_$byte *xXx_5b2_ /* actuid_host_epe_ */ ;
ndr_$ulong_int xXx_1352_ /* actuid_host_i_ */ ;
cleanup_status = pfm_$cleanup (&cleanup_rec);
if (cleanup_status.all != pfm_$cleanup_set) {
    if (free_ins) rpc_$free_pkt(ip);
    pfm_$signal (cleanup_status);
    }


/* marshalling init */
data_offset=h_->data_offset;
bound=0;

/* bound calculations */
bound += 23;

/* buffer allocation */
if(free_ins=(bound+data_offset>sizeof(rpc_$ppkt_t)))
    ip=rpc_$alloc_pkt(bound);
else 
    ip= &ins;
rpc_$init_mp(mp, dbp, ip, data_offset);

/* marshalling */
rpc_$marshall_ulong_int(mp, (*actuid_).time_high);
rpc_$advance_mp(mp, 4);
rpc_$marshall_ushort_int(mp, (*actuid_).time_low);
rpc_$advance_mp(mp, 2);
rpc_$marshall_ushort_int(mp, (*actuid_).reserved);
rpc_$advance_mp(mp, 2);
rpc_$marshall_byte(mp, (*actuid_).family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_5b2_ = &(*actuid_).host[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_5b2_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_5b2_ = &(*actuid_).host[0];
for (xXx_1352_=7; xXx_1352_; xXx_1352_--){
rpc_$marshall_byte(mp, (*xXx_5b2_));
rpc_$advance_mp(mp, 1);
xXx_5b2_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, boot_time_);
rpc_$advance_mp(mp, 4);

/* runtime call */
ilen=mp-dbp;
op= &outs;
rpc_$sar(h_,
 (long)0+rpc_$idempotent,
 &conv_$if_spec,
 0L,
 ip,
 ilen,
 op,
 (long)sizeof(rpc_$ppkt_t),
 &routs,
 &olen,
 (rpc_$drep_t *)&drep,
 &free_outs,
 &st);

/* unmarshalling init */
rpc_$init_mp(mp, dbp, routs, data_offset);
if (rpc_$equal_drep (drep, rpc_$local_drep)) {

/* unmarshalling */
rpc_$unmarshall_ulong_int(mp, (*seq_));
rpc_$advance_mp(mp, 4);
rpc_$unmarshall_long_int(mp, (*st_).all);
} else {
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, (*seq_));
rpc_$advance_mp(mp, 4);
rpc_$convert_long_int(drep, rpc_$local_drep, mp, (*st_).all);
}

/* buffer deallocation */
if(free_outs) rpc_$free_pkt(routs);
if(free_ins) rpc_$free_pkt(ip);
pfm_$rls_cleanup (&cleanup_rec, &cleanup_status);

}
globaldef conv_$epv_t conv_$client_epv = {
conv_$who_are_you_csr
};
