/* $Id: attach.h,v 1.27 1999-03-23 18:24:38 danw Exp $ */

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

int add_main(int argc, char **argv);
int attach_main(int argc, char **argv);
int attachandrun_main(int argc, char **argv);
int detach_main(int argc, char **argv);
int fsid_main(int argc, char **argv);
int zinit_main(int argc, char **argv);

extern char *whoami;
