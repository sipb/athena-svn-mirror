/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GNOME_PRINT_CONFIG_DIALOG_PRIVATE_H__
#define __GNOME_PRINT_CONFIG_DIALOG_PRIVATE_H__

/*
 *  gnome-print-config-dialog-private.h: A dialog to configure specific 
 *  printer settings.
 *
 *  NOTE: This interface is considered private and should not be used by 
 *  applications directly!
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
 *      Andreas J. Guelzow <aguelzow@taliesin.ca>
 *
 *  Copyright (C) 2003  Andreas J. Guelzow
 *
 */

#include "gnome-print-config-dialog.h"

struct _GnomePrintConfigDialog {
	GtkDialog dialog;

	GnomePrintConfig *config;

	GtkWidget *duplex;
	GtkWidget *duplex_image;
	GtkWidget *tumble;
	GtkWidget *tumble_image;
};

struct _GnomePrintConfigDialogClass {
	GtkDialogClass parent_class;
};


#endif /*  __GNOME_PRINT_CONFIG_DIALOG_PRIVATE_H__ */
