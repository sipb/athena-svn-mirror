/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-string-picker.h - A widget to pick a string from a list.

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

   Authors: Ramiro Estrugo <ramiro@eazel.com>
*/

#ifndef EEL_STRING_PICKER_H
#define EEL_STRING_PICKER_H

#include <eel/eel-caption.h>
#include <eel/eel-string-list.h>

/*
 * EelStringPicker is made up of 2 widgets. 
 *
 * [title label] [string list]
 *
 * The user can select a string from the list.
 */
G_BEGIN_DECLS

#define EEL_TYPE_STRING_PICKER            (eel_string_picker_get_type ())
#define EEL_STRING_PICKER(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_STRING_PICKER, EelStringPicker))
#define EEL_STRING_PICKER_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_STRING_PICKER, EelStringPickerClass))
#define EEL_IS_STRING_PICKER(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_STRING_PICKER))

typedef struct EelStringPicker		 EelStringPicker;
typedef struct EelStringPickerClass      EelStringPickerClass;
typedef struct EelStringPickerDetail     EelStringPickerDetail;

struct EelStringPicker
{
	/* Super Class */
	EelCaption caption;
	
	/* Private stuff */
	EelStringPickerDetail *detail;
};

struct EelStringPickerClass
{
	EelCaptionClass parent_class;

	/* Signals */
	void (* changed) (EelStringPicker *string_picker);
};

/* This is the string used to insert a separator. */
#define EEL_STRING_PICKER_SEPARATOR_STRING "----------"

GtkType        eel_string_picker_get_type                  (void);
GtkWidget*     eel_string_picker_new                       (void);

/* Set the list of strings. */
void           eel_string_picker_set_string_list           (EelStringPicker       *string_picker,
							    const EelStringList   *string_list);

/* Access a copy of the list of strings. */
EelStringList *eel_string_picker_get_string_list           (const EelStringPicker *string_picker);


/* Get the selected string.  Resulting string needs to be freed with g_free(). */
char *         eel_string_picker_get_selected_string       (EelStringPicker       *string_picker);


/* Set the selected string.  The internal string list needs to contain the 'string'. */
void           eel_string_picker_set_selected_string       (EelStringPicker       *string_picker,
							    const char            *string);
void           eel_string_picker_set_selected_string_index (EelStringPicker       *string_picker,
							    guint                  index);

/* Add a new string to the picker. */
void           eel_string_picker_insert_string             (EelStringPicker       *string_picker,
							    const char            *string);
void           eel_string_picker_insert_separator          (EelStringPicker       *string_picker);

/* Does the string picker contain the given string ? */
gboolean       eel_string_picker_contains                  (const EelStringPicker *string_picker,
							    const char            *string);

/* Return the index of the given string */
int            eel_string_picker_get_index_for_string      (const EelStringPicker *string_picker,
							    const char            *string);

/* Remove all entries from the string picker. */
void           eel_string_picker_clear                     (EelStringPicker       *string_picker);

/* Set the list of insensitive strings */
void           eel_string_picker_set_insensitive_list      (EelStringPicker       *string_picker,
							    const EelStringList   *insensitive_list);

G_END_DECLS

#endif /* EEL_STRING_PICKER_H */

