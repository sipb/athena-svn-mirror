#ifndef time_base_included
#define time_base_included
#include "idl_base.h"
typedef ndr_$ulong_int time_$clockh_t;
typedef struct time_$clock_t time_$clock_t;
struct time_$clock_t {
time_$clockh_t high;
ndr_$ushort_int low;
};
#endif
