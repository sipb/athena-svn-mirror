/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-font-picker.h - A simple widget to select scalable fonts.

   Copyright (C) 1999, 2000 Eazel, Inc.
   Copyright (C) 2002, Bent Spoon Software

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

   Authors: Ramiro Estrugo <ramiro@eazel.com>, Darin Adler <darin@bentspoon.com>
*/

#ifndef EEL_FONT_PICKER_H
#define EEL_FONT_PICKER_H

#include <eel/eel-caption.h>

G_BEGIN_DECLS

#define EEL_TYPE_FONT_PICKER            (eel_font_picker_get_type ())
#define EEL_FONT_PICKER(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_FONT_PICKER, EelFontPicker))
#define EEL_FONT_PICKER_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_FONT_PICKER, EelFontPickerClass))
#define EEL_IS_FONT_PICKER(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_FONT_PICKER))

typedef struct EelFontPickerDetails EelFontPickerDetails;

typedef struct
{
	EelCaption parent_part;
	EelFontPickerDetails *details;
} EelFontPicker;

typedef struct
{
	EelCaptionClass parent_class_part;
	void (* changed) (EelFontPicker *font_picker);
} EelFontPickerClass;

GType      eel_font_picker_get_type          (void);
GtkWidget* eel_font_picker_new               (PangoContext  *context);
char *     eel_font_picker_get_selected_font (EelFontPicker *font_picker);
void       eel_font_picker_set_selected_font (EelFontPicker *font_picker,
					      const char    *font);

G_END_DECLS

#endif /* EEL_FONT_PICKER_H */
