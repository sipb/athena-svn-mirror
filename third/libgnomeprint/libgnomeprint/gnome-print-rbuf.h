/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gnome-print-rbuf: backend that renders into a transformed
 *                   rectangular RGB(A) buffer
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
 *    Lauris Kaplinski (lauris@ariman.ee)
 *
 *  Copyright (C) 2000-2001 Ximian Inc.
 */

#ifndef __GNOME_PRINT_RBUF_H__
#define __GNOME_PRINT_RBUF_H__

#include <glib.h>

G_BEGIN_DECLS

#define GNOME_TYPE_PRINT_RBUF         (gnome_print_rbuf_get_type ())
#define GNOME_PRINT_RBUF(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PRINT_RBUF, GnomePrintRBuf))
#define GNOME_IS_PRINT_RBUF(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PRINT_RBUF))

typedef struct _GnomePrintRBuf GnomePrintRBuf;

#include <libgnomeprint/gnome-print.h>

GType gnome_print_rbuf_get_type (void);

GnomePrintContext * gnome_print_rbuf_new (guchar *pixels,
								  gint width, gint height, gint rowstride,
								  gdouble page2buf[6], gboolean alpha);

G_END_DECLS

#endif /* __GNOME_PRINT_RBUF_H__ */

