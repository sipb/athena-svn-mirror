#define NIDL_GENERATED
#define NIDL_SSTUB
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

static void rproc_$create_simple_ssr
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
pfm_$cleanup_rec cleanup_rec;
status_$t cleanup_status;

/* local variables */
status_$t st_;
rproc_$t proc_;
volatile rproc_$arg_t *argv_;
ndr_$ushort_int xXx_bd9_ /* argv_el_strlen_ */ ;
ndr_$ushort_int xXx_6f6_ /* argv_cv_ */ ;
rproc_$arg_t *xXx_c10_ /* argv_epe_ */ ;
ndr_$ulong_int xXx_c7f_ /* argv_i_ */ ;
ndr_$long_int argc_;
ndr_$char pname_[256];
ndr_$ushort_int xXx_1cf3_ /* pname_strlen_ */ ;
cleanup_status = pfm_$cleanup (&cleanup_rec);
if (cleanup_status.all != pfm_$cleanup_set) {
    if (argv_!= NULL) rpc_$free(argv_);
    pfm_$signal (cleanup_status);
    }

argv_= NULL;


/* unmarshalling init */
data_offset=h->data_offset;
rpc_$init_mp(mp, dbp, ins, data_offset);
if (rpc_$equal_drep (drep, rpc_$local_drep)) {

/* unmarshalling */
rpc_$unmarshall_string(xXx_1cf3_, mp, pname_);
rpc_$advance_mp(mp, xXx_1cf3_+1);
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$unmarshall_long_int(mp, argc_);
rpc_$advance_mp(mp, 4);
rpc_$unmarshall_ushort_int (mp, count);
argv_ = (rproc_$arg_t *) rpc_$malloc (count * sizeof(rproc_$arg_t ));
rpc_$advance_mp(mp, 2);
rpc_$unmarshall_ushort_int(mp, xXx_6f6_);
rpc_$advance_mp(mp, 2);
if (xXx_6f6_>count) SIGNAL(nca_status_$invalid_bound);
xXx_c10_ = argv_;
for (xXx_c7f_=xXx_6f6_; xXx_c7f_; xXx_c7f_--){
rpc_$align_ptr_relative (mp, dbp, 2);
rpc_$unmarshall_string(xXx_bd9_, mp, (*xXx_c10_));
rpc_$advance_mp(mp, xXx_bd9_+1);
xXx_c10_++;
}
} else {
rpc_$convert_string(drep, rpc_$local_drep, xXx_1cf3_, mp, pname_);
rpc_$advance_mp(mp, xXx_1cf3_+1);
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$convert_long_int(drep, rpc_$local_drep, mp, argc_);
rpc_$advance_mp(mp, 4);
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, count);
argv_= (rproc_$arg_t *) rpc_$malloc(count * sizeof(rproc_$arg_t ));
rpc_$advance_mp(mp, 2);
rpc_$convert_ushort_int (drep, rpc_$local_drep, mp, xXx_6f6_);
rpc_$advance_mp(mp, 2);
if (xXx_6f6_>count) SIGNAL(nca_status_$invalid_bound);
xXx_c10_ = argv_;
for (xXx_c7f_=xXx_6f6_; xXx_c7f_; xXx_c7f_--){
rpc_$align_ptr_relative (mp, dbp, 2);
rpc_$convert_string(drep, rpc_$local_drep, xXx_bd9_, mp, (*xXx_c10_));
rpc_$advance_mp(mp, xXx_bd9_+1);
xXx_c10_++;
}
}

/* server call */
rproc_$create_simple(h, pname_, argc_, (rproc_$arg_t *)argv_, &proc_, &st_);
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
rpc_$marshall_long_int(mp, proc_);
rpc_$advance_mp(mp, 4);
rpc_$marshall_long_int(mp, st_.all);
rpc_$advance_mp(mp, 4);

*olen=mp-dbp;
if (argv_ != NULL) rpc_$free((char *)argv_);
pfm_$rls_cleanup (&cleanup_rec, &cleanup_status);


st->all=status_$ok;
}
globaldef rproc_$epv_t rproc_$manager_epv = {
rproc_$create_simple
};

static rpc_$server_stub_t rproc_$server_epva[]={
(rpc_$server_stub_t)rproc_$create_simple_ssr
};
globaldef rpc_$epv_t rproc_$server_epv=(rpc_$epv_t)rproc_$server_epva;
