#ifndef __GPA_TRANSPORT_SELECTOR_H__
#define __GPA_TRANSPORT_SELECTOR_H__

/*
 * GPATransportSelector
 *
 * Transport selector for gnome-print
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <glib/gmacros.h>

G_BEGIN_DECLS

#include "gpa-widget.h"

#define GPA_TYPE_TRANSPORT_SELECTOR (gpa_transport_selector_get_type ())
#define GPA_TRANSPORT_SELECTOR(obj) (GTK_CHECK_CAST ((obj), GPA_TYPE_TRANSPORT_SELECTOR, GPATransportSelector))
#define GPA_TRANSPORT_SELECTOR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GPA_TYPE_TRANSPORT_SELECTOR, GPATransportSelectorClass))
#define GPA_IS_TRANSPORT_SELECTOR(obj) (GTK_CHECK_TYPE ((obj), GPA_TYPE_TRANSPORT_SELECTOR))
#define GPA_IS_TRANSPORT_SELECTOR_CLASS (GTK_CHECK_CLASS ((obj), GPA_TYPE_TRANSPORT_SELECTOR))

typedef struct _GPATransportSelector GPATransportSelector;
typedef struct _GPATransportSelectorClass GPATransportSelectorClass;

#include <libgnomeprint/private/gpa-private.h>
#include "gpa-widget.h"

struct _GPATransportSelector {
	GPAWidget widget;
	GtkWidget *hbox;
	GtkWidget *menu;
	GtkWidget *fentry;
	GtkWidget *centry;
	GtkWidget *rbhbox;
	GtkWidget *rbdefault;
	GtkWidget *rbother;
	GtkWidget *rbentry;
	GPANode *printer;
	GSList *transportlist;
};

struct _GPATransportSelectorClass {
	GPAWidgetClass widget_class;
};

GtkType gpa_transport_selector_get_type (void);

G_END_DECLS

#endif
