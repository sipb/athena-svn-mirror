/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

/*
 * $Revision: 1.1.1.1 $
 */

/*
 * Master Configuration header file
 *
 * All compliant .c files should include this file.  It will always 
 * include appropriate files for the MC product to define things like
 * IS_POSIX_SOURCE and whatnot.
 */
#include <stdio.h>
#include "os.h"
#include "config.h"
#include "mcdefs.h"
