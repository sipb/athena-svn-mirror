/* gok-page-feedbacks.h
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto
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

#include <gnome.h>
#include <glade/glade.h>
#include "gok-feedback.h"

gboolean gok_page_feedbacks_initialize (GladeXML* xml);
void gok_page_feedbacks_refresh (void);
gboolean gok_page_feedbacks_apply (void);
gboolean gok_page_feedbacks_revert (void);
void gok_page_feedbacks_backup (void);
void gok_page_actions_action_changed (GtkEditable* pEditControl);
void gok_page_feedbacks_update_controls (GokFeedback* pFeedback);
void gok_page_feedbacks_button_clicked_change_name (void);
void gok_page_feedbacks_button_clicked_new (void);
void gok_page_feedbacks_fill_combo_feedback_names (void);
void gok_page_feedbacks_button_clicked_delete (void);
void gok_page_feedbacks_check_keyflashing_clicked (void);
void gok_page_feedbacks_spin_keyflashing_changed (void);
void gok_page_feedbacks_check_sound_clicked (void);
void gok_page_feedbacks_entry_soundname_changed (void);
void gok_page_feedbacks_get_sound_file (void);
void gok_feedbacks_update_sound_combo (void);
gboolean gok_page_feedbacks_get_changed (void);
void gok_page_feedbacks_set_changed (gboolean bTrueFalse);
void gok_page_feedbacks_feedback_changed (GtkEditable* pEditControl);

