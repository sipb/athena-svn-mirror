/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gpa-checkbutton.h: a checkbutton selector for boolean GPANodes
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
 *  Authors :
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright (C) 2003 Ximian, Inc. 
 */

#ifndef __GPA_CHECKBUTTON_H__
#define __GPA_CHECKBUTTON_H__

#include <glib.h>

G_BEGIN_DECLS

#include "gpa-widget.h"

#define GPA_TYPE_CHECKBUTTON      (gpa_checkbutton_get_type ())
#define GPA_CHECKBUTTON(o)        (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_CHECKBUTTON, GPACheckbutton))
#define GPA_IS_CHECKBUTTON(o)     (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_CHECKBUTTON))

typedef struct _GPACheckbutton      GPACheckbutton;
typedef struct _GPACheckbuttonClass GPACheckbuttonClass;

#include <libgnomeprint/private/gpa-node.h>
#include "gpa-widget.h"

struct _GPACheckbutton {
	GPAWidget widget;
	
	GtkWidget *checkbox;   /* The widget */

	gchar *path;           /* The path that we are listening to */
	GPANode *node;         /* node we are a selector for */
	GPANode *config;       /* the GPAConfig node */

	gulong handler;        /* signal handler of ->node "modified" signal */
	gulong handler_config; /* signal handler of ->config "modified" signal */

	gboolean updating;     /* A flag used to ignore emmissions create by us */
};

struct _GPACheckbuttonClass {
	GPAWidgetClass widget_class;
};

GType       gpa_checkbutton_get_type (void);

GtkWidget*  gpa_checkbutton_new (GnomePrintConfig *config,
				 const guchar *path, const guchar *label);

G_END_DECLS

#endif /* __GPA_CHECKBUTTON_H__ */
