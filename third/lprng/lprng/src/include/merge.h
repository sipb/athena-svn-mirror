/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: merge.h,v 1.1.1.4 2000-03-31 15:48:08 mwhitson Exp $
 ***************************************************************************/


#ifndef _MERGE_H_
#define _MERGE_H_ 1

/* PROTOTYPES */

int  Mergesort(void *, size_t, size_t,
        int (*)(const void *, const void *));

#endif
