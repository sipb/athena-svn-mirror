/* $Id: stringlist.h,v 1.1 1998-03-17 03:44:04 cfields Exp $ */

/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

typedef struct {
  int alloced, length;
  char **strings;
} string_list;

int athdir__add_string(string_list **list, char *string, int front);
int athdir__parse_string(string_list **list, char *string, char sep);
char **athdir__make_string_array(string_list **list);
void athdir__free_string_array(char **array);
