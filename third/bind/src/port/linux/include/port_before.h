#undef WANT_IRS_NIS
#undef WANT_IRS_PW
#define SIG_FN void

/* We grab paths.h now to avoid troubles with redefining _PATH_HEQUIV later. */
#include <paths.h>
