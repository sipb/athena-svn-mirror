/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-multipage.h: Wrapper for printing several pages onto a
 *                           single output page
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
 *    Chris Lahey <clahey@helixcode.com>
 *
 *  Copyright 1999-2003 Ximian, Inc. and authors
 */

#ifndef __GNOME_PRINT_MULTIPAGE_H__
#define __GNOME_PRINT_MULTIPAGE_H__

#include <glib.h>

G_BEGIN_DECLS

/*
 * Prints multiple input pages to single a output page.
 *
 * Component pages are placed according to affine transformation matrixes,
 * given at multipage creation time. Notice, that depending on matrix
 * type, your effective page size may differ from the output page size.
 * To handle that, you have to use the GnomePrintJob methods.
 *
 */
#define GNOME_TYPE_PRINT_MULTIPAGE         (gnome_print_multipage_get_type ())
#define GNOME_PRINT_MULTIPAGE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PRINT_MULTIPAGE, GnomePrintMultipage))
#define GNOME_PRINT_MULTIPAGE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k),    GNOME_TYPE_PRINT_MULTIPAGE, GnomePrintMultipageClass))
#define GNOME_IS_PRINT_MULTIPAGE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PRINT_MULTIPAGE))
#define GNOME_IS_PRINT_MULTIPAGE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k),    GNOME_TYPE_PRINT_MULTIPAGE))
#define GNOME_PRINT_MULTIPAGE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o),  GNOME_TYPE_PRINT_MULTIPAGE, GnomePrintMultipageClass))

typedef struct _GnomePrintMultipage      GnomePrintMultipage;
typedef struct _GnomePrintMultipageClass GnomePrintMultipageClass;

#include <libgnomeprint/gnome-print.h>

GType gnome_print_multipage_get_type (void);

GnomePrintContext *gnome_print_multipage_new (GnomePrintContext *subpc, GList *affines);

gint gnome_print_multipage_finish_page (GnomePrintMultipage *mp);

G_END_DECLS

#endif /* __GNOME_PRINT_MULTIPAGE_H__ */
