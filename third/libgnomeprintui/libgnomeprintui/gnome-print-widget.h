/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-widget.h: Configuration widgets for GnomePrintConfig options
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright 2003 Ximian, Inc.
 */


#ifndef __GNOME_PRINT_WIDGET_H__
#define __GNOME_PRINT_WIDGET_H__

#include <glib.h>

G_BEGIN_DECLS

#include <libgnomeprint/gnome-print-config.h>
#include <gtk/gtkwidget.h>

typedef enum {
	   GNOME_PRINT_WIDGET_CHECKBUTTON
} GnomePrintWidgetType;

#ifdef GNOME_PRINT_UNSTABLE_API
GtkWidget * gnome_print_widget_new      (GnomePrintConfig *config, const guchar *path, GnomePrintWidgetType type);
GtkWidget * gnome_print_checkbutton_new (GnomePrintConfig *config, const guchar *path, const guchar *label);
GtkWidget * gnome_print_radiobutton_new (GnomePrintConfig *config, const guchar *path, GnomePrintConfigOption *options);
#endif

G_END_DECLS

#endif /* __GNOME_PRINT_WIDGET_H__ */
