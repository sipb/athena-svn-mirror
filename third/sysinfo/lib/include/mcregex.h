/*
 * Copyright (c) 1992-1998 by Michael A. Cooper.
 * All rights reserved.
 */

/*
 * $Revision: 1.1.1.1 $
 *
 * MagniComp Regular Expression header file
 */

#ifndef __mcregex_h__
#define __mcregex_h__

#include "mconfig.h"

/*
 * Types of regular expression functions
 */
#define RE_REGCOMP		1		/* POSIX regcomp() */
#define RE_COMP			2		/* BSD re_comp() */
#define RE_REGCMP		3		/* SYSV regcmp() */

#endif	/* __mcregex_h__ */
