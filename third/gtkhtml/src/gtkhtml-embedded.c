/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Copyright (C) 2000 Helix Code Inc.
 *
 *  Authors: Michael Zucchi <notzed@helixcode.com>
 *
 *  An embeddable html widget.
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
 */

#include <config.h>
#include <gnome.h>

#include "gtkhtml-embedded.h"
#include "htmlengine.h"

static void gtk_html_embedded_class_init (GtkHTMLEmbeddedClass *class);
static void gtk_html_embedded_init       (GtkHTMLEmbedded *gspaper);

static void gtk_html_embedded_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void gtk_html_embedded_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

/* saved parent calls */
static void (*old_add)(GtkContainer *container, GtkWidget *child);
static void (*old_remove)(GtkContainer *container, GtkWidget *child);

static GtkBin *parent_class;

enum {
	DRAW_GDK,
	DRAW_PRINT,
	CHANGED,
	LAST_SIGNAL
};
	
static guint signals [LAST_SIGNAL] = { 0 };

guint
gtk_html_embedded_get_type (void)
{
	static guint select_paper_type = 0;
	
	if (!select_paper_type) {
		GtkTypeInfo html_embedded_info = {
			"GtkHTMLEmbedded",
			sizeof (GtkHTMLEmbedded),
			sizeof (GtkHTMLEmbeddedClass),
			(GtkClassInitFunc) gtk_html_embedded_class_init,
			(GtkObjectInitFunc) gtk_html_embedded_init,
			NULL,
			NULL
		};
    
		select_paper_type = gtk_type_unique (gtk_bin_get_type (), &html_embedded_info);
	}
  
	return select_paper_type;
}

static void
free_param(void *key, void *value, void *data)
{
	g_free (key);
	g_free (value);
}

static void
gtk_html_embedded_finalize (GtkObject *object)
{
	GtkHTMLEmbedded *eb = GTK_HTML_EMBEDDED(object);

	g_hash_table_foreach (eb->params, free_param, 0);
	g_hash_table_destroy (eb->params);
	g_free(eb->classid);
	g_free(eb->type);

	GTK_OBJECT_CLASS(parent_class)->finalize (object);
}

static void
gtk_html_embedded_changed (GtkHTMLEmbedded *ge)
{
	gtk_signal_emit (GTK_OBJECT (ge), signals[CHANGED]);
}

static void gtk_html_embedded_add (GtkContainer *container, GtkWidget *child)
{
	g_return_if_fail (container != NULL);

	/* can't add something twice */
	g_return_if_fail( GTK_BIN(container)->child == NULL );

	old_add(container, child);
	gtk_html_embedded_changed(GTK_HTML_EMBEDDED(container));
}

static void gtk_html_embedded_remove (GtkContainer *container, GtkWidget *child)
{
	g_return_if_fail (container != NULL);
	g_return_if_fail( GTK_BIN(container)->child != NULL );

	old_remove(container, child);

	gtk_html_embedded_changed(GTK_HTML_EMBEDDED(container));
}

typedef void (*draw_print_signal)(GtkObject *, gpointer, gpointer);
typedef void (*draw_gdk_signal)(GtkObject *, gpointer, gpointer, gint, gint, gpointer);

static void 
draw_gdk_signal_marshaller(GtkObject * object, GtkSignalFunc func,
			   gpointer func_data, GtkArg * args) {
	draw_gdk_signal ff = (draw_gdk_signal)func;
	(*ff)(object,
	      GTK_VALUE_POINTER(args[0]),
	      GTK_VALUE_POINTER(args[1]),
	      GTK_VALUE_INT(args[2]),
	      GTK_VALUE_INT(args[3]),
	      func_data);
}

static void
gtk_html_embedded_class_init (GtkHTMLEmbeddedClass *class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkContainerClass *container_class;
	
	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass*) class;
	container_class = (GtkContainerClass*) class;

	parent_class = gtk_type_class (gtk_bin_get_type ());

	signals [CHANGED] =
		gtk_signal_new ("changed",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GtkHTMLEmbeddedClass, changed),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);
	signals[DRAW_GDK] = 
		gtk_signal_new("draw_gdk", GTK_RUN_FIRST,
			       object_class->type, 
			       GTK_SIGNAL_OFFSET(GtkHTMLEmbeddedClass, draw_gdk),
			       draw_gdk_signal_marshaller, GTK_TYPE_NONE, 4,
			       GTK_TYPE_POINTER, GTK_TYPE_POINTER,
			       GTK_TYPE_INT, GTK_TYPE_INT);
	
	signals[DRAW_PRINT] = 
		gtk_signal_new("draw_print", GTK_RUN_FIRST,
			       object_class->type,
			       GTK_SIGNAL_OFFSET(GtkHTMLEmbeddedClass, draw_print),
			       gtk_marshal_NONE__POINTER,
			       GTK_TYPE_NONE, 1,
			       GTK_TYPE_POINTER);
	

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	object_class->finalize = gtk_html_embedded_finalize;

	widget_class->size_request = gtk_html_embedded_size_request;
	widget_class->size_allocate = gtk_html_embedded_size_allocate;

	old_add = container_class->add;
	container_class->add = gtk_html_embedded_add;
	old_remove = container_class->remove;
	container_class->remove = gtk_html_embedded_remove;
}

static void
gtk_html_embedded_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GtkBin *bin;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (requisition != NULL);
	
	bin = GTK_BIN (widget);

	if (bin->child) {
		gtk_widget_size_request (bin->child, requisition);
	} else {
		requisition->width = widget->requisition.width;
		requisition->height = widget->requisition.height;
	}
}

static void
gtk_html_embedded_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GtkBin *bin;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (allocation != NULL);
	
	bin = GTK_BIN (widget);

	if (bin->child && GTK_WIDGET_VISIBLE (bin->child)) {
		gtk_widget_size_allocate(bin->child, allocation);
	}
	widget->allocation = *allocation;
}

static void
gtk_html_embedded_init (GtkHTMLEmbedded *ge)
{
	ge->descent = 0;
	ge->params  = g_hash_table_new (g_str_hash, g_str_equal);
}

/**
 * gtk_html_embedded_new:
 * 
 * Create a new GtkHTMLEmbedded widget.
 * Note that this function should never be called outside of gtkhtml.
 * 
 * Return value: A new GtkHTMLEmbedded widget.
 **/
GtkWidget *
gtk_html_embedded_new (char *classid, char *name, char *type, char *data, int width, int height)
{
	GtkHTMLEmbedded *em;

	em = (GtkHTMLEmbedded *)( gtk_type_new (gtk_html_embedded_get_type ()));

	if (width != -1 || height != -1)
		gtk_widget_set_usize (GTK_WIDGET (em), width, height);

	em->width = width;
	em->height = height;
	em->type = type ? g_strdup(type) : NULL;
	em->classid = g_strdup(classid);
	em->name = g_strdup(name);
	em->data = g_strdup(data);

	return (GtkWidget *)em;
}

/**
 * gtk_html_embedded_get_parameter:
 * @ge: the #GtkHTMLEmbedded widget.
 * @param: the parameter to examine.
 *
 * Returns: the value of the parameter.
 */
char *
gtk_html_embedded_get_parameter (GtkHTMLEmbedded *ge, char *param)
{
	return g_hash_table_lookup (ge->params, param);
}

/**
 * gtk_html_embedded_set_parameter:
 * @ge: The #GtkHTMLEmbedded widget.
 * @param: the name of the parameter to set.
 * @value: the value of the parameter.
 * 
 * The parameter named @name to the @value.
 */
void
gtk_html_embedded_set_parameter (GtkHTMLEmbedded *ge, char *param, char *value)
{
	gchar *lookup;

	if (!param)
		return;
	lookup = (gchar *) g_hash_table_lookup (ge->params, param);
	if (lookup)
		g_free (lookup);
	g_hash_table_insert (ge->params,
			     lookup ? param : g_strdup (param),
			     value  ? g_strdup(value) : NULL);
}

/**
 * gtk_html_embedded_set_descent:
 * @ge: the GtkHTMLEmbedded widget.
 * @descent: the value of the new descent.
 *
 * Set the descent of the widget beneath the baseline.
 */
void
gtk_html_embedded_set_descent (GtkHTMLEmbedded *ge, int descent)
{
	if (ge->descent == descent)
		return;

	ge->descent = descent;
	gtk_html_embedded_changed (ge);
}

