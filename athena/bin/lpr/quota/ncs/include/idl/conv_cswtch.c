#define NIDL_GENERATED
#define NIDL_CSWTCH
#include "conv.h"

void conv_$who_are_you
#ifdef __STDC__
 (
  /* [in] */handle_t h,
  /* [in] */uuid_$t *actuid,
  /* [in] */ndr_$ulong_int boot_time,
  /* [out] */ndr_$ulong_int *seq,
  /* [out] */status_$t *st)
#else
 (h, actuid, boot_time, seq, st)
handle_t h;
uuid_$t *actuid;
ndr_$ulong_int boot_time;
ndr_$ulong_int *seq;
status_$t *st;
#endif
{
(*conv_$client_epv.conv_$who_are_you)(h, actuid, boot_time, seq, st);
}
