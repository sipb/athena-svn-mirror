/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GNOME_PRINT_DIALOG_PRIVATE_H__
#define __GNOME_PRINT_DIALOG_PRIVATE_H__

/*
 *  gnome-print-dialog-private.h: Private structs of gnome-print-dialog
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
 *    Michael Zucchi <notzed@helixcode.com>
 *    Chema Celorio <chema@celorio.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2002 Ximian Inc.
 *
 */

struct _GnomePrintDialog {
	GtkDialog dialog;

	GnomePrintConfig *config;

	GtkWidget *notebook;

	GtkWidget *job;
	GtkWidget *printer;
};

struct _GnomePrintDialogClass {
	GtkDialogClass parent_class;
};


#endif /*  __GNOME_PRINT_DIALOG_PRIVATE_H__ */
