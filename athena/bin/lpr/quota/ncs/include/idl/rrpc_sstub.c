#define NIDL_GENERATED
#define NIDL_SSTUB
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

static void rrpc_$are_you_there_ssr
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

/* server call */
rrpc_$are_you_there(h, &st_);
data_offset=h->data_offset;
bound=0;

/* bound calculations */
bound += 4;

/* buffer allocation */
if(*free_outs=(bound>omax))
    *routs=rpc_$alloc_pkt(bound);
else
    *routs=outs;
rpc_$init_mp(mp, dbp, *routs, data_offset);

/* marshalling */
rpc_$marshall_long_int(mp, st_.all);
rpc_$advance_mp(mp, 4);

*olen=mp-dbp;

st->all=status_$ok;
}

static void rrpc_$inq_stats_ssr
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
ndr_$long_int l_stat_;
volatile ndr_$long_int *stats_;
ndr_$ushort_int xXx_49d_ /* stats_cv_ */ ;
ndr_$long_int *xXx_15cd_ /* stats_epe_ */ ;
ndr_$ulong_int xXx_1742_ /* stats_i_ */ ;
ndr_$ulong_int max_stats_;
cleanup_status = pfm_$cleanup (&cleanup_rec);
if (cleanup_status.all != pfm_$cleanup_set) {
    if (stats_!= NULL) rpc_$free(stats_);
    pfm_$signal (cleanup_status);
    }

stats_= NULL;


/* unmarshalling init */
data_offset=h->data_offset;
rpc_$init_mp(mp, dbp, ins, data_offset);
if (rpc_$equal_drep (drep, rpc_$local_drep)) {

/* unmarshalling */
rpc_$unmarshall_ulong_int(mp, max_stats_);
} else {
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, max_stats_);
}
stats_ = (ndr_$long_int *) rpc_$malloc ((max_stats_-0+1) * sizeof(ndr_$long_int ));

/* server call */
rrpc_$inq_stats(h, max_stats_, (ndr_$long_int *)stats_, &l_stat_, &st_);
bound=0;

/* bound calculations */
bound += ((l_stat_-0+1)) * 4;
bound += 16;

/* buffer allocation */
if(*free_outs=(bound>omax))
    *routs=rpc_$alloc_pkt(bound);
else
    *routs=outs;
rpc_$init_mp(mp, dbp, *routs, data_offset);

/* marshalling */
count = (max_stats_-0+1);
rpc_$marshall_ushort_int (mp, count);
rpc_$advance_mp(mp, 2);
xXx_49d_ = (l_stat_-0+1);
rpc_$marshall_ushort_int(mp, xXx_49d_);
rpc_$advance_mp(mp, 2);
if (xXx_49d_>count) SIGNAL(nca_status_$invalid_bound);
#ifdef ALIGNED_SCALAR_ARRAYS
rpc_$align_ptr_relative (mp, dbp, 4);
xXx_15cd_ = &*stats_;
rpc_$block_copy((rpc_$byte_p_t)xXx_15cd_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (xXx_49d_*4));
rpc_$advance_mp (mp, (xXx_49d_*4));
#else
xXx_15cd_ = &*stats_;
for (xXx_1742_=xXx_49d_; xXx_1742_; xXx_1742_--){
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_long_int(mp, (*xXx_15cd_));
rpc_$advance_mp(mp, 4);
xXx_15cd_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_long_int(mp, l_stat_);
rpc_$advance_mp(mp, 4);
rpc_$marshall_long_int(mp, st_.all);
rpc_$advance_mp(mp, 4);

*olen=mp-dbp;
if (stats_ != NULL) rpc_$free((char *)stats_);
pfm_$rls_cleanup (&cleanup_rec, &cleanup_status);


st->all=status_$ok;
}

static void rrpc_$inq_interfaces_ssr
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
ndr_$long_int l_if_;
volatile rpc_$if_spec_t *ifs_;
ndr_$ushort_int xXx_1110_ /* ifs_el_id_host_cv_ */ ;
ndr_$byte *xXx_1083_ /* ifs_el_id_host_epe_ */ ;
ndr_$ulong_int xXx_18e5_ /* ifs_el_id_host_i_ */ ;
ndr_$ushort_int xXx_546_ /* ifs_el_port_cv_ */ ;
ndr_$ushort_int *xXx_8a_ /* ifs_el_port_epe_ */ ;
ndr_$ulong_int xXx_1552_ /* ifs_el_port_i_ */ ;
ndr_$ushort_int xXx_1d92_ /* ifs_cv_ */ ;
rpc_$if_spec_t *xXx_1e25_ /* ifs_epe_ */ ;
ndr_$ulong_int xXx_df2_ /* ifs_i_ */ ;
ndr_$ulong_int max_ifs_;
cleanup_status = pfm_$cleanup (&cleanup_rec);
if (cleanup_status.all != pfm_$cleanup_set) {
    if (ifs_!= NULL) rpc_$free(ifs_);
    pfm_$signal (cleanup_status);
    }

ifs_= NULL;


/* unmarshalling init */
data_offset=h->data_offset;
rpc_$init_mp(mp, dbp, ins, data_offset);
if (rpc_$equal_drep (drep, rpc_$local_drep)) {

/* unmarshalling */
rpc_$unmarshall_ulong_int(mp, max_ifs_);
} else {
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, max_ifs_);
}
ifs_ = (rpc_$if_spec_t *) rpc_$malloc ((max_ifs_-0+1) * sizeof(rpc_$if_spec_t ));

/* server call */
rrpc_$inq_interfaces(h, max_ifs_, (rpc_$if_spec_t *)ifs_, &l_if_, &st_);
bound=0;

/* bound calculations */
bound += ((l_if_-0+1)) * 91;
bound += 17;

/* buffer allocation */
if(*free_outs=(bound>omax))
    *routs=rpc_$alloc_pkt(bound);
else
    *routs=outs;
rpc_$init_mp(mp, dbp, *routs, data_offset);

/* marshalling */
count = (max_ifs_-0+1);
rpc_$marshall_ushort_int (mp, count);
rpc_$advance_mp(mp, 2);
xXx_1d92_ = (l_if_-0+1);
rpc_$marshall_ushort_int(mp, xXx_1d92_);
rpc_$advance_mp(mp, 2);
if (xXx_1d92_>count) SIGNAL(nca_status_$invalid_bound);
xXx_1e25_ = &ifs_[0];
for (xXx_df2_=xXx_1d92_; xXx_df2_; xXx_df2_--){
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, (*xXx_1e25_).vers);
rpc_$advance_mp(mp, 4);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_8a_ = &(*xXx_1e25_).port[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_8a_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (32*2));
rpc_$advance_mp (mp, (32*2));
#else
xXx_8a_ = &(*xXx_1e25_).port[0];
for (xXx_1552_=32; xXx_1552_; xXx_1552_--){
rpc_$marshall_ushort_int(mp, (*xXx_8a_));
rpc_$advance_mp(mp, 2);
xXx_8a_++;
}
#endif
rpc_$marshall_ushort_int(mp, (*xXx_1e25_).opcnt);
rpc_$advance_mp(mp, 2);
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, (*xXx_1e25_).id.time_high);
rpc_$advance_mp(mp, 4);
rpc_$marshall_ushort_int(mp, (*xXx_1e25_).id.time_low);
rpc_$advance_mp(mp, 2);
rpc_$marshall_ushort_int(mp, (*xXx_1e25_).id.reserved);
rpc_$advance_mp(mp, 2);
rpc_$marshall_byte(mp, (*xXx_1e25_).id.family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_1083_ = &(*xXx_1e25_).id.host[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_1083_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_1083_ = &(*xXx_1e25_).id.host[0];
for (xXx_18e5_=7; xXx_18e5_; xXx_18e5_--){
rpc_$marshall_byte(mp, (*xXx_1083_));
rpc_$advance_mp(mp, 1);
xXx_1083_++;
}
#endif
xXx_1e25_++;
}
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_long_int(mp, l_if_);
rpc_$advance_mp(mp, 4);
rpc_$marshall_long_int(mp, st_.all);
rpc_$advance_mp(mp, 4);

*olen=mp-dbp;
if (ifs_ != NULL) rpc_$free((char *)ifs_);
pfm_$rls_cleanup (&cleanup_rec, &cleanup_status);


st->all=status_$ok;
}

static void rrpc_$shutdown_ssr
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

/* server call */
rrpc_$shutdown(h, &st_);
data_offset=h->data_offset;
bound=0;

/* bound calculations */
bound += 4;

/* buffer allocation */
if(*free_outs=(bound>omax))
    *routs=rpc_$alloc_pkt(bound);
else
    *routs=outs;
rpc_$init_mp(mp, dbp, *routs, data_offset);

/* marshalling */
rpc_$marshall_long_int(mp, st_.all);
rpc_$advance_mp(mp, 4);

*olen=mp-dbp;

st->all=status_$ok;
}
globaldef rrpc_$epv_t rrpc_$manager_epv = {
rrpc_$are_you_there,
rrpc_$inq_stats,
rrpc_$inq_interfaces,
rrpc_$shutdown
};

static rpc_$server_stub_t rrpc_$server_epva[]={
(rpc_$server_stub_t)rrpc_$are_you_there_ssr,
(rpc_$server_stub_t)rrpc_$inq_stats_ssr,
(rpc_$server_stub_t)rrpc_$inq_interfaces_ssr,
(rpc_$server_stub_t)rrpc_$shutdown_ssr
};
globaldef rpc_$epv_t rrpc_$server_epv=(rpc_$epv_t)rrpc_$server_epva;
