/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains definitions used by the sorting routines.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/sort.h,v $
 *	$Id: sort.h,v 1.3 1991-04-08 21:00:58 lwvanels Exp $
 *	$Author: lwvanels $
 */

#include <mit-copyright.h>

#ifndef __olc_sort_h
#define __olc_sort_h

#include <olc/lang.h>

#define sort_key__none			0
#define sort_key__user_name		1
#define sort_key__consultant_name	2
#define sort_key__time			3
#define sort_key__question_status	4
#define sort_key__topic			5
#define sort_key__nseen			6
#define sort_key__instance		7
#define sort_key__connected_consultant	8
#define sort_key__foo			9

typedef struct {
    short	key;
    short	reversed;
} sort_keys;

#endif
