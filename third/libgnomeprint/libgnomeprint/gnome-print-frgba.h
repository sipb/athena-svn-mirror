/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-frgba.h: Wrapper context that renders
 *                       semitransparent objects as bitmaps
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
 *    Chema Celorio <chema@celorio.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright 2000-2003 Ximian, Inc. and authors
 */

#ifndef __GNOME_PRINT_FRGBA_H__
#define __GNOME_PRINT_FRGBA_H__

#include <glib.h>

G_BEGIN_DECLS

#define GNOME_TYPE_PRINT_FRGBA  (gnome_print_frgba_get_type ())
#define GNOME_PRINT_FRGBA(o)    (G_TYPE_CHECK_INSTANCE_CAST ((o), GNOME_TYPE_PRINT_FRGBA, GnomePrintFRGBA))
#define GNOME_IS_PRINT_FRGBA(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GNOME_TYPE_PRINT_FRGBA))

typedef struct _GnomePrintFRGBA      GnomePrintFRGBA;
typedef struct _GnomePrintFRGBAClass GnomePrintFRGBAClass;

#include <libgnomeprint/gnome-print.h>

GType gnome_print_frgba_get_type (void);

GnomePrintContext * gnome_print_frgba_new (GnomePrintContext *context);

G_END_DECLS

#endif /* __GNOME_PRINT_FRGBA_H__ */

