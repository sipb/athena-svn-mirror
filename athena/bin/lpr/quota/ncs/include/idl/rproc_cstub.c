#define NIDL_GENERATED
#define NIDL_CSTUB
#include "rproc.h"
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

static void rproc_$create_simple_csr
#ifdef __STDC__
 (
  /* [in] */handle_t h_,
  /* [in] */ndr_$char pname_[256],
  /* [in] */ndr_$long_int argc_,
  /* [in] */rproc_$args_t argv_,
  /* [out] */rproc_$t *proc_,
  /* [out] */status_$t *st_)
#else
 (h_, pname_, argc_, argv_, proc_, st_)
handle_t h_;
ndr_$char pname_[256];
ndr_$long_int argc_;
rproc_$args_t argv_;
rproc_$t *proc_;
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
ndr_$ushort_int xXx_bd9_ /* argv_el_strlen_ */ ;
ndr_$ushort_int xXx_6f6_ /* argv_cv_ */ ;
rproc_$arg_t *xXx_c10_ /* argv_epe_ */ ;
ndr_$ulong_int xXx_c7f_ /* argv_i_ */ ;
ndr_$ushort_int xXx_1cf3_ /* pname_strlen_ */ ;
cleanup_status = pfm_$cleanup (&cleanup_rec);
if (cleanup_status.all != pfm_$cleanup_set) {
    if (free_ins) rpc_$free_pkt(ip);
    pfm_$signal (cleanup_status);
    }


/* marshalling init */
data_offset=h_->data_offset;
bound=0;

/* bound calculations */
bound += strlen((ndr_$char *) pname_);
xXx_c10_= argv_;
for(xXx_c7f_=(argc_-1+1); xXx_c7f_; xXx_c7f_--){
bound += strlen((ndr_$char *) (*xXx_c10_));
xXx_c10_++;
}
bound += 4 * ((argc_-1+1));
bound += 14;

/* buffer allocation */
if(free_ins=(bound+data_offset>sizeof(rpc_$ppkt_t)))
    ip=rpc_$alloc_pkt(bound);
else 
    ip= &ins;
rpc_$init_mp(mp, dbp, ip, data_offset);

/* marshalling */
rpc_$marshall_string(xXx_1cf3_, 256, mp, pname_);
rpc_$advance_mp(mp, xXx_1cf3_+1);
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_long_int(mp, argc_);
rpc_$advance_mp(mp, 4);
count = (argc_-1+1);
rpc_$marshall_ushort_int (mp, count);
rpc_$advance_mp(mp, 2);
xXx_6f6_ = (argc_-1+1);
rpc_$marshall_ushort_int(mp, xXx_6f6_);
rpc_$advance_mp(mp, 2);
if (xXx_6f6_>count) SIGNAL(nca_status_$invalid_bound);
xXx_c10_ = argv_;
for (xXx_c7f_=xXx_6f6_; xXx_c7f_; xXx_c7f_--){
rpc_$align_ptr_relative (mp, dbp, 2);
rpc_$marshall_string(xXx_bd9_, 128, mp, (*xXx_c10_));
rpc_$advance_mp(mp, xXx_bd9_+1);
xXx_c10_++;
}

/* runtime call */
ilen=mp-dbp;
op= &outs;
rpc_$sar(h_,
 (long)0,
 &rproc_$if_spec,
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
rpc_$unmarshall_long_int(mp, (*proc_));
rpc_$advance_mp(mp, 4);
rpc_$unmarshall_long_int(mp, (*st_).all);
} else {
rpc_$convert_long_int(drep, rpc_$local_drep, mp, (*proc_));
rpc_$advance_mp(mp, 4);
rpc_$convert_long_int(drep, rpc_$local_drep, mp, (*st_).all);
}

/* buffer deallocation */
if(free_outs) rpc_$free_pkt(routs);
if(free_ins) rpc_$free_pkt(ip);
pfm_$rls_cleanup (&cleanup_rec, &cleanup_status);

}
globaldef rproc_$epv_t rproc_$client_epv = {
rproc_$create_simple_csr
};
