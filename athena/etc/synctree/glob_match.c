/*
 * glob_match.c:  csh-style regexp pattern matcher.
 *
 * Copyright 1989 by Richard P. Basch
 */

#define regexp(pattern,string)	glob_match(pattern,string)
#include "regexp.c"
