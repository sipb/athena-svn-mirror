/* $Id: agetopt.h,v 1.2 1999-03-23 18:24:00 danw Exp $ */

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

struct agetopt_option {
  const char *longname;
  char shortname;
  int arg;
};

int attach_getopt(int argc, char **argv, struct agetopt_option *options);

extern int optind;
extern char *optarg;
