/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gpa-spinbutton.h:
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
 *    Lutz Müller <lutz@users.sourceforge.net>
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright (C) 2004 Lutz Müller
 *  Copyright (C) 2003 Ximian, Inc. 
 */

#ifndef __GPA_SPINBUTTON_H__
#define __GPA_SPINBUTTON_H__

#include <glib.h>

G_BEGIN_DECLS

#include "gpa-widget.h"

#define GPA_TYPE_SPINBUTTON      (gpa_spinbutton_get_type ())
#define GPA_SPINBUTTON(o)        (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_SPINBUTTON, GPASpinbutton))
#define GPA_IS_SPINBUTTON(o)     (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_SPINBUTTON))

typedef struct _GPASpinbutton      GPASpinbutton;
typedef struct _GPASpinbuttonClass GPASpinbuttonClass;

#include <libgnomeprint/private/gpa-node.h>
#include "gpa-widget.h"

struct _GPASpinbutton {
	GPAWidget widget;
	
	GtkWidget *spinbutton; /* The widget */

	gchar *path;           /* The path that we are listening to */
	GPANode *node;         /* node we are a selector for */
	GPANode *config;       /* the GPAConfig node */

	gulong handler;        /* signal handler of ->node "modified" signal */
	gulong handler_config; /* signal handler of ->config "modified" signal */

	gboolean loading, saving, updating;

	gdouble lower, upper, step_increment, page_increment, page_size;
	gdouble climb_rate;
	guint digits;

	gdouble value; /* Value in Pt */
	gchar *unit;

	gdouble factor; /* 1 pt is displayed as <factor> units */
};

struct _GPASpinbuttonClass {
	GPAWidgetClass widget_class;
};

GType       gpa_spinbutton_get_type (void);

GtkWidget*  gpa_spinbutton_new (GnomePrintConfig *config,
		const guchar *path, gdouble lower, gdouble upper,
		gdouble step_increment, gdouble page_increment,
		gdouble page_size, gdouble climb_rate, guint digits);
void        gpa_spinbutton_set_unit (GPASpinbutton *spinbutton,
				     const gchar *unit);

/* Call this function if you changed values like lower or upper */
void        gpa_spinbutton_update (GPASpinbutton *spinbutton);

G_END_DECLS

#endif /* __GPA_SPINBUTTON_H__ */
