/*
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/zephyr/server/zstring.h,v $
 *	$Id: zstring.h,v 1.4 1997-09-14 21:54:52 ghudson Exp $
 *	$Author: ghudson $
 */

#include <zephyr/mit-copyright.h>

#ifndef __zstring_h
#define __zstring_h __FILE__

#define STRING_HASH_TABLE_SIZE	1024

#include <stdio.h>

typedef struct _String
{
  char *string;				/* the string itself */
  int ref_count;			/* for gc */
  unsigned long hash_val;		/* hash value for this string */
  struct _String *next, *prev;		/* for linking in hash table */
} String;

String *make_string __P((char *s, int downcase));
void free_string __P((String *z));
String *find_string __P((char *s, int downcase));
String *dup_string __P((String *z));
int comp_string __P((String *a, String *b));
void print_string_table __P((FILE *f));

#endif /* __zstring_h */

