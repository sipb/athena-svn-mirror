/* gok-settings-page-actions.h
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
#include "gok-action.h"

gboolean gok_page_actions_initialize (GladeXML* xml);
void gok_page_actions_refresh (void);
gboolean gok_page_actions_apply (void);
gboolean gok_page_actions_revert (void);
void gok_page_actions_backup (void);
void gok_page_actions_toggle_type_switch (gboolean Pressed);
void gok_page_actions_toggle_source_corepointer (gboolean Pressed);
void gok_page_actions_toggle_source_joystick (gboolean Pressed);
void gok_page_actions_toggle_source_input (gboolean Pressed);
void gok_page_actions_toggle_type_valuator (gboolean Pressed);
void gok_page_actions_button_clicked_new (void);
void gok_page_actions_button_clicked_delete (void);
void gok_page_actions_button_clicked_change_name (void);
void gok_page_actions_action_changed (GtkEditable* pEditControl);
void gok_page_actions_enable_switch_controls (gboolean bTrueFalse);
void gok_page_actions_enable_valuator_controls (gboolean bTrueFalse);
void gok_page_actions_enable_radios_type (gboolean bTrueFalse);
void gok_page_actions_fill_combo_action_names (void);
gint gok_page_actions_get_radio_number (GtkRadioButton* pRadioButtonGiven);
void gok_page_actions_input_device_changed (GtkEditable* pEditable);
void gok_page_actions_update_controls (GokAction* pAction);
void gok_page_actions_set_number (gint NumberSwitch);
void gok_page_actions_set_state (gint State);
void gok_page_actions_set_rate (gint Rate);
void gok_page_actions_pointer_keyaverage (gboolean OnOff);
void gok_page_actions_set_type (gboolean OnOff);
void gok_page_actions_set_is_corepointer (gboolean bCorePointer);
gboolean gok_page_actions_get_changed (void);
void gok_page_actions_set_changed (gboolean bTrueFalse);

