
/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* copy-paste.h -- functions originating in nautilus-gtk-extensions.c

   Copyright (C) 1999, 2000 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: John Sullivan <sullivan@eazel.com>
*/


#ifndef COPY_PASTE_H
#define COPY_PASTE_H

#include <gtk/gtk.h>

void ammonite_gtk_label_make_bold (GtkLabel *label);
void ammonite_gtk_widget_set_font (GtkWidget *widget, GdkFont *font);
GdkFont * ammonite_gdk_font_get_bold (const GdkFont *plain_font);
void ammonite_gtk_style_set_font (GtkStyle *style, GdkFont *font);

#endif /* COPY_PASTE_H */