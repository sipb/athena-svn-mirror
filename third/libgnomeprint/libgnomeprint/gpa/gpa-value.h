/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GPA_VALUE_H__
#define __GPA_VALUE_H__

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
 *   Jose M. Celorio <chema@ximian.com>
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2000-2001 Ximian, Inc. and Jose M. Celorio
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#define GPA_TYPE_VALUE (gpa_value_get_type ())
#define GPA_VALUE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_VALUE, GPAValue))
#define GPA_VALUE_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GPA_TYPE_VALUE, GPAValueClass))
#define GPA_IS_VALUE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_VALUE))
#define GPA_IS_VALUE_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GPA_TYPE_VALUE))
#define GPA_VALUE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GPA_TYPE_VALUE, GPAValueClass))

typedef struct _GPAValue GPAValue;
typedef struct _GPAValueClass GPAValueClass;

#include <libxml/tree.h>
#include "gpa-node-private.h"

struct _GPAValue {
	GPANode node;
	guchar *value;
};

struct _GPAValueClass {
	GPANodeClass node_class;
};

#define GPA_VALUE_VALUE(v) ((v) ? GPA_VALUE (v)->value : NULL)

GType gpa_value_get_type (void);

GPANode *gpa_value_new (const guchar *id, const guchar *content);

GPANode *gpa_value_new_from_tree (const guchar *id, xmlNodePtr tree);

gboolean gpa_value_set_value_forced (GPAValue *value, const guchar *val);

G_END_DECLS

#endif
