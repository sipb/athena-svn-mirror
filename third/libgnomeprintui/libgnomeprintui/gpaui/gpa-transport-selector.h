/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gpa-trasnport-selector.h: Simple OptionMenu for selecting transports
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
 *    Lauris Kaplinski <lauris@ximian.com>
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright (C) 2000-2003 Ximian, Inc. 
 *
 */

#ifndef __GPA_TRANSPORT_SELECTOR_H__
#define __GPA_TRANSPORT_SELECTOR_H__

#include <glib.h>

G_BEGIN_DECLS

#include "gpa-widget.h"

#define GPA_TYPE_TRANSPORT_SELECTOR (gpa_transport_selector_get_type ())
#define GPA_TRANSPORT_SELECTOR(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_TRANSPORT_SELECTOR, GPATransportSelector))
#define GPA_IS_TRANSPORT_SELECTOR(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_TRANSPORT_SELECTOR))

typedef struct _GPATransportSelector      GPATransportSelector;
typedef struct _GPATransportSelectorClass GPATransportSelectorClass;

#include <libgnomeprint/private/gpa-node.h>
#include "gpa-widget.h"

struct _GPATransportSelector {
	GPAWidget widget;
	
	GtkWidget *menu;         /* The widget */

	GPANode *node;         /* node we are listening to the Settings.Transport.Backend key */
	GPANode *config;       /* the GPAConfig node */

	gulong handler;        /* signal handler of ->node "modified" signal */
	gulong handler_config; /* signal handler of ->config "modified" signal */

	GtkWidget *file_entry;   /* Print to file entry, "output.ps" */
	GtkWidget *custom_entry; /* The custom printer command, "lpr -pFOO -#10" */

	gboolean updating;     /* A flag used to ignore emmissions create by us */
};

struct _GPATransportSelectorClass {
	GPAWidgetClass widget_class;
};

GtkType gpa_transport_selector_get_type (void);

G_END_DECLS

#endif /* __GPA_TRANSPORT_SELECTOR_H__ */
