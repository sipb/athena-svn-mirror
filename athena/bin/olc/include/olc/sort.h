/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains definitions used by the sorting routines.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/sort.h,v $
 * $Id: sort.h,v 1.1 1990-04-27 12:04:21 vanharen Exp $
 */

#ifndef __olc_sort_h
#define __olc_sort_h

#include <olc/lang.h>

#if 0
enum sort_key {
    sort_key__none=1,
    sort_key__user_name,
    sort_key__consultant_name,
    sort_key__time,
    sort_key__question_status,
    sort_key__topic,
    sort_key__nseen,
    sort_key__instance,
    sort_key__connected_consultant, /* puts unconnected consultants last */
    sort_key__foo
};

typedef struct {
    enum sort_key	key : 4;
    unsigned int	reversed : 1;
} sort_keys;

#endif

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
