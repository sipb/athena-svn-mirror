#ifndef __GNOME_PRINT_FAX_H__
#define __GNOME_PRINT_FAX_H__

/*
 * Group 3 fax driver
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * References :
 * [1] Portable Document Format Referece Manual, Version 1.3 (March 11, 1999)
 *
 * Authors:
 *   Roberto Majadas "telemaco" <phoenix@nova.es>
 *
 * Copyright 2000-2001 Ximian, Inc. and authors
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#define GNOME_TYPE_PRINT_FAX (gnome_print_fax_get_type ())
#define GNOME_PRINT_FAX(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PRINT_FAX, GnomePrintFAX))
#define GNOME_PRINT_FAX_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GNOME_TYPE_PRINT_FAX, GnomePrintFAXClass))
#define GNOME_IS_PRINT_FAX(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PRINT_FAX))
#define GNOME_IS_PRINT_FAX_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GNOME_TYPE_PRINT_FAX))
#define GNOME_PRINT_FAX_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GNOME_TYPE_PRINT_FAX, GnomePrintFAXClass))

typedef struct _GnomePrintFAX        GnomePrintFAX;
typedef struct _GnomePrintFAXPrivate GnomePrintFAXPrivate;
typedef struct _GnomePrintFAXClass   GnomePrintFAXClass;

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-rgbp.h>

struct _GnomePrintFAX
{
	GnomePrintRGBP rgbp;

	GnomePrintFAXPrivate *priv;
};

struct _GnomePrintFAXPrivate
{
	gint run_length ;
	gint run_length_color ;
	gint actual_color;	
	gint first_code_of_row ;
	gint fax_encode_buffer ;
	gint fax_encode_buffer_pivot ;
	gint first_code_of_doc ;
};

struct _GnomePrintFAXClass
{
	GnomePrintRGBPClass parent_class;
};

GType gnome_print_fax_get_type (void);

GnomePrintContext *gnome_print_fax_new (GnomePrintConfig *config);

G_END_DECLS

#endif /* __GNOME_PRINT_FAX_H__ */

