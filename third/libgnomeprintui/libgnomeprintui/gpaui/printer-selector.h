#ifndef __GPA_PRINTER_SELECTOR_H__
#define __GPA_PRINTER_SELECTOR_H__

/*
 * GPAPrinterSelector
 *
 * Simple OptonMenu for selecting printers
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <glib/gmacros.h>

G_BEGIN_DECLS

#define GPA_TYPE_PRINTER_SELECTOR (gpa_printer_selector_get_type ())
#define GPA_PRINTER_SELECTOR(obj) (GTK_CHECK_CAST ((obj), GPA_TYPE_PRINTER_SELECTOR, GPAPrinterSelector))
#define GPA_PRINTER_SELECTOR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GPA_TYPE_PRINTER_SELECTOR, GPAPrinterSelectorClass))
#define GPA_IS_PRINTER_SELECTOR(obj) (GTK_CHECK_TYPE ((obj), GPA_TYPE_PRINTER_SELECTOR))
#define GPA_IS_PRINTER_SELECTOR_CLASS (GTK_CHECK_CLASS ((obj), GPA_TYPE_PRINTER_SELECTOR))

typedef struct _GPAPrinterSelector GPAPrinterSelector;
typedef struct _GPAPrinterSelectorClass GPAPrinterSelectorClass;

#include <libgnomeprint/private/gpa-private.h>
#include "gpa-widget.h"

struct _GPAPrinterSelector {
	GPAWidget widget;
	GtkWidget *menu; /* Option menu */
	GPANode *printers; /* <Printers> node */
	GSList *printerlist; /* GSList of <Printer> nodes */
};

struct _GPAPrinterSelectorClass {
	GPAWidgetClass widget_class;
};

GtkType gpa_printer_selector_get_type (void);

G_END_DECLS

#endif
