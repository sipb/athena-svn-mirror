/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-widget.c: Configuration widgets for GnomePrintConfig options
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

#define GNOME_PRINT_UNSTABLE_API

#include <config.h>

#include <gtk/gtk.h>
#include <libgnomeprint/private/gpa-node-private.h>
#include <libgnomeprint/gnome-print-config.h>
#include <libgnomeprint/private/gnome-print-config-private.h>

#include "gnome-print-widget.h"
#include <libgnomeprintui/gpaui/gpa-checkbutton.h>
#include <libgnomeprintui/gpaui/gpa-radiobutton.h>

GtkWidget *
gnome_print_radiobutton_new (GnomePrintConfig *config, const guchar *path, GnomePrintConfigOption *options)
{
	GtkWidget *widget;
	GPANode *node;
	GPANode *gpa_config;
	   
	g_return_val_if_fail (config, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_CONFIG (config), NULL);
	g_return_val_if_fail (path, NULL);

	gpa_config = gnome_print_config_get_node (config);
	node = gpa_node_lookup (gpa_config, path);
	if (!node) {
		g_warning ("Could not find \"%s\" node inside gnome_print_widget_new", path);
		return NULL;
	}
	gpa_node_unref (node);
	
	widget = gpa_radiobutton_new (config, path, (GPAApplicationOption *) options);

	return widget;
}

GtkWidget *
gnome_print_checkbutton_new (GnomePrintConfig *config, const guchar *path, const guchar *label)
{
	GtkWidget *widget;
	GPANode *node;
	GPANode *gpa_config;
	   
	g_return_val_if_fail (config, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_CONFIG (config), NULL);
	g_return_val_if_fail (path, NULL);

	gpa_config = gnome_print_config_get_node (config);
	node = gpa_node_lookup (gpa_config, path);
	if (!node) {
		g_warning ("Could not find \"%s\" node inside gnome_print_widget_new", path);
		return NULL;
	}
	gpa_node_unref (node);
	
	widget = gpa_checkbutton_new (config, path, label);

	return widget;
}

GtkWidget *
gnome_print_widget_new (GnomePrintConfig *config, const guchar *path, GnomePrintWidgetType type)
{
	GtkWidget *widget;
	GPANode *gpa_config;
	GPANode *node;
	
	g_return_val_if_fail (config, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_CONFIG (config), NULL);
	g_return_val_if_fail (path, NULL);

	gpa_config = gnome_print_config_get_node (config);
	node = gpa_node_lookup (gpa_config, path);
	if (!node) {
		   g_warning ("Could not find \"%s\" node inside gnome_print_widget_new", path);
		   return NULL;
	}
	gpa_node_unref (node);

	switch (type) {
	case GNOME_PRINT_WIDGET_CHECKBUTTON:
		widget = gpa_checkbutton_new (config, path, "Some label here");
		break;
	default:
		widget = gtk_check_button_new_with_mnemonic ("_Invalid GnomePrintWidget type");
		break;
	}

	gtk_widget_show_all (widget);
	
	return widget;
}
