/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
   Copyright (C) 2000 CodeFactory AB
   Copyright (C) 2000 Jonas Borgström <jonas@codefactory.se>
   Copyright (C) 2000 Anders Carlsson <andersca@codefactory.se>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs.h>

#include "gtkhtmlcontext.h"
#include "browser-window.h"

gint
main (gint argc, gchar **argv)
{
	GtkWidget *browser;

	g_thread_init(NULL);

	gtk_init (&argc, &argv);

	puts ("Initializing gnome-vfs...");
	gnome_vfs_init ();

	browser = browser_window_new (NULL);
	g_signal_connect (G_OBJECT (browser), "destroy",
			  G_CALLBACK (gtk_main_quit), NULL);
	gtk_widget_show (browser);

	gtk_main ();

	return 0;
}
