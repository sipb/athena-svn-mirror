/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-state.h: 
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Copyright (C) 2004 Red Hat, Inc.
 *
 */

#ifndef __GPA_STATE_H__
#define __GPA_STATE_H__

#include <glib.h>
#include "gpa-node.h"

G_BEGIN_DECLS

#define GPA_TYPE_STATE         (gpa_state_get_type ())
#define GPA_STATE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_STATE, GPAState))
#define GPA_IS_STATE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_STATE))

#define GPA_STATE_ID(k)         (GPA_STATE (k)->id)

typedef struct _GPAState      GPAState;
typedef struct _GPAStateClass GPAStateClass;

struct _GPAState {
	GPANode node;

	char *value;
};

struct _GPAStateClass {
	GPANodeClass node_class;
};

GType gpa_state_get_type (void);

GPAState * gpa_state_new (const char *id);

G_END_DECLS

#endif /* __GPA_STATE_H__ */
