/* $Id: Snoop.h,v 1.1 1997-02-25 19:09:17 ghudson Exp $ */

/* Copyright 1997 by the Massachusetts Institute of Technology.
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

#ifndef XJ_SNOOP_H
#define XJ_SNOOP_H

#include <Jets.h>

extern JetClass snoop_jet_class;

typedef struct {int littlefoo;} Snoop_class_part;

typedef struct snoop_class_rec {
  CoreClassPart		core_class;
  Snoop_class_part	snoop_class;
} Snoop_class_rec;

extern Snoop_class_rec snoop_class_rec;

#define MAX_FMTS 32

typedef struct {
  XjCallback	*event_proc;
} Snoop_part;

typedef struct snoop_rec {
  CorePart	core;
  Snoop_part	snoop;
} Snoop_rec;

typedef struct snoop_rec *Snoop_jet;
typedef struct snoop_class_rec *Snoop_jet_class;

#define XjNeventProc "eventProc"
#define XjCEventProc "EventProc"

#endif /* XJ_SNOOP_H */
