/*
* gok-editor.h
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
* Copyright 2001,2002 University Of Toronto
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

#ifndef __GOKEDITOR_H__
#define __GOKEDITOR_H__

void gok_editor_run (void);
void gok_editor_close (void);
void gok_editor_on_exit (void);
void gok_editor_new(void);
void gok_editor_open_file (void);
void gok_editor_new_file (void);
void gok_editor_touch_file ( gboolean modified );
void gok_editor_show_parameters (GokKey* pKey);
void gok_editor_next_key (void);
void gok_editor_previous_key (void);
void gok_editor_add_key (void);
void gok_editor_delete_key (void);
void gok_editor_duplicate_key (void);
void gok_editor_update_key (void);
void gok_editor_update_keyboard(GokKeyboard* pKeyboard);
gboolean on_editor_keyboard_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data);
void gok_editor_keyboard_key_press (GtkWidget* pWidget);
gboolean gok_editor_save_current_keyboard (void);
gboolean gok_editor_save_current_keyboard_as (void);
gboolean gok_editor_save_keyboard (GokKeyboard* pKeyboard, gchar* Filename);
gboolean gok_editor_print_outputs (FILE* pFile, GokOutput* pOutput, gboolean bWrapper);
gboolean gok_editor_validate_keyboard (GokKeyboard* pKeyboard);
void gok_editor_message_filename_bad (gchar* Filename);
void gok_editor_update_title (void);

#endif /* #ifndef __GOKEDITOR_H__ */
