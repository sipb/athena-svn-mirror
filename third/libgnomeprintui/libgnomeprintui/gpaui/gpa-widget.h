#ifndef __GPA_WIDGET_H__
#define __GPA_WIDGET_H__

/*
 * gpa-widget
 *
 * Abstract base class for gnome-print configuration widgets
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#include <gtk/gtkbin.h>
#include <libgnomeprint/gnome-print-config.h>

#define GPA_TYPE_WIDGET (gpa_widget_get_type ())
#define GPA_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPA_TYPE_WIDGET, GPAWidget))
#define GPA_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GPA_TYPE_WIDGET, GPAWidgetClass))
#define GPA_IS_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPA_TYPE_WIDGET))
#define GPA_IS_WIDGET_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GPA_TYPE_WIDGET))
#define GPA_WIDGET_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GPA_TYPE_WIDGET, GPAWidgetClass))

typedef struct _GPAWidget GPAWidget;
typedef struct _GPAWidgetClass GPAWidgetClass;

struct _GPAWidget {
	GtkBin bin;
	GnomePrintConfig *config;
};

struct _GPAWidgetClass {
	GtkBinClass bin_class;
	gint (* construct) (GPAWidget *widget);
};

GtkType gpa_widget_get_type (void);

GtkWidget *gpa_widget_new (GtkType type, GnomePrintConfig *config);

gboolean gpa_widget_construct (GPAWidget *widget, GnomePrintConfig *config);

G_END_DECLS

#endif
