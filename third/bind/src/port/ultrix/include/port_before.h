#define WANT_IRS_NIS
#undef WANT_IRS_PW

#include <time.h>
struct timespec {
	time_t	tv_sec;		/* seconds */
	long	tv_nsec;	/* nanoseconds */
};
