#define EXTERN extern
#include "pdftexd.h"

#define flushstring() do {                                 \
	strptr--;                                              \
	poolptr = strstart[strptr];                            \
} while (0)

#define objinfo(n) objtab[n].int0

#define nozip       0
#define zipwritting 1
#define zipfinish   2
#define pdf1bp      65782
