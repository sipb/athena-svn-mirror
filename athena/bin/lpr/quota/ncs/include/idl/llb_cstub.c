#define NIDL_GENERATED
#define NIDL_CSTUB
#include "llb.h"
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

static void llb_$insert_csr
#ifdef __STDC__
 (
  /* [in] */handle_t h_,
  /* [in] */lb_$entry_t *xentry_,
  /* [out] */status_$t *status_)
#else
 (h_, xentry_, status_)
handle_t h_;
lb_$entry_t *xentry_;
status_$t *status_;
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
ndr_$ushort_int xXx_11dd_ /* xentry_saddr_data_cv_ */ ;
ndr_$byte *xXx_e3a_ /* xentry_saddr_data_epe_ */ ;
ndr_$ulong_int xXx_101d_ /* xentry_saddr_data_i_ */ ;
ndr_$ushort_int xXx_1417_ /* xentry_annotation_cv_ */ ;
ndr_$char *xXx_19f_ /* xentry_annotation_epe_ */ ;
ndr_$ulong_int xXx_489_ /* xentry_annotation_i_ */ ;
ndr_$ushort_int xXx_17b9_ /* xentry_obj_interface_host_cv_ */ ;
ndr_$byte *xXx_1c02_ /* xentry_obj_interface_host_epe_ */ ;
ndr_$ulong_int xXx_187c_ /* xentry_obj_interface_host_i_ */ ;
ndr_$ushort_int xXx_1f0_ /* xentry_obj_type_host_cv_ */ ;
ndr_$byte *xXx_1865_ /* xentry_obj_type_host_epe_ */ ;
ndr_$ulong_int xXx_1141_ /* xentry_obj_type_host_i_ */ ;
ndr_$ushort_int xXx_f3b_ /* xentry_object_host_cv_ */ ;
ndr_$byte *xXx_155_ /* xentry_object_host_epe_ */ ;
ndr_$ulong_int xXx_c28_ /* xentry_object_host_i_ */ ;
cleanup_status = pfm_$cleanup (&cleanup_rec);
if (cleanup_status.all != pfm_$cleanup_set) {
    if (free_ins) rpc_$free_pkt(ip);
    pfm_$signal (cleanup_status);
    }


/* marshalling init */
data_offset=h_->data_offset;
bound=0;

/* bound calculations */
bound += 148;

/* buffer allocation */
if(free_ins=(bound+data_offset>sizeof(rpc_$ppkt_t)))
    ip=rpc_$alloc_pkt(bound);
else 
    ip= &ins;
rpc_$init_mp(mp, dbp, ip, data_offset);

/* marshalling */
rpc_$marshall_ulong_int(mp, (*xentry_).object.time_high);
rpc_$advance_mp(mp, 4);
rpc_$marshall_ushort_int(mp, (*xentry_).object.time_low);
rpc_$advance_mp(mp, 2);
rpc_$marshall_ushort_int(mp, (*xentry_).object.reserved);
rpc_$advance_mp(mp, 2);
rpc_$marshall_byte(mp, (*xentry_).object.family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_155_ = &(*xentry_).object.host[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_155_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_155_ = &(*xentry_).object.host[0];
for (xXx_c28_=7; xXx_c28_; xXx_c28_--){
rpc_$marshall_byte(mp, (*xXx_155_));
rpc_$advance_mp(mp, 1);
xXx_155_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, (*xentry_).obj_type.time_high);
rpc_$advance_mp(mp, 4);
rpc_$marshall_ushort_int(mp, (*xentry_).obj_type.time_low);
rpc_$advance_mp(mp, 2);
rpc_$marshall_ushort_int(mp, (*xentry_).obj_type.reserved);
rpc_$advance_mp(mp, 2);
rpc_$marshall_byte(mp, (*xentry_).obj_type.family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_1865_ = &(*xentry_).obj_type.host[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_1865_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_1865_ = &(*xentry_).obj_type.host[0];
for (xXx_1141_=7; xXx_1141_; xXx_1141_--){
rpc_$marshall_byte(mp, (*xXx_1865_));
rpc_$advance_mp(mp, 1);
xXx_1865_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, (*xentry_).obj_interface.time_high);
rpc_$advance_mp(mp, 4);
rpc_$marshall_ushort_int(mp, (*xentry_).obj_interface.time_low);
rpc_$advance_mp(mp, 2);
rpc_$marshall_ushort_int(mp, (*xentry_).obj_interface.reserved);
rpc_$advance_mp(mp, 2);
rpc_$marshall_byte(mp, (*xentry_).obj_interface.family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_1c02_ = &(*xentry_).obj_interface.host[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_1c02_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_1c02_ = &(*xentry_).obj_interface.host[0];
for (xXx_187c_=7; xXx_187c_; xXx_187c_--){
rpc_$marshall_byte(mp, (*xXx_1c02_));
rpc_$advance_mp(mp, 1);
xXx_1c02_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, (*xentry_).flags);
rpc_$advance_mp(mp, 4);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_19f_ = &(*xentry_).annotation[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_19f_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (64*1));
rpc_$advance_mp (mp, (64*1));
#else
xXx_19f_ = &(*xentry_).annotation[0];
for (xXx_489_=64; xXx_489_; xXx_489_--){
rpc_$marshall_char(mp, (*xXx_19f_));
rpc_$advance_mp(mp, 1);
xXx_19f_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, (*xentry_).saddr_len);
rpc_$advance_mp(mp, 4);
rpc_$marshall_ushort_int(mp, (*xentry_).saddr.family);
rpc_$advance_mp(mp, 2);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_e3a_ = &(*xentry_).saddr.data[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_e3a_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (14*1));
rpc_$advance_mp (mp, (14*1));
#else
xXx_e3a_ = &(*xentry_).saddr.data[0];
for (xXx_101d_=14; xXx_101d_; xXx_101d_--){
rpc_$marshall_byte(mp, (*xXx_e3a_));
rpc_$advance_mp(mp, 1);
xXx_e3a_++;
}
#endif

/* runtime call */
ilen=mp-dbp;
op= &outs;
rpc_$sar(h_,
 (long)0,
 &llb_$if_spec,
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
rpc_$unmarshall_long_int(mp, (*status_).all);
} else {
rpc_$convert_long_int(drep, rpc_$local_drep, mp, (*status_).all);
}

/* buffer deallocation */
if(free_outs) rpc_$free_pkt(routs);
if(free_ins) rpc_$free_pkt(ip);
pfm_$rls_cleanup (&cleanup_rec, &cleanup_status);

}

static void llb_$delete_csr
#ifdef __STDC__
 (
  /* [in] */handle_t h_,
  /* [in] */lb_$entry_t *xentry_,
  /* [out] */status_$t *status_)
#else
 (h_, xentry_, status_)
handle_t h_;
lb_$entry_t *xentry_;
status_$t *status_;
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
ndr_$ushort_int xXx_11dd_ /* xentry_saddr_data_cv_ */ ;
ndr_$byte *xXx_e3a_ /* xentry_saddr_data_epe_ */ ;
ndr_$ulong_int xXx_101d_ /* xentry_saddr_data_i_ */ ;
ndr_$ushort_int xXx_1417_ /* xentry_annotation_cv_ */ ;
ndr_$char *xXx_19f_ /* xentry_annotation_epe_ */ ;
ndr_$ulong_int xXx_489_ /* xentry_annotation_i_ */ ;
ndr_$ushort_int xXx_17b9_ /* xentry_obj_interface_host_cv_ */ ;
ndr_$byte *xXx_1c02_ /* xentry_obj_interface_host_epe_ */ ;
ndr_$ulong_int xXx_187c_ /* xentry_obj_interface_host_i_ */ ;
ndr_$ushort_int xXx_1f0_ /* xentry_obj_type_host_cv_ */ ;
ndr_$byte *xXx_1865_ /* xentry_obj_type_host_epe_ */ ;
ndr_$ulong_int xXx_1141_ /* xentry_obj_type_host_i_ */ ;
ndr_$ushort_int xXx_f3b_ /* xentry_object_host_cv_ */ ;
ndr_$byte *xXx_155_ /* xentry_object_host_epe_ */ ;
ndr_$ulong_int xXx_c28_ /* xentry_object_host_i_ */ ;
cleanup_status = pfm_$cleanup (&cleanup_rec);
if (cleanup_status.all != pfm_$cleanup_set) {
    if (free_ins) rpc_$free_pkt(ip);
    pfm_$signal (cleanup_status);
    }


/* marshalling init */
data_offset=h_->data_offset;
bound=0;

/* bound calculations */
bound += 148;

/* buffer allocation */
if(free_ins=(bound+data_offset>sizeof(rpc_$ppkt_t)))
    ip=rpc_$alloc_pkt(bound);
else 
    ip= &ins;
rpc_$init_mp(mp, dbp, ip, data_offset);

/* marshalling */
rpc_$marshall_ulong_int(mp, (*xentry_).object.time_high);
rpc_$advance_mp(mp, 4);
rpc_$marshall_ushort_int(mp, (*xentry_).object.time_low);
rpc_$advance_mp(mp, 2);
rpc_$marshall_ushort_int(mp, (*xentry_).object.reserved);
rpc_$advance_mp(mp, 2);
rpc_$marshall_byte(mp, (*xentry_).object.family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_155_ = &(*xentry_).object.host[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_155_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_155_ = &(*xentry_).object.host[0];
for (xXx_c28_=7; xXx_c28_; xXx_c28_--){
rpc_$marshall_byte(mp, (*xXx_155_));
rpc_$advance_mp(mp, 1);
xXx_155_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, (*xentry_).obj_type.time_high);
rpc_$advance_mp(mp, 4);
rpc_$marshall_ushort_int(mp, (*xentry_).obj_type.time_low);
rpc_$advance_mp(mp, 2);
rpc_$marshall_ushort_int(mp, (*xentry_).obj_type.reserved);
rpc_$advance_mp(mp, 2);
rpc_$marshall_byte(mp, (*xentry_).obj_type.family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_1865_ = &(*xentry_).obj_type.host[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_1865_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_1865_ = &(*xentry_).obj_type.host[0];
for (xXx_1141_=7; xXx_1141_; xXx_1141_--){
rpc_$marshall_byte(mp, (*xXx_1865_));
rpc_$advance_mp(mp, 1);
xXx_1865_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, (*xentry_).obj_interface.time_high);
rpc_$advance_mp(mp, 4);
rpc_$marshall_ushort_int(mp, (*xentry_).obj_interface.time_low);
rpc_$advance_mp(mp, 2);
rpc_$marshall_ushort_int(mp, (*xentry_).obj_interface.reserved);
rpc_$advance_mp(mp, 2);
rpc_$marshall_byte(mp, (*xentry_).obj_interface.family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_1c02_ = &(*xentry_).obj_interface.host[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_1c02_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_1c02_ = &(*xentry_).obj_interface.host[0];
for (xXx_187c_=7; xXx_187c_; xXx_187c_--){
rpc_$marshall_byte(mp, (*xXx_1c02_));
rpc_$advance_mp(mp, 1);
xXx_1c02_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, (*xentry_).flags);
rpc_$advance_mp(mp, 4);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_19f_ = &(*xentry_).annotation[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_19f_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (64*1));
rpc_$advance_mp (mp, (64*1));
#else
xXx_19f_ = &(*xentry_).annotation[0];
for (xXx_489_=64; xXx_489_; xXx_489_--){
rpc_$marshall_char(mp, (*xXx_19f_));
rpc_$advance_mp(mp, 1);
xXx_19f_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, (*xentry_).saddr_len);
rpc_$advance_mp(mp, 4);
rpc_$marshall_ushort_int(mp, (*xentry_).saddr.family);
rpc_$advance_mp(mp, 2);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_e3a_ = &(*xentry_).saddr.data[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_e3a_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (14*1));
rpc_$advance_mp (mp, (14*1));
#else
xXx_e3a_ = &(*xentry_).saddr.data[0];
for (xXx_101d_=14; xXx_101d_; xXx_101d_--){
rpc_$marshall_byte(mp, (*xXx_e3a_));
rpc_$advance_mp(mp, 1);
xXx_e3a_++;
}
#endif

/* runtime call */
ilen=mp-dbp;
op= &outs;
rpc_$sar(h_,
 (long)0,
 &llb_$if_spec,
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
rpc_$unmarshall_long_int(mp, (*status_).all);
} else {
rpc_$convert_long_int(drep, rpc_$local_drep, mp, (*status_).all);
}

/* buffer deallocation */
if(free_outs) rpc_$free_pkt(routs);
if(free_ins) rpc_$free_pkt(ip);
pfm_$rls_cleanup (&cleanup_rec, &cleanup_status);

}

static void llb_$lookup_csr
#ifdef __STDC__
 (
  /* [in] */handle_t h_,
  /* [in] */uuid_$t *object_,
  /* [in] */uuid_$t *obj_type_,
  /* [in] */uuid_$t *obj_interface_,
  /* [in, out] */lb_$lookup_handle_t *entry_handle_,
  /* [in] */ndr_$ulong_int max_num_results_,
  /* [out] */ndr_$ulong_int *num_results_,
  /* [out] */lb_$entry_t result_entries_[6],
  /* [out] */status_$t *status_)
#else
 (h_, object_, obj_type_, obj_interface_, entry_handle_, max_num_results_, num_results_, result_entries_, status_)
handle_t h_;
uuid_$t *object_;
uuid_$t *obj_type_;
uuid_$t *obj_interface_;
lb_$lookup_handle_t *entry_handle_;
ndr_$ulong_int max_num_results_;
ndr_$ulong_int *num_results_;
lb_$entry_t result_entries_[6];
status_$t *status_;
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
ndr_$ushort_int xXx_1b37_ /* result_entries_el_saddr_data_cv_ */ ;
ndr_$byte *xXx_7c9_ /* result_entries_el_saddr_data_epe_ */ ;
ndr_$ulong_int xXx_13c2_ /* result_entries_el_saddr_data_i_ */ ;
ndr_$ushort_int xXx_1dc7_ /* result_entries_el_annotation_cv_ */ ;
ndr_$char *xXx_1efb_ /* result_entries_el_annotation_epe_ */ ;
ndr_$ulong_int xXx_c19_ /* result_entries_el_annotation_i_ */ ;
ndr_$ushort_int xXx_b4e_ /* result_entries_el_obj_interface_host_cv_ */ ;
ndr_$byte *xXx_1154_ /* result_entries_el_obj_interface_host_epe_ */ ;
ndr_$ulong_int xXx_df3_ /* result_entries_el_obj_interface_host_i_ */ ;
ndr_$ushort_int xXx_413_ /* result_entries_el_obj_type_host_cv_ */ ;
ndr_$byte *xXx_1c72_ /* result_entries_el_obj_type_host_epe_ */ ;
ndr_$ulong_int xXx_b5c_ /* result_entries_el_obj_type_host_i_ */ ;
ndr_$ushort_int xXx_ae_ /* result_entries_el_object_host_cv_ */ ;
ndr_$byte *xXx_507_ /* result_entries_el_object_host_epe_ */ ;
ndr_$ulong_int xXx_d29_ /* result_entries_el_object_host_i_ */ ;
ndr_$ushort_int xXx_587_ /* result_entries_cv_ */ ;
lb_$entry_t *xXx_1349_ /* result_entries_epe_ */ ;
ndr_$ulong_int xXx_126e_ /* result_entries_i_ */ ;
ndr_$ushort_int xXx_1b24_ /* obj_interface_host_cv_ */ ;
ndr_$byte *xXx_56f_ /* obj_interface_host_epe_ */ ;
ndr_$ulong_int xXx_19a1_ /* obj_interface_host_i_ */ ;
ndr_$ushort_int xXx_2b_ /* obj_type_host_cv_ */ ;
ndr_$byte *xXx_e27_ /* obj_type_host_epe_ */ ;
ndr_$ulong_int xXx_19dd_ /* obj_type_host_i_ */ ;
ndr_$ushort_int xXx_1a45_ /* object_host_cv_ */ ;
ndr_$byte *xXx_1832_ /* object_host_epe_ */ ;
ndr_$ulong_int xXx_1e63_ /* object_host_i_ */ ;
cleanup_status = pfm_$cleanup (&cleanup_rec);
if (cleanup_status.all != pfm_$cleanup_set) {
    if (free_ins) rpc_$free_pkt(ip);
    pfm_$signal (cleanup_status);
    }


/* marshalling init */
data_offset=h_->data_offset;
bound=0;

/* bound calculations */
bound += 65;

/* buffer allocation */
if(free_ins=(bound+data_offset>sizeof(rpc_$ppkt_t)))
    ip=rpc_$alloc_pkt(bound);
else 
    ip= &ins;
rpc_$init_mp(mp, dbp, ip, data_offset);

/* marshalling */
rpc_$marshall_ulong_int(mp, (*object_).time_high);
rpc_$advance_mp(mp, 4);
rpc_$marshall_ushort_int(mp, (*object_).time_low);
rpc_$advance_mp(mp, 2);
rpc_$marshall_ushort_int(mp, (*object_).reserved);
rpc_$advance_mp(mp, 2);
rpc_$marshall_byte(mp, (*object_).family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_1832_ = &(*object_).host[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_1832_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_1832_ = &(*object_).host[0];
for (xXx_1e63_=7; xXx_1e63_; xXx_1e63_--){
rpc_$marshall_byte(mp, (*xXx_1832_));
rpc_$advance_mp(mp, 1);
xXx_1832_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, (*obj_type_).time_high);
rpc_$advance_mp(mp, 4);
rpc_$marshall_ushort_int(mp, (*obj_type_).time_low);
rpc_$advance_mp(mp, 2);
rpc_$marshall_ushort_int(mp, (*obj_type_).reserved);
rpc_$advance_mp(mp, 2);
rpc_$marshall_byte(mp, (*obj_type_).family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_e27_ = &(*obj_type_).host[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_e27_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_e27_ = &(*obj_type_).host[0];
for (xXx_19dd_=7; xXx_19dd_; xXx_19dd_--){
rpc_$marshall_byte(mp, (*xXx_e27_));
rpc_$advance_mp(mp, 1);
xXx_e27_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, (*obj_interface_).time_high);
rpc_$advance_mp(mp, 4);
rpc_$marshall_ushort_int(mp, (*obj_interface_).time_low);
rpc_$advance_mp(mp, 2);
rpc_$marshall_ushort_int(mp, (*obj_interface_).reserved);
rpc_$advance_mp(mp, 2);
rpc_$marshall_byte(mp, (*obj_interface_).family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_56f_ = &(*obj_interface_).host[0];
rpc_$block_copy((rpc_$byte_p_t)xXx_56f_, (rpc_$byte_p_t)mp, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_56f_ = &(*obj_interface_).host[0];
for (xXx_19a1_=7; xXx_19a1_; xXx_19a1_--){
rpc_$marshall_byte(mp, (*xXx_56f_));
rpc_$advance_mp(mp, 1);
xXx_56f_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$marshall_ulong_int(mp, (*entry_handle_));
rpc_$advance_mp(mp, 4);
rpc_$marshall_ulong_int(mp, max_num_results_);
rpc_$advance_mp(mp, 4);

/* runtime call */
ilen=mp-dbp;
op= &outs;
rpc_$sar(h_,
 (long)0+rpc_$idempotent,
 &llb_$if_spec,
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
rpc_$unmarshall_ulong_int(mp, (*entry_handle_));
rpc_$advance_mp(mp, 4);
rpc_$unmarshall_ulong_int(mp, (*num_results_));
rpc_$advance_mp(mp, 4);
rpc_$unmarshall_ushort_int(mp, xXx_587_);
rpc_$advance_mp(mp, 2);
if (xXx_587_>6) SIGNAL(nca_status_$invalid_bound);
xXx_1349_ = &result_entries_[0];
for (xXx_126e_=xXx_587_; xXx_126e_; xXx_126e_--){
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$unmarshall_ulong_int(mp, (*xXx_1349_).object.time_high);
rpc_$advance_mp(mp, 4);
rpc_$unmarshall_ushort_int(mp, (*xXx_1349_).object.time_low);
rpc_$advance_mp(mp, 2);
rpc_$unmarshall_ushort_int(mp, (*xXx_1349_).object.reserved);
rpc_$advance_mp(mp, 2);
rpc_$unmarshall_byte(mp, (*xXx_1349_).object.family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_507_ = &(*xXx_1349_).object.host[0];
rpc_$block_copy((rpc_$byte_p_t)mp, (rpc_$byte_p_t)xXx_507_, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_507_ = &(*xXx_1349_).object.host[0];
for (xXx_d29_=7; xXx_d29_; xXx_d29_--){
rpc_$unmarshall_byte(mp, (*xXx_507_));
rpc_$advance_mp(mp, 1);
xXx_507_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$unmarshall_ulong_int(mp, (*xXx_1349_).obj_type.time_high);
rpc_$advance_mp(mp, 4);
rpc_$unmarshall_ushort_int(mp, (*xXx_1349_).obj_type.time_low);
rpc_$advance_mp(mp, 2);
rpc_$unmarshall_ushort_int(mp, (*xXx_1349_).obj_type.reserved);
rpc_$advance_mp(mp, 2);
rpc_$unmarshall_byte(mp, (*xXx_1349_).obj_type.family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_1c72_ = &(*xXx_1349_).obj_type.host[0];
rpc_$block_copy((rpc_$byte_p_t)mp, (rpc_$byte_p_t)xXx_1c72_, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_1c72_ = &(*xXx_1349_).obj_type.host[0];
for (xXx_b5c_=7; xXx_b5c_; xXx_b5c_--){
rpc_$unmarshall_byte(mp, (*xXx_1c72_));
rpc_$advance_mp(mp, 1);
xXx_1c72_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$unmarshall_ulong_int(mp, (*xXx_1349_).obj_interface.time_high);
rpc_$advance_mp(mp, 4);
rpc_$unmarshall_ushort_int(mp, (*xXx_1349_).obj_interface.time_low);
rpc_$advance_mp(mp, 2);
rpc_$unmarshall_ushort_int(mp, (*xXx_1349_).obj_interface.reserved);
rpc_$advance_mp(mp, 2);
rpc_$unmarshall_byte(mp, (*xXx_1349_).obj_interface.family);
rpc_$advance_mp(mp, 1);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_1154_ = &(*xXx_1349_).obj_interface.host[0];
rpc_$block_copy((rpc_$byte_p_t)mp, (rpc_$byte_p_t)xXx_1154_, (ndr_$ulong_int) (7*1));
rpc_$advance_mp (mp, (7*1));
#else
xXx_1154_ = &(*xXx_1349_).obj_interface.host[0];
for (xXx_df3_=7; xXx_df3_; xXx_df3_--){
rpc_$unmarshall_byte(mp, (*xXx_1154_));
rpc_$advance_mp(mp, 1);
xXx_1154_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$unmarshall_ulong_int(mp, (*xXx_1349_).flags);
rpc_$advance_mp(mp, 4);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_1efb_ = &(*xXx_1349_).annotation[0];
rpc_$block_copy((rpc_$byte_p_t)mp, (rpc_$byte_p_t)xXx_1efb_, (ndr_$ulong_int) (64*1));
rpc_$advance_mp (mp, (64*1));
#else
xXx_1efb_ = &(*xXx_1349_).annotation[0];
for (xXx_c19_=64; xXx_c19_; xXx_c19_--){
rpc_$unmarshall_char(mp, (*xXx_1efb_));
rpc_$advance_mp(mp, 1);
xXx_1efb_++;
}
#endif
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$unmarshall_ulong_int(mp, (*xXx_1349_).saddr_len);
rpc_$advance_mp(mp, 4);
rpc_$unmarshall_ushort_int(mp, (*xXx_1349_).saddr.family);
rpc_$advance_mp(mp, 2);
#ifdef ALIGNED_SCALAR_ARRAYS
xXx_7c9_ = &(*xXx_1349_).saddr.data[0];
rpc_$block_copy((rpc_$byte_p_t)mp, (rpc_$byte_p_t)xXx_7c9_, (ndr_$ulong_int) (14*1));
rpc_$advance_mp (mp, (14*1));
#else
xXx_7c9_ = &(*xXx_1349_).saddr.data[0];
for (xXx_13c2_=14; xXx_13c2_; xXx_13c2_--){
rpc_$unmarshall_byte(mp, (*xXx_7c9_));
rpc_$advance_mp(mp, 1);
xXx_7c9_++;
}
#endif
xXx_1349_++;
}
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$unmarshall_long_int(mp, (*status_).all);
} else {
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, (*entry_handle_));
rpc_$advance_mp(mp, 4);
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, (*num_results_));
rpc_$advance_mp(mp, 4);
rpc_$convert_ushort_int (drep, rpc_$local_drep, mp, xXx_587_);
rpc_$advance_mp(mp, 2);
if (xXx_587_>6) SIGNAL(nca_status_$invalid_bound);
xXx_1349_ = &result_entries_[0];
for (xXx_126e_=xXx_587_; xXx_126e_; xXx_126e_--){
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, (*xXx_1349_).object.time_high);
rpc_$advance_mp(mp, 4);
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, (*xXx_1349_).object.time_low);
rpc_$advance_mp(mp, 2);
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, (*xXx_1349_).object.reserved);
rpc_$advance_mp(mp, 2);
rpc_$convert_byte(drep, rpc_$local_drep, mp, (*xXx_1349_).object.family);
rpc_$advance_mp(mp, 1);
xXx_507_ = &(*xXx_1349_).object.host[0];
for (xXx_d29_=7; xXx_d29_; xXx_d29_--){
rpc_$convert_byte(drep, rpc_$local_drep, mp, (*xXx_507_));
rpc_$advance_mp(mp, 1);
xXx_507_++;
}
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, (*xXx_1349_).obj_type.time_high);
rpc_$advance_mp(mp, 4);
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, (*xXx_1349_).obj_type.time_low);
rpc_$advance_mp(mp, 2);
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, (*xXx_1349_).obj_type.reserved);
rpc_$advance_mp(mp, 2);
rpc_$convert_byte(drep, rpc_$local_drep, mp, (*xXx_1349_).obj_type.family);
rpc_$advance_mp(mp, 1);
xXx_1c72_ = &(*xXx_1349_).obj_type.host[0];
for (xXx_b5c_=7; xXx_b5c_; xXx_b5c_--){
rpc_$convert_byte(drep, rpc_$local_drep, mp, (*xXx_1c72_));
rpc_$advance_mp(mp, 1);
xXx_1c72_++;
}
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, (*xXx_1349_).obj_interface.time_high);
rpc_$advance_mp(mp, 4);
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, (*xXx_1349_).obj_interface.time_low);
rpc_$advance_mp(mp, 2);
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, (*xXx_1349_).obj_interface.reserved);
rpc_$advance_mp(mp, 2);
rpc_$convert_byte(drep, rpc_$local_drep, mp, (*xXx_1349_).obj_interface.family);
rpc_$advance_mp(mp, 1);
xXx_1154_ = &(*xXx_1349_).obj_interface.host[0];
for (xXx_df3_=7; xXx_df3_; xXx_df3_--){
rpc_$convert_byte(drep, rpc_$local_drep, mp, (*xXx_1154_));
rpc_$advance_mp(mp, 1);
xXx_1154_++;
}
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, (*xXx_1349_).flags);
rpc_$advance_mp(mp, 4);
xXx_1efb_ = &(*xXx_1349_).annotation[0];
for (xXx_c19_=64; xXx_c19_; xXx_c19_--){
rpc_$convert_char(drep, rpc_$local_drep, mp, (*xXx_1efb_));
rpc_$advance_mp(mp, 1);
xXx_1efb_++;
}
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$convert_ulong_int(drep, rpc_$local_drep, mp, (*xXx_1349_).saddr_len);
rpc_$advance_mp(mp, 4);
rpc_$convert_ushort_int(drep, rpc_$local_drep, mp, (*xXx_1349_).saddr.family);
rpc_$advance_mp(mp, 2);
xXx_7c9_ = &(*xXx_1349_).saddr.data[0];
for (xXx_13c2_=14; xXx_13c2_; xXx_13c2_--){
rpc_$convert_byte(drep, rpc_$local_drep, mp, (*xXx_7c9_));
rpc_$advance_mp(mp, 1);
xXx_7c9_++;
}
xXx_1349_++;
}
rpc_$align_ptr_relative (mp, dbp, 4);
rpc_$convert_long_int(drep, rpc_$local_drep, mp, (*status_).all);
}

/* buffer deallocation */
if(free_outs) rpc_$free_pkt(routs);
if(free_ins) rpc_$free_pkt(ip);
pfm_$rls_cleanup (&cleanup_rec, &cleanup_status);

}
globaldef llb_$epv_t llb_$client_epv = {
llb_$insert_csr,
llb_$delete_csr,
llb_$lookup_csr
};
