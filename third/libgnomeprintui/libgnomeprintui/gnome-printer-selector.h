/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-printer-selector.h:
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
 *    Raph Levien <raph@acm.org>
 *    Miguel de Icaza <miguel@kernel.org>
 *    Lauris Kaplinski <lauris@ximian.com>
 *    Chema Celorio <chema@ximian.com>
 *
 *  Copyright (C) 1999-2003 Ximian Inc. and authors
 *
 */

#ifndef __GNOME_PRINTER_SELECTOR_H__
#define __GNOME_PRINTER_SELECTOR_H__

#include <glib.h>

G_BEGIN_DECLS

#define GNOME_TYPE_PRINTER_SELECTOR         (gnome_printer_selector_get_type ())
#define GNOME_PRINTER_SELECTOR(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PRINTER_SELECTOR, GnomePrinterSelector))
#define GNOME_PRINTER_SELECTOR_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k),    GNOME_TYPE_PRINTER_SELECTOR, GnomePrinterSelectorClass))
#define GNOME_IS_PRINTER_SELECTOR(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PRINTER_SELECTOR))
#define GNOME_IS_PRINTER_SELECTOR_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k),    GNOME_TYPE_PRINTER_SELECTOR))
#define GNOME_PRINTER_SELECTOR_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o),  GNOME_TYPE_PRINTER_SELECTOR, GnomePrinterSelectorClass))

typedef struct _GnomePrinterSelector      GnomePrinterSelector;
typedef struct _GnomePrinterSelectorClass GnomePrinterSelectorClass;

#include <libgnomeprint/gnome-print-config.h>
#include <libgnomeprintui/gpaui/gpa-widget.h>

struct _GnomePrinterSelector {
	GPAWidget gpawidget;
	GtkAccelGroup *accel_group;
	GtkWidget *printers;   /* GPAPrinterSelector   */
	GtkWidget *settings;   /* GPASettingsSelector  */
	GtkWidget *transport;  /* GPATransportSelector */
	GtkWidget *state, *type, *location, *comment;
};

struct _GnomePrinterSelectorClass {
	GPAWidgetClass gpa_widget_class;
};


GtkType            gnome_printer_selector_get_type (void);
GtkWidget *        gnome_printer_selector_new (GnomePrintConfig *config);
GtkWidget *        gnome_printer_selector_new_default (void);
GnomePrintConfig * gnome_printer_selector_get_config (GnomePrinterSelector *psel);

G_END_DECLS

#endif /* __GNOME_PRINTER_SELECTOR_H__ */
