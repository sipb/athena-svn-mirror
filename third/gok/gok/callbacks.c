/* callbacks.c
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <gdk/gdkx.h>
#ifdef HAVE_XINPUT
#include <X11/extensions/XInput.h>
#endif

#include <X11/Xatom.h>
#include <glade/glade.h>
#include "callbacks.h"
#include "gok-scanner.h"
#include "gok-input.h"
#include "gok-gconf-keys.h"
#include "gok-page-keysizespace.h"
#include "gok-page-actions.h"
#include "gok-page-wordcomplete.h"
#include "gok-page-accessmethod.h"
#include "gok-page-feedbacks.h"
#include "gok-settings-dialog.h"
#include "gok-log.h"
#include "main.h"
#include "gok-editor.h"
#include "gok-spy.h"
#include "gok-data.h"

static GdkFilterReturn
gok_xkb_filter (GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data)
{
	XkbEvent *xevent = gdk_xevent;

	if (xevent->any.type == gok_xkb_base_event_type)
	{
		gok_keyboard_notify_xkb_event (xevent);
	}
	return GDK_FILTER_CONTINUE;
}

static GdkFilterReturn
gok_input_extension_filter (GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data)
{
	XEvent *xevent = gdk_xevent;
#ifdef HAVE_XINPUT
	if (xevent->type == gok_input_types[GOK_INPUT_TYPE_MOTION])
	{
		XDeviceMotionEvent *motion = (XDeviceMotionEvent *) xevent;
		gok_main_motion_listener (motion->axes_count, motion->axis_data, 
					  motion->device_state, (long) motion->time);
	}
	else if ((xevent->type == gok_input_types[GOK_INPUT_TYPE_BUTTON_PRESS]) || 
		 (xevent->type == gok_input_types[GOK_INPUT_TYPE_BUTTON_RELEASE]))
	{
		XDeviceButtonEvent *button = (XDeviceButtonEvent *) xevent;
		gok_main_button_listener (button->button, 
					  (xevent->type == gok_input_types[GOK_INPUT_TYPE_BUTTON_PRESS]) ? 1 : 0, 
					  button->state, 
					  button->time);
	}
	else 
	{
		return GDK_FILTER_CONTINUE;
	}
	return GDK_FILTER_REMOVE;
#else
	return GDK_FILTER_CONTINUE;
#endif
}

void
on_window1_destroy                     (GtkObject       *object,
                                        gpointer         user_data)
{
	/* this must be done before calling SPI_event_quit */
	gok_spy_deregister_mousebuttonlistener ((void *)gok_main_button_listener);

	SPI_event_quit ();
}

void
on_window1_realize                     (GtkWidget       *widget,
                                        gpointer         user_data)
{
  gboolean is_dock = (gboolean) user_data;
  GdkKeymap *map = gdk_keymap_get_default ();

  if (is_dock) {
	  gok_main_set_wm_dock (TRUE);
  }
  else {
	  if (widget->window) {
		  gdk_window_set_decorations (widget->window, GDK_DECOR_ALL | 
					      GDK_DECOR_MINIMIZE | GDK_DECOR_MAXIMIZE);
		  gdk_window_set_functions (widget->window, GDK_FUNC_MOVE | GDK_FUNC_RESIZE);
	  }
  }

  gtk_window_stick (GTK_WINDOW (widget));

  if (!gok_keyboard_xkb_select (GDK_WINDOW_XDISPLAY (widget->window)))
	  g_warning ("Could not register for XKB events!");

  gok_input_init (gok_input_get_current (), gok_input_extension_filter);

  gdk_window_add_filter (NULL,
			 gok_xkb_filter,
			 NULL);

  g_signal_connect (map, "keys-changed",
		    (GCallback) gok_spy_keymap_listener, NULL);
}


gboolean
on_window1_button_press_event          (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
        GokKey *pKey = (GokKey *) user_data;
	if (event->button == 1 && 
	    !gok_scanner_current_state_uses_core_mouse_button (1))
	{
	        gok_feedback_set_selected_key (pKey);
		gok_keyboard_output_selectedkey ();
	}
	return FALSE;
}

gboolean
on_window1_button_release_event          (GtkWidget       *widget,
					  GdkEventButton  *event,
					  gpointer         user_data)
{
	return FALSE;
}


gboolean
on_window1_button_toggle_event          (GtkWidget       *widget,
					 gpointer         user_data)
{
        GokKey *pKey = (GokKey *) user_data;
        gok_key_update_toggle_state (pKey);
	return FALSE;
}


gboolean
on_editor_button_press_event          (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	gok_editor_keyboard_key_press (widget);
	return FALSE;
}

gboolean
on_window1_motion_notify_event         (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data)
{	
	int x, y;
	x = event->x;
	y = event->y;

	gok_scanner_mouse_movement (x, y);
	return FALSE;
}

gboolean
on_window1_enter_notify_event         (GtkWidget        *widget,
				       GdkEventCrossing *event,
				       gpointer          user_data)
{	
        GdkCursor *cursor;
        if (gok_data_get_drive_corepointer ()) {
	  gok_data_set_drive_corepointer (FALSE);
	  gok_main_set_cursor (NULL);
	}
	else {
	  cursor = gdk_cursor_new (GDK_ARROW);
	  gok_main_set_cursor (cursor);
	  gdk_cursor_destroy (cursor);
	}
	return FALSE;
}

gboolean
on_window1_leave_notify_event         (GtkWidget        *widget,
				       GdkEventCrossing *event,
				       gpointer          user_data)
{	
       return FALSE;
}

void
on_window1_size_allocate               (GtkWidget       *widget,
                                        GtkAllocation   *allocation,
                                        gpointer         user_data)
{
        gok_log ("SizeA x = %d, y = %d, width = %d, height = %d\n", allocation->x, allocation->y, allocation->width, allocation->height); 
	gok_keyboard_on_window_resize ();
}


gboolean
on_window1_configure_event        (GtkWidget       *widget,
                                        GdkEventConfigure  *event,
                                        gpointer         user_data)
{
	gok_main_store_window_center();
	return FALSE;
	
}


void
on_entryName_changed                   (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gok_page_accessmethod_method_changed ((GtkEditable*)editable);
}


gboolean
on_dialogSettings_delete_event         (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	/* don't close the dialog, just hide it */
	gok_settingsdialog_hide();
	return TRUE;
}

gboolean
on_window1_delete_event         (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gok_main_store_window_center();
	return FALSE;
}

gboolean
on_dialogSettings_destroy_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  return FALSE;
}

void
on_buttonKeySize_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{

}

void
on_spinSpacing_changed                 (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gint spacing;
	char* pString;

	pString = gtk_editable_get_chars (editable, 0, -1);
	if (strlen (pString) != 0)
	{
		spacing = atoi (pString);
		gok_settings_page_keysizespace_display_keysizespacing (-1, -1, spacing);
	}
	g_free (pString);
}


void
on_spinWidth_changed                   (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gint width;
	char* pString;

	pString = gtk_editable_get_chars (editable, 0, -1);
	if (strlen (pString) != 0)
	{
		width = atoi (pString);
		gok_settings_page_keysizespace_display_keysizespacing (width, -1, -1);
	}
	g_free (pString);
}


void
on_spinHeight_changed                  (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gint height;
	char* pString;

	pString = gtk_editable_get_chars (editable, 0, -1);
	if (strlen (pString) != 0)
	{
		height = atoi (pString);
		gok_settings_page_keysizespace_display_keysizespacing (-1, height, -1);
	}
	g_free (pString);
}



void
on_radiobuttonTypeSwitch_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gok_page_actions_toggle_type_switch (gtk_toggle_button_get_active (togglebutton));
}


void
on_radiobuttonTypeValuator_toggled (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gok_page_actions_toggle_type_valuator (gtk_toggle_button_get_active (togglebutton));
}


void
on_entryActionName_changed               (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gok_page_actions_action_changed ((GtkEditable*)editable);
}

void
on_entry_input_device_changed          (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gok_page_actions_input_device_changed ((GtkEditable*)editable);
}

void
on_buttonNewAction_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_page_actions_button_clicked_new ();
}

void
on_buttonDeleteAction_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_page_actions_button_clicked_delete ();
}

void
on_buttonChangeName_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_page_actions_button_clicked_change_name();
}

void
on_buttonAccessMethodWizard_clicked    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget* pDialog;
	
	pDialog = gtk_message_dialog_new ((GtkWindow*)gok_settingsdialog_get_window(),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		_("Sorry, Access Method Wizard not implemented yet."));
	
	gtk_window_set_title (GTK_WINDOW (pDialog), _("GOK Access Method Wizard"));
	gtk_dialog_run (GTK_DIALOG (pDialog));
	gtk_widget_destroy (pDialog);
}



void
on_radiobuttonSwitch1_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if (gtk_toggle_button_get_active (togglebutton) == TRUE)
	{
		gok_page_actions_set_number(1);
	}
}

void
on_radiobuttonSwitch2_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if (gtk_toggle_button_get_active (togglebutton) == TRUE)
	{
		gok_page_actions_set_number(2);
	}
}


void
on_radiobuttonSwitch3_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if (gtk_toggle_button_get_active (togglebutton) == TRUE)
	{
		gok_page_actions_set_number(3);
	}
}


void
on_radiobuttonSwitch4_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if (gtk_toggle_button_get_active (togglebutton) == TRUE)
	{
		gok_page_actions_set_number(4);
	}
}


void
on_radiobuttonSwitch5_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if (gtk_toggle_button_get_active (togglebutton) == TRUE)
	{
		gok_page_actions_set_number(5);
	}
}


void
on_radiobuttonSwitchPress_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if (gtk_toggle_button_get_active (togglebutton) == TRUE)
	{
		gok_page_actions_set_state (ACTION_STATE_PRESS);
	}
}


void
on_radiobuttonSwitchRelease_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if (gtk_toggle_button_get_active (togglebutton) == TRUE)
	{
		gok_page_actions_set_state (ACTION_STATE_RELEASE);
	}
}

void
on_spinSwitchDelay_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gok_page_actions_set_rate (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(editable)));	
}

void
on_radiobuttonButtonPress_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if (gtk_toggle_button_get_active (togglebutton) == TRUE)
	{
		gok_page_actions_set_state (ACTION_STATE_PRESS);
	}
}


void
on_radiobuttonButtonRelease_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if (gtk_toggle_button_get_active (togglebutton) == TRUE)
	{
		gok_page_actions_set_state (ACTION_STATE_RELEASE);
	}
}


void
on_radiobuttonButtonClick_toggled      (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	if (gtk_toggle_button_get_active (togglebutton) == TRUE)
	{
		gok_page_actions_set_state (ACTION_STATE_CLICK);
	}
}

void
on_spinButtonDelay_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gok_page_actions_set_rate (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(editable)));	
}


void
on_spinDwellRate_changed               (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gok_page_actions_set_rate (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(editable)));	
}


void
on_checkKeyAverage_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gok_page_actions_pointer_keyaverage (gtk_toggle_button_get_active (togglebutton));
}


void
on_spinKeyAverage_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gok_page_actions_set_rate (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(editable)));	
}


void
on_file1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_new1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gok_editor_new_file();
}


void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gok_editor_open_file();
}


void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gok_editor_save_current_keyboard();
}


void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gok_editor_save_current_keyboard_as();
}


void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gok_editor_on_exit();
}


void
on_help1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_gok_editor_help1_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}

gboolean
on_windowEditor_delete_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	SPI_event_quit ();

  return FALSE;
}

void
on_buttonNext_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_editor_next_key();
}


void
on_buttonPrevious_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_editor_previous_key();
}


void
on_buttonAddNewKey_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_editor_add_key();
}


void
on_buttonDeleteKey_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_editor_delete_key();
}


void
on_buttonDuplicate_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_editor_duplicate_key();
}


void
on_spinLeft_insert_text                (GtkEditable     *editable,
                                        gchar           *new_text,
                                        gint             new_text_length,
                                        gint            *position,
                                        gpointer         user_data)
{
	gok_editor_update_key();

}


void
on_spinRight_insert_text               (GtkEditable     *editable,
                                        gchar           *new_text,
                                        gint             new_text_length,
                                        gint            *position,
                                        gpointer         user_data)
{
	gok_editor_update_key();

}


void
on_spinTop_insert_text                 (GtkEditable     *editable,
                                        gchar           *new_text,
                                        gint             new_text_length,
                                        gint            *position,
                                        gpointer         user_data)
{
	gok_editor_update_key();

}


void
on_spinBottom_insert_text              (GtkEditable     *editable,
                                        gchar           *new_text,
                                        gint             new_text_length,
                                        gint            *position,
                                        gpointer         user_data)
{
	gok_editor_update_key();

}



void
on_dialogSettings_show                 (GtkWidget       *widget,
                                        gpointer         user_data)
{
	/* this needs to be done the first time the settings dialog is shown */
	gok_settings_page_keysizespace_display_keysizespacing (-1, -1, -1);
}


void
on_buttonAddFeedback_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_page_feedbacks_button_clicked_new();
}


void
on_buttonDeleteFeedback_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_page_feedbacks_button_clicked_delete();
}


void
on_buttonChangeFeedbackName_clicked    (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_page_feedbacks_button_clicked_change_name();
}


void
on_buttonFeedbackSoundFile_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_page_feedbacks_get_sound_file();
}

void
on_entryFeedback_changed               (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gok_page_feedbacks_feedback_changed (editable);
}


void
on_checkKeyFlashing_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_page_feedbacks_check_keyflashing_clicked();
}


void
on_spinKeyFlashing_changed             (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gok_page_feedbacks_spin_keyflashing_changed();
}


void
on_checkSoundOn_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	gok_page_feedbacks_check_sound_clicked();
}

void
on_entrySoundName_changed              (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gok_page_feedbacks_entry_soundname_changed();
}

void
on_SpeakLabelCheckButton_toggled (GtkToggleButton *button)
{
	gok_page_feedbacks_check_speech_toggled (button);
}

void
on_notebook2_switch_page               (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        gint             page_num,
                                        gpointer         user_data)
{
	if (page_num == PAGE_NUM_ACCESS_METHODS)
	{
		gok_page_accessmethod_page_active();
	}
}


void
on_action_type_notebook_change_current_page
                                        (GtkNotebook     *notebook,
                                        gint             offset,
                                        gpointer         user_data)
{

}


void
on_2d_valuator_radiobutton_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	/* we always use 2d for now; the 1-d axis selection is an RFE */
}


void
on_1d_radiobutton_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	/* TODO */
}


void
on_axis_selection_spinbutton_changed   (GtkEditable     *editable,
                                        gpointer         user_data)
{
	/* TODO */
}


void
on_activate_on_enter_button_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gok_page_actions_set_type (ACTION_TYPE_MOUSEPOINTER);
}


void
on_activate_on_dwell_button_toggled    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gok_page_actions_set_type (ACTION_TYPE_DWELL);
}


void
on_activate_on_move_button_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gok_page_actions_set_type (ACTION_TYPE_VALUECHANGE);
}


void
on_pointer_delay_spinbutton_changed    (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gok_page_actions_set_rate (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(editable)));	
}


void
on_key_averaging_button_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gok_page_actions_pointer_keyaverage (gtk_toggle_button_get_active (togglebutton));
}

void
on_ValuatorSensitivityScale_value_changed (GtkRange *range,
					   gpointer  user_data)
{
	gok_data_set_valuator_sensitivity (gtk_range_get_value (range));
}

void
on_dock_checkbutton_toggled (GtkToggleButton *button, 
			     gpointer user_data)
{
	GtkWidget *dockbottom, *docktop;
	GokDockType dock_type;
	gboolean is_dock = gtk_toggle_button_get_active (button);
	GladeXML *xml = gok_settingsdialog_get_xml ();
	dockbottom = glade_xml_get_widget (xml, "DockBottomRadiobutton");
	docktop = glade_xml_get_widget (xml, "DockTopRadiobutton");

	gtk_widget_set_sensitive (docktop, is_dock);
	gtk_widget_set_sensitive (dockbottom, is_dock);

	if (is_dock)
	{
		if (docktop &&  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (docktop)))
		{
			dock_type = GOK_DOCK_TOP;
		}
		else
		{
			dock_type = GOK_DOCK_BOTTOM;
		}
	}
	else
	{
		dock_type = GOK_DOCK_NONE;
	}
	gok_data_set_dock_type (dock_type);
}

void
on_fill_checkbutton_toggled (GtkToggleButton *button, 
			     gpointer user_data)
{
	gok_data_set_expand (gtk_toggle_button_get_active (button));
}

void
on_dock_top_radiobutton_toggled (GtkToggleButton *button, 
				 gpointer user_data)
{
	if (gtk_toggle_button_get_active (button))
		gok_data_set_dock_type (GOK_DOCK_TOP);
}

void
on_dock_bottom_radiobutton_toggled (GtkToggleButton *button, 
				    gpointer user_data)
{
	if (gtk_toggle_button_get_active (button))
		gok_data_set_dock_type (GOK_DOCK_BOTTOM);
}

void
on_aux_keyboard_dir_entry_changed (GtkEntry *entry,
				   gpointer user_data)
{
	gok_data_set_aux_keyboard_directory (gtk_entry_get_text (entry));
}


void
on_xkb_compose_keyboard_radiobutton_toggled (GtkToggleButton *button, 
					     gpointer user_data)
{
	if (gtk_toggle_button_get_active (button))
		gok_data_set_compose_keyboard_type (GOK_COMPOSE_XKB);
}

void
on_alpha_compose_keyboard_radiobutton_toggled (GtkToggleButton *button, 
					       gpointer user_data)
{
	if (gtk_toggle_button_get_active (button))
		gok_data_set_compose_keyboard_type (GOK_COMPOSE_ALPHA);
}

void
on_freq_compose_keyboard_radiobutton_toggled (GtkToggleButton *button, 
					      gpointer user_data)
{
	if (gtk_toggle_button_get_active (button))
		gok_data_set_compose_keyboard_type (GOK_COMPOSE_ALPHAFREQ);
}

void
on_file_compose_keyboard_radiobutton_toggled (GtkToggleButton *button, 
					      gpointer user_data)
{
	if (gtk_toggle_button_get_active (button))
		gok_data_set_compose_keyboard_type (GOK_COMPOSE_CUSTOM);
}

void
on_compose_keyboard_file_entry_changed (GtkEditable *editable, 
					gpointer user_data)
{
	gok_data_set_custom_compose_filename (
		gtk_editable_get_chars (editable, 0, -1));
}

void
on_core_pointer_button_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gok_page_actions_set_is_corepointer (
		gtk_toggle_button_get_active (togglebutton));
}


void
on_xinput_device_button_toggled        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gok_page_actions_set_is_corepointer (
		!gtk_toggle_button_get_active (togglebutton));
}


void
on_joystick_button_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gok_page_actions_set_is_corepointer (
		!gtk_toggle_button_get_active (togglebutton));}


void
on_checkExtraWordList_toggled          (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gok_page_wordcomplete_toggle_wordlist (gtk_toggle_button_get_active (togglebutton));
}

void
on_buttonHelp_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	on_button_help (button, user_data);
}


void
on_buttonApply_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	on_button_try (button, user_data);
}


void
on_buttonRevert_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	on_button_revert (button, user_data);
}


void
on_buttonCancel_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	on_button_cancel (button, user_data);
}


void
on_buttonOK_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	on_button_ok (button, user_data);
}


