/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GPA_OPTION_H__
#define __GPA_OPTION_H__

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

#define GPA_TYPE_OPTION (gpa_option_get_type ())
#define GPA_OPTION(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_OPTION, GPAOption))
#define GPA_OPTION_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GPA_TYPE_OPTION, GPAOptionClass))
#define GPA_IS_OPTION(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_OPTION))
#define GPA_IS_OPTION_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GPA_TYPE_OPTION))
#define GPA_OPTION_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GPA_TYPE_OPTION, GPAOptionClass))

typedef struct _GPAOption GPAOption;
typedef struct _GPAOptionClass GPAOptionClass;

#include <libxml/tree.h>
#include "gpa-list.h"

typedef enum {
	GPA_OPTION_TYPE_NONE,
	GPA_OPTION_TYPE_NODE,
	GPA_OPTION_TYPE_KEY,
	GPA_OPTION_TYPE_LIST,
	GPA_OPTION_TYPE_ITEM,
	GPA_OPTION_TYPE_STRING
} GPAOptionType;

struct _GPAOption {
	GPANode node;
	GPAOptionType type;
	GPANode *name;
	GPANode *children;
	guchar *value;
};

struct _GPAOptionClass {
	GPANodeClass node_class;

	/* Creates key - i.e. settings-side items from given option */
	GPANode * (* create_key) (GPAOption *option);
};

GType gpa_option_get_type (void);

GPANode *gpa_option_new_from_tree (xmlNodePtr tree);

/* GPAOptionList */

GPAList *gpa_option_list_new_from_tree (xmlNodePtr tree);

/* Public interface */

GPANode *gpa_option_create_key (GPAOption *option);

#define GPA_OPTION_IS_NODE(o) (o && GPA_IS_OPTION (o) && (GPA_OPTION (o)->type == GPA_OPTION_TYPE_NODE))
#define GPA_OPTION_IS_KEY(o) (o && GPA_IS_OPTION (o) && (GPA_OPTION (o)->type == GPA_OPTION_TYPE_KEY))
#define GPA_OPTION_IS_LIST(o) (o && GPA_IS_OPTION (o) && (GPA_OPTION (o)->type == GPA_OPTION_TYPE_LIST))
#define GPA_OPTION_IS_ITEM(o) (o && GPA_IS_OPTION (o) && (GPA_OPTION (o)->type == GPA_OPTION_TYPE_ITEM))
#define GPA_OPTION_IS_STRING(o) (o && GPA_IS_OPTION (o) && (GPA_OPTION (o)->type == GPA_OPTION_TYPE_STRING))

/* Strictly private helpers */

GPAOption *gpa_option_get_child_by_id (GPAOption *option, const guchar *id);

/* Contructors etc. */
/* fixme: These can probably be implemented via some abstract parent class */

#define GPA_OPTION_NODE(n) GPA_OPTION (n)
#define GPA_OPTION_LIST(n) GPA_OPTION (n)
#define GPA_OPTION_ITEM(n) GPA_OPTION (n)
#define GPA_OPTION_STRING(n) GPA_OPTION (n)
#define GPA_OPTION_KEY(n) GPA_OPTION (n)

#define GPAOptionNode GPAOption
#define GPAOptionList GPAOption
#define GPAOptionItem GPAOption
#define GPAOptionKey GPAOption

GPANode *gpa_option_node_new (const guchar *id);
GPANode *gpa_option_list_new (const guchar *id);
GPANode *gpa_option_item_new (const guchar *id, const guchar *name);
GPANode *gpa_option_string_new (const guchar *id, const guchar *value);
GPANode *gpa_option_key_new (const guchar *id, const guchar *value);

gboolean gpa_option_node_append_child (GPAOptionNode *option, GPAOption *child);
gboolean gpa_option_list_append_child (GPAOptionList *option, GPAOption *child);
gboolean gpa_option_item_append_child (GPAOptionItem *option, GPAOption *child);
gboolean gpa_option_key_append_child (GPAOptionKey *option, GPAOption *child);

G_END_DECLS

#endif /* __GPA_OPTION_H__ */




