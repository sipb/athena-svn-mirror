#define NIDL_GENERATED
#define NIDL_CSTUB
#include "rrpc.h"
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

static void rrpc_$are_you_there_csr
#ifdef __STDC__
 (
  /* [in] */handle_t h_,
  /* [out] */status_$t *st_)
#else
 (h_, st_)
handle_t h_;
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
ndr_$ushort_int data_offset;
ndr_$ulong_int bound;
rpc_$mp_t mp;
rpc_$mp_t dbp;
ndr_$ushort_int count;
ip= &ins;

/* runtime call */
ilen=0;
op= &outs;
rpc_$sar(h_,
 (long)0+rpc_$idempotent,
 &rrpc_$if_spec,
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
data_offset=h_->data_offset;
rpc_$init_mp(mp, dbp, routs, data_offset);
if (rpc_$equal_drep (drep, rpc_$local_drep)) {

/* unmarshalling */
rpc_$unmarshall_long_int(mp, (*st_).all);
} else {
rpc_$convert_long_int(drep, rpc_$local_drep, mp, (*st_).all);
}

/* buffer deallocation */
if(free_outs) rpc_$free_pkt(routs);
}

static void rrpc_$inq_stats_csr
#ifdef __STDC__
 (
  /* [in] */handle_t h_,
  /* [in] */ndr_$ulong_int max_stats_,
  /* [out] */rrpc_$stat_vec_t stats_,
  /* [out] */ndr_$long_int *l_stat_,
  /* [out] */status_$t *st_)
#else
 (h_, max_stats_, stats_, l_stat_, st_)
handle_t h_;
ndr_$ulong_int max_stats_;
rrpc_$stat_vec_t stats_;
ndr_$long_int *l_stat_;
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
ndr_$ushort_int xXx_49d_ /* stats_cv_ */ ;
ndr_$long_int *xXx_15cd_ /* stats_epe_ */ ;
ndr_$ulong_int xXx_1742_ /* stats_i_ */ ;
cleanup_status = pfm_$cleanup (&cleanup_rec);
if (cleanup_status.all != pfm_$cleanup_set) {
    if (free_ins) rpc_$free_pkt(ip);
    pfm_$signal (cleanup_status);
    }


/* marshalling init */
data_offset=h_->data_offset;
bound=0;

/* bound calculations */
bound += 4;

/* buffer allocation */
if(free_ins=(bound+data_offset>sizeof(rpc_$ppkt_t)))
    ip=rpc_$alloc_pkt(bound);
else 
    ip= &ins;
rpc_$init_mp(mp, dbp, ip, data_offset);

/* marshalling */
rpc_$marshall_ulong_int(mp, max_stats_);
rpc_$advance_mp(mp, 4);

/* runtime call */
ilen=mp-dbp;
op= &outs;
rpc_$sar(h_,
 (long)0+rpc_$idempotent,
 &rrpc_$if_spec,
 1L,
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
count = (max_stats_-0+1);
rpc_$advance_mp(mp, 2);
rpc_$unmarshall_ushort_int(mp, xXx_49d_);
rpc_$advance_mp(mp, 2);
if (xXx_49d_>count) SIGNAL(nca_status_$invalid_bound);
#ifdef ALIGNED_SCALAR_ARRAYS
rpc_$align_ptr_relative (mp, dbp, 4);
xXx_15cd_ = &stats_[0];
rpc_$block_copy((rpc_$byte_p_t)mp, (rpc_$byte_p_t)xXx_15cd_, (ndr_$ulong_int) (xXx_49d_*4));
rpc_$advance_mp (mp, (xXx_49d_*4));
#else
xXx_15cd_ = &stats_[0];
for (xXx_1742_=xXx_49d_; xXx_1742_; xXx_1742_--){
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$unmarshall_long_int(mp, (*xXx_15cd_));
rpc_$advance_mp(mp, 4);
xXx_15cd_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$unmarshall_long_int(mp, (*l_stat_));
rpc_$advance_mp(mp, 4);
rpc_$unmarshall_long_int(mp, (*st_).all);
} else {
count = (max_stats_-0+1);
rpc_$advance_mp(mp, 2);
rpc_$convert_ushort_int (drep, rpc_$local_drep, mp, xXx_49d_);
rpc_$advance_mp(mp, 2);
if (xXx_49d_>count) SIGNAL(nca_status_$invalid_bound);
xXx_15cd_ = &stats_[0];
for (xXx_1742_=xXx_49d_; xXx_1742_; xXx_1742_--){
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$convert_long_int(drep, rpc_$local_drep, mp, (*xXx_15cd_));
rpc_$advance_mp(mp, 4);
xXx_15cd_++;
}
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$convert_long_int(drep, rpc_$local_drep, mp, (*l_stat_));
rpc_$advance_mp(mp, 4);
rpc_$convert_long_int(drep, rpc_$local_drep, mp, (*st_).all);
}

/* buffer deallocation */
if(free_outs) rpc_$free_pkt(routs);
if(free_ins) rpc_$free_pkt(ip);
pfm_$rls_cleanup (&cleanup_rec, &cleanup_status);

}

static void rrpc_$inq_interfaces_csr
#ifdef __STDC__
 (
  /* [in] */handle_t h_,
  /* [in] */ndr_$ulong_int max_ifs_,
  /* [out] */rrpc_$interface_vec_t ifs_,
  /* [out] */ndr_$long_int *l_if_,
  /* [out] */status_$t *st_)
#else
 (h_, max_ifs_, ifs_, l_if_, st_)
handle_t h_;
ndr_$ulong_int max_ifs_;
rrpc_$interface_vec_t ifs_;
ndr_$long_int *l_if_;
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
ndr_$ushort_int xXx_1110_ /* ifs_el_id_host_cv_ */ ;
ndr_$byte *xXx_1083_ /* ifs_el_id_host_epe_ */ ;
ndr_$ulong_int xXx_18e5_ /* ifs_el_id_host_i_ */ ;
ndr_$ushort_int xXx_546_ /* ifs_el_port_cv_ */ ;
ndr_$ushort_int *xXx_8a_ /* ifs_el_port_epe_ */ ;
ndr_$ulong_int xXx_1552_ /* ifs_el_port_i_ */ ;
ndr_$ushort_int xXx_1d92_ /* ifs_cv_ */ ;
rpc_$if_spec_t *xXx_1e25_ /* ifs_epe_ */ ;
ndr_$ulong_int xXx_df2_ /* ifs_i_ */ ;
cleanup_status = pfm_$cleanup (&cleanup_rec);
if (cleanup_status.all != pfm_$cleanup_set) {
    if (free_ins) rpc_$free_pkt(ip);
    pfm_$signal (cleanup_status);
    }


/* marshalling init */
data_offset=h_->data_offset;
bound=0;

/* bound calculations */
bound += 4;

/* buffer allocation */
if(free_ins=(bound+data_offset>sizeof(rpc_$ppkt_t)))
    ip=rpc_$alloc_pkt(bound);
else 
    ip= &ins;
rpc_$init_mp(mp, dbp, ip, data_offset);

/* marshalling */
rpc_$marshall_ulong_int(mp, max_ifs_);
rpc_$advance_mp(mp, 4);

/* runtime call */
ilen=mp-dbp;
op= &outs;
rpc_$sar(h_,
 (long)0+rpc_$idempotent,
 &rrpc_$if_spec,
 2L,
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
count = (max_ifs_-0+1);
rpc_$advance_mp(mp, 2);
rpc_$unmarshall_ushort_int(mp, xXx_1d92_);
rpc_$advance_mp(mp, 2);
if (xXx_1d92_>count) SIGNAL(nca_status_$invalid_bound);
xXx_1e25_ = &ifs_[0];
for (xXx_df2_=xXx_1d92_; xXx_df2_; xXx_df2_--){
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$unmarshall_ulong_int(mp, (*xXx_1e25_).vers);
rpc_$advance_mp(mp, 4);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_8a_ = &(*xXx_1e25_).port[0];
rpc_$block_copy((rpc_$byte_p_t)mp, (rpc_$byte_p_t)xXx_8a_, (ndr_$ulong_int) (32*2));
rpc_$advance_mp (mp, (32*2));
#else
xXx_8a_ = &(*xXx_1e25_).port[0];
for (xXx_1552_=32; xXx_1552_; xXx_1552_--){
rpc_$unmarshall_ushort_int(mp, (*xXx_8a_));
rpc_$advance_mp(mp, 2);
xXx_8a_++;
}
#endif
rpc_$unmarshall_ushort_int(mp, (*xXx_1e25_).opcnt);
rpc_$advance_mp(mp, 2);
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$unmarshall_ulong_int(mp, (*xXx_1e25_).id.time_high);
rpc_$advance_mp(mp, 4);
rpc_$unmarshall_ushort_int(mp, (*xXx_1e25_).id.time_low);
rpc_$advance_mp(mp, 2);
rpc_$unmarshall_ushort_int(mp, (*xXx_1e25_).id.reserved);
rpc_$advance_mp(mp, 2);
rpc_$unmarshall_byte(mp, (*xXx_1e25_).id.family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_1083_ = &(*xXx_1e25_).id.host[0];
rpc_$block_copy((rpc_$byte_p_t)mp, (rpc_$byte_p_t)xXx_1083_, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_1083_ = &(*xXx_1e25_).id.host[0];
for (xXx_18e5_=7; xXx_18e5_; xXx_18e5_--){
rpc_$unmarshall_byte(mp, (*xXx_1083_));
rpc_$advance_mp(mp, 1);
xXx_1083_++;
}
#endif
xXx_1e25_++;
}
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$unmarshall_long_int(mp, (*l_if_));
rpc_$advance_mp(mp, 4);
rpc_$unmarshall_long_int(mp, (*st_).all);
} else {
count = (max_ifs_-0+1);
rpc_$advance_mp(mp, 2);
rpc_$convert_ushort_int (drep, rpc_$local_drep, mp, xXx_1d92_);
rpc_$advance_mp(mp, 2);
if (xXx_1d92_>count) SIGNAL(nca_status_$invalid_bound);
xXx_1e25_ = &ifs_[0];
for (xXx_df2_=xXx_1d92_; xXx_df2_; xXx_df2_--){
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, (*xXx_1e25_).vers);
rpc_$advance_mp(mp, 4);
xXx_8a_ = &(*xXx_1e25_).port[0];
for (xXx_1552_=32; xXx_1552_; xXx_1552_--){
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, (*xXx_8a_));
rpc_$advance_mp(mp, 2);
xXx_8a_++;
}
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, (*xXx_1e25_).opcnt);
rpc_$advance_mp(mp, 2);
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, (*xXx_1e25_).id.time_high);
rpc_$advance_mp(mp, 4);
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, (*xXx_1e25_).id.time_low);
rpc_$advance_mp(mp, 2);
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, (*xXx_1e25_).id.reserved);
rpc_$advance_mp(mp, 2);
rpc_$convert_byte(drep, rpc_$local_drep, mp, (*xXx_1e25_).id.family);
rpc_$advance_mp(mp, 1);
xXx_1083_ = &(*xXx_1e25_).id.host[0];
for (xXx_18e5_=7; xXx_18e5_; xXx_18e5_--){
rpc_$convert_byte(drep, rpc_$local_drep, mp, (*xXx_1083_));
rpc_$advance_mp(mp, 1);
xXx_1083_++;
}
xXx_1e25_++;
}
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$convert_long_int(drep, rpc_$local_drep, mp, (*l_if_));
rpc_$advance_mp(mp, 4);
rpc_$convert_long_int(drep, rpc_$local_drep, mp, (*st_).all);
}

/* buffer deallocation */
if(free_outs) rpc_$free_pkt(routs);
if(free_ins) rpc_$free_pkt(ip);
pfm_$rls_cleanup (&cleanup_rec, &cleanup_status);

}

static void rrpc_$shutdown_csr
#ifdef __STDC__
 (
  /* [in] */handle_t h_,
  /* [out] */status_$t *st_)
#else
 (h_, st_)
handle_t h_;
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
ndr_$ushort_int data_offset;
ndr_$ulong_int bound;
rpc_$mp_t mp;
rpc_$mp_t dbp;
ndr_$ushort_int count;
ip= &ins;

/* runtime call */
ilen=0;
op= &outs;
rpc_$sar(h_,
 (long)0,
 &rrpc_$if_spec,
 3L,
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
data_offset=h_->data_offset;
rpc_$init_mp(mp, dbp, routs, data_offset);
if (rpc_$equal_drep (drep, rpc_$local_drep)) {

/* unmarshalling */
rpc_$unmarshall_long_int(mp, (*st_).all);
} else {
rpc_$convert_long_int(drep, rpc_$local_drep, mp, (*st_).all);
}

/* buffer deallocation */
if(free_outs) rpc_$free_pkt(routs);
}
globaldef rrpc_$epv_t rrpc_$client_epv = {
rrpc_$are_you_there_csr,
rrpc_$inq_stats_csr,
rrpc_$inq_interfaces_csr,
rrpc_$shutdown_csr
};
