#ifndef STRING_UTILS_H__
#define STRING_UTILS_H__

#include "xdvi-config.h"

extern Boolean str_is_prefix ARGS((char *, char *));
extern Boolean str_is_postfix ARGS((char *, char *));
extern int length_of_int ARGS((int));
/* extern int length_of_long ARGS((long)); */
extern int length_of_ulong ARGS((unsigned long));
extern char *normalize_and_expand_filename ARGS((char *));

#endif /* STRING_UTILS_H__ */
