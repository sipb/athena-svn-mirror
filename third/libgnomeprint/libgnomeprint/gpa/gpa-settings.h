/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GPA_SETTINGS_H__
#define __GPA_SETTINGS_H__

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

#define GPA_TYPE_SETTINGS (gpa_settings_get_type ())
#define GPA_SETTINGS(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_SETTINGS, GPASettings))
#define GPA_SETTINGS_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GPA_TYPE_SETTINGS, GPASettingsClass))
#define GPA_IS_SETTINGS(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_SETTINGS))
#define GPA_IS_SETTINGS_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GPA_TYPE_SETTINGS))
#define GPA_SETTINGS_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GPA_TYPE_SETTINGS, GPASettingsClass))

typedef struct _GPASettings GPASettings;
typedef struct _GPASettingsClass GPASettingsClass;

#include <libxml/tree.h>
#include "gpa-list.h"

/* GPASettings */

struct _GPASettings {
	GPANode node;

	GPANode *name;
	GPANode *model;
	GPANode *keys;
};


struct _GPASettingsClass {
	GPANodeClass node_class;
};

GType gpa_settings_get_type (void);

GPANode *gpa_settings_new_empty (const guchar *name);
GPANode *gpa_settings_new_from_model (GPANode *model, const guchar *name);
GPANode *gpa_settings_new_from_model_full (GPANode *model, const guchar *id, const guchar *name);
GPANode *gpa_settings_new_from_model_and_tree (GPANode *model, xmlNodePtr tree);

xmlNodePtr gpa_settings_write (xmlDocPtr doc, GPANode *settings);

gboolean gpa_settings_copy (GPASettings *dst, GPASettings *src);

G_END_DECLS

#endif /* __GPA_SETTINGS_H__ */
