/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gpa-widget.h:  Abstract base class for gnome-print configuration widgets
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
 * Copyright (C) 2000-2003 Ximian, Inc. 
 *
 */

#ifndef __GPA_WIDGET_H__
#define __GPA_WIDGET_H__

#include <glib.h>

G_BEGIN_DECLS

#include <gtk/gtkbin.h>
#include <libgnomeprint/gnome-print-config.h>
#include <libgnomeprint/private/gpa-node.h>

#define GPA_TYPE_WIDGET         (gpa_widget_get_type ())
#define GPA_WIDGET(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GPA_TYPE_WIDGET, GPAWidget))
#define GPA_WIDGET_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST    ((k), GPA_TYPE_WIDGET, GPAWidgetClass))
#define GPA_IS_WIDGET(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GPA_TYPE_WIDGET))
#define GPA_IS_WIDGET_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE    ((k), GPA_TYPE_WIDGET))
#define GPA_WIDGET_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS  ((o), GPA_TYPE_WIDGET, GPAWidgetClass))

typedef struct _GPAWidget      GPAWidget;
typedef struct _GPAWidgetClass GPAWidgetClass;

struct _GPAWidget {
	GtkBin bin;
	/* FIXME: Should be a GPANode * of type GPARoot (Chema) */
	GnomePrintConfig *config;
};

struct _GPAWidgetClass {
	GtkBinClass bin_class;
	gint (* construct) (GPAWidget *widget);
};

GtkType     gpa_widget_get_type (void);

GtkWidget * gpa_widget_new       (GtkType type, GnomePrintConfig *config);
gboolean    gpa_widget_construct (GPAWidget *widget, GnomePrintConfig *config);

G_END_DECLS

#endif /* __GPA_WIDGET_H__ */
