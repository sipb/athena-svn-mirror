/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gpa-option-menu.h:
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
 *   Chema Celorio <chema@ximian.com>
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001-2003 Ximian, Inc. 
 *
 */

#ifndef __GPA_OPTION_MENU_H__
#define __GPA_OPTION_MENU_H__

#include <glib.h>

G_BEGIN_DECLS

#define GPA_TYPE_OPTION_MENU         (gpa_option_menu_get_type ())
#define GPA_OPTION_MENU(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_OPTION_MENU, GPAOptionMenu))
#define GPA_IS_OPTION_MENU(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_OPTION_MENU))

#include "gpa-widget.h"
#include <libgnomeprint/private/gpa-node.h>

typedef struct _GPAOptionMenu      GPAOptionMenu;
typedef struct _GPAOptionMenuClass GPAOptionMenuClass;

struct _GPAOptionMenu {
	GPAWidget widget;

	GtkWidget *menu;       /* The widget */

	GPANode *node;         /* node we are listening to */
	GPANode *config;       /* GPAConfig */
	gchar *key;            /* key of @node, ej. Settings.Media.PhysicalSize */

	gulong handler;        /* signal handler of ->node "modified" signal */
	gulong handler_config; /* signal handler of ->config "modified" signal */

	gboolean updating;     /* A flag used to ignore emmissions create by us */
};

struct _GPAOptionMenuClass {
	GPAWidgetClass widget_class;
};

GType       gpa_option_menu_get_type (void);

GtkWidget * gpa_option_menu_new       (GnomePrintConfig *config, const guchar *key);

G_END_DECLS

#endif /* __GPA_OPTION_MENU_H__ */
