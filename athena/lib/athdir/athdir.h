/* $Id: athdir.h,v 1.1 1998-03-17 03:43:09 cfields Exp $ */

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

#ifndef ATHDIR__H
#define ATHDIR__H

#define ATHDIR_SUPPRESSEDITORIALS	(1<<0)
#define ATHDIR_SUPPRESSSEARCH		(1<<1)
#define ATHDIR_MACHINEDEPENDENT		(1<<2)
#define ATHDIR_MACHINEINDEPENDENT	(1<<3)
#define ATHDIR_LISTSEARCHDIRECTORIES	(1<<4)

char **athdir_get_paths(char *base_path, char *type,
			char *sys, char **syscompat, char *machine,
			char *aux, int flags);
void athdir_free_paths(char **paths);
int athdir_native(char *what, char *sys);

#endif
