/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GPA_KEY_H__
#define __GPA_KEY_H__

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
 *   Jose M. Celorio <chema@ximian.com>
 *
 * Copyright (C) 2000-2001 Ximian, Inc. and Jose M. Celorio
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#define GPA_TYPE_KEY (gpa_key_get_type ())
#define GPA_KEY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_KEY, GPAKey))
#define GPA_KEY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GPA_TYPE_KEY, GPAKeyClass))
#define GPA_IS_KEY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_KEY))
#define GPA_IS_KEY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GPA_TYPE_KEY))
#define GPA_KEY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GPA_TYPE_KEY, GPAKeyClass))

typedef struct _GPAKey GPAKey;
typedef struct _GPAKeyClass GPAKeyClass;

#include <libxml/tree.h>
#include "gpa-list.h"

/* GPAKey */

#define GPA_KEY_VALUE_IS_QUARK_SHIFT (GPA_MODIFIED_LAST_SHIFT + 1)
#define GPA_KEY_VALUE_IS_QUARK_FLAG (1 << GPA_KEY_VALUE_IS_QUARK_SHIFT)
#define GPA_KEY_LAST_SHIFT GPA_KEY_VALUE_IS_QUARK_SHIFT

struct _GPAKey {
	GPANode node;

	GPANode *children;
	GPANode *option;
	guchar *value;
};


struct _GPAKeyClass {
	GPANodeClass node_class;
};

GType gpa_key_get_type (void);

GPANode *gpa_key_new_from_option (GPANode *node);

xmlNodePtr gpa_key_write (xmlDocPtr doc, GPANode *key);

gboolean gpa_key_merge_from_tree (GPANode *key, xmlNodePtr tree);

gboolean gpa_key_merge_from_key (GPAKey *dst, GPAKey *src);

gboolean gpa_key_copy (GPANode *dst, GPANode *src);

#define GPA_KEY_ID(k) (GPA_KEY (k)->id)
#define GPA_KEY_HAS_OPTION(k) ((k) && GPA_KEY (k)->option)
#define GPA_KEY_OPTION(k) ((k) && GPA_KEY (k)->option ? GPA_OPTION (GPA_KEY (k)->option) : NULL)


G_END_DECLS

#endif /* __GPA_KEY_H__ */
