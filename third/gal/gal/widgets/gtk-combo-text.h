/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * gtk-combo-text.h - A combo box for selecting from a list.
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _GTK_COMBO_TEXT_H
#define _GTK_COMBO_TEXT_H

#include <gal/widgets/gtk-combo-box.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_COMBO_TEXT(obj)	    GTK_CHECK_CAST (obj, gtk_combo_text_get_type (), GtkComboText)
#define GTK_COMBO_TEXT_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_combo_text_get_type (), GtkComboTextClass)
#define GTK_IS_COMBO_TEXT(obj)      GTK_CHECK_TYPE (obj, gtk_combo_text_get_type ())

typedef struct _GtkComboText	   GtkComboText;
/* typedef struct _GtkComboTextPrivate GtkComboTextPrivate;*/
typedef struct _GtkComboTextClass  GtkComboTextClass;

struct _GtkComboText {
	GtkComboBox parent;

	GtkWidget *entry;
	GtkWidget *list;
	GtkWidget *scrolled_window;
	GtkStateType cache_mouse_state;
	GtkWidget *cached_entry;
	gboolean case_sensitive;
	GHashTable*elements;
};

struct _GtkComboTextClass {
	GtkComboBoxClass parent_class;
};


GtkType    gtk_combo_text_get_type  (void);
GtkWidget *gtk_combo_text_new       (gboolean const is_scrolled);
void       gtk_combo_text_construct (GtkComboText *ct, gboolean const is_scrolled);

gint       gtk_combo_text_set_case_sensitive (GtkComboText *combo_text,
					      gboolean val);
void       gtk_combo_text_select_item (GtkComboText *combo_text,
				       int elem);
void       gtk_combo_text_set_text (GtkComboText *combo_text,
				       const gchar *text);
void       gtk_combo_text_add_item    (GtkComboText *combo_text,
				       const gchar *item,
				       const gchar *value);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif
