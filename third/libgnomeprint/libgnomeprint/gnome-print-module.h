/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-module.h:
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
 *  Copyright 2000-2003 Ximian, Inc.
 */

#ifndef __GNOME_PRINT_MODULE_H__
#define __GNOME_PRINT_MODULE_H__

G_BEGIN_DECLS

typedef struct _GnomePrintModule GnomePrintModule;

struct _GnomePrintModule {
	   gint flags;
};

typedef enum {
	   GNOME_PRINT_MODULE_TRANSPORT      = 1 >> 0,
	   GNOME_PRINT_MODULE_PRINTER_SOURCE = 1 >> 1,
	   GNOME_PRINT_MODULE_QUEUE          = 1 >> 2,
} GnomePrintModuleFlags;

typedef struct _GpaModuleInfo GpaModuleInfo;
struct _GpaModuleInfo {
	void (*printer_list_append) (gpointer printers);
};

void gnome_print_module_init (void);

G_END_DECLS

#endif /* __GNOME_PRINT_MODULE_H__ */
