/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GPA_LIST_H__
#define __GPA_LIST_H__

/*
 * This file is part of libgnomeprint 2
 *
 * Libgnomeprint is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Libgnomeprint is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the libgnomeprint; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors :
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#define GPA_TYPE_LIST (gpa_list_get_type ())
#define GPA_LIST(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_LIST, GPAList))
#define GPA_LIST_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GPA_TYPE_LIST, GPAListClass))
#define GPA_IS_LIST(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_LIST))
#define GPA_IS_LIST_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GPA_TYPE_LIST))
#define GPA_LIST_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GPA_TYPE_LIST, GPAListClass))

#include "gpa-node-private.h"

/* GPAList */

typedef struct _GPAList GPAList;
typedef struct _GPAListClass GPAListClass;

struct _GPAList {
	GPANode node;
	GType childtype;
	GPANode *children;
	guint has_def : 1;
	GPANode *def;
};

struct _GPAListClass {
	GPANodeClass node_class;
};

GType gpa_list_get_type (void);

GPAList *gpa_list_construct (GPAList *list, GType childtype, gboolean has_default);

gboolean gpa_list_set_default (GPAList *list, GPANode *def);

/* Debugging */
gint     gpa_list_get_length (GPAList *list);

/* Deprecated ... probably */
GPANode *gpa_list_new (GType childtype, gboolean has_def);
/* Deprecated ... probably */
gboolean gpa_list_add_child (GPAList *list, GPANode *child, GPANode *ref);

G_END_DECLS

#endif
