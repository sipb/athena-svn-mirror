/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-1999, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: globmatch.h,v 1.2 2001-03-07 01:20:08 ghudson Exp $
 ***************************************************************************/



#ifndef _GLOBMATCH_H_
#define _GLOBMATCH_H_ 1

/* PROTOTYPES */
int glob_pattern( char *pattern, const char *str );
int Globmatch( char *pattern, const char *str );
int Globmatch_list( struct line_list *l, char *str );

#endif
