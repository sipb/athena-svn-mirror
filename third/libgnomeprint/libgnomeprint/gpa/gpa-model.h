/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GPA_MODEL_H__
#define __GPA_MODEL_H__

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

#define GPA_TYPE_MODEL (gpa_model_get_type ())
#define GPA_MODEL(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_MODEL, GPAModel))
#define GPA_MODEL_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GPA_TYPE_MODEL, GPAModelClass))
#define GPA_IS_MODEL(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_MODEL))
#define GPA_IS_MODEL_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GPA_TYPE_MODEL))
#define GPA_MODEL_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GPA_TYPE_MODEL, GPAModelClass))

typedef struct _GPAModel GPAModel;
typedef struct _GPAModelClass GPAModelClass;

#include <libxml/tree.h>
#include "gpa-list.h"

/* GPAModel */

struct _GPAModel {
	GPANode node;
	
	guint loaded : 1;

	guchar *vendorid;

	GPANode *name;
	GPANode *vendor;
	GPANode *options;
};

struct _GPAModelClass {
	GPANodeClass node_class;
};

GType gpa_model_get_type (void);

GPANode *gpa_model_new_from_info_tree (xmlNodePtr tree);
GPANode *gpa_model_new_from_tree (xmlNodePtr tree);

GPAList *gpa_model_list_new_from_info_tree (xmlNodePtr tree);

GPANode *gpa_model_get_by_id (const guchar *id);

gboolean gpa_model_load (GPAModel *model);

#define GPA_MODEL_ENSURE_LOADED(m) ((m) && GPA_IS_MODEL(m) && (GPA_MODEL(m)->loaded || gpa_model_load (GPA_MODEL(m))))

G_END_DECLS

#endif /* __GPA_MODEL_H__ */
