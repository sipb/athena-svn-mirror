/*	args.c, dds/ls/ls_admin, pato, 11/05/86
 *	Command argument tokenizer
 *
 * ========================================================================== 
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 * 
 * Apollo Computer Inc. reserves all rights, title and interest with respect
 * to copying, modification or the distribution of such software programs
 * and associated documentation, except those rights specifically granted
 * by Apollo in a Product Software Program License, Source Code License
 * or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
 * Apollo and Licensee.  Without such license agreements, such software
 * programs may not be used, copied, modified or distributed in source
 * or object code form.  Further, the copyright notice must appear on the
 * media, the supporting documentation and packaging as set forth in such
 * agreements.  Such License Agreements do not grant any rights to use
 * Apollo Computer's name or trademarks in advertising or publicity, with
 * respect to the distribution of the software programs without the specific
 * prior written permission of Apollo.  Trademark agreements may be obtained
 * in a separate Trademark License Agreement.
 * ========================================================================== 
 *
 *	This file contains hard tabs - use DM commmand: ts 1 9 -r
 *
 */

#include "std.h"
#include "lb_args.h"
#include <ctype.h>

#define TRUE 1
#define FALSE 0

static char *buf = NULL;
static char buf_start[512];
static char **buf_ptr = NULL;
static char *buf_ptr_start[256];

static start_str()
{
	buf = buf_start;
	buf_ptr = buf_ptr_start;
}	

static savestr(str)
	char *str;
{
	int alloc_len = strlen(str) + 1;

	(void) strcpy(buf, str);
	*buf_ptr++ = buf;
	buf += alloc_len;
}

static char *dostring(buf)
   char *buf;
{
	char *start = buf++;

	while (*buf != '"') {
		if (*buf == '\\') buf++;
		if (*buf == '\0') break;
		buf++;
	}
	/* get rid of opening quote - by sliding string over */
	if (*buf == '"') {
		*buf = '\0';
		strcpy(start, start+1);
		strcpy(buf-1, buf+1);
		buf -= 2;
	} else {
		strcpy(start, start+1);
	}
		
	return buf;
}


void args_$get(buf, argc, argv)
   char *buf;
   int *argc;
   char ***argv;
{
	char *start;
	char t;

	start_str();	
	*argc = 0;
	*argv = buf_ptr;
	while (*buf != '\0') {
		while (isspace(*buf)) buf++;
		if (*buf == '\0')
			break;
		start = buf;
		while (!isspace(*buf)) {
			if (*buf == '"')
				buf = dostring(buf);
			if (*buf == '\\')
				buf++;
			if (*buf == '\0')
				break;
			buf++;
		}
		t = *buf;
		*buf = '\0';
                savestr(start);
		(*argc)++;
		if (t == '\0')
			break;
		*buf++ = t;
	}
	*buf_ptr = NULL;
}

