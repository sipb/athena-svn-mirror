/* defui.c
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
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

#include "config.h"
#include "defui.h"
#include "brlui.h"
#include "spui.h"
#include "kbui.h"
#include "magui.h"
#include "genui.h"
#include "gnopiui.h"
#include "bmui.h"

#include "brlconf.h"
#include "coreconf.h"
#include "kbconf.h"
#include "magconf.h"
#include "spconf.h"
#include "cmdmapconf.h"
#include "presconf.h"
#include "bmconf.h"
#include "SRMessages.h"

#include <glade/glade.h>
#include "srintl.h"

/**
 *
 * External interface widgets.
 *
**/
extern Braille   *braille_setting;
extern General   *general_setting;
extern Keyboard  *keyboard_setting;
extern Speech 	 *speech_setting;
extern Magnifier *magnifier_setting;

/**
 *
 * Default interface widgets
 *
**/
GtkWidget	*w_default_load;



static gboolean
defui_question_message (const gchar *msg)
{
    GtkWidget 	*dialog = NULL;
    gboolean 	rv = FALSE;
    gchar	*message = NULL;
    gint 	response;
    
    message = g_strdup_printf (_("Are you sure you want to load default values for %s?"), msg);
    
    dialog = gtk_message_dialog_new (
	    NULL,
	    GTK_DIALOG_DESTROY_WITH_PARENT,
	    GTK_MESSAGE_INFO,GTK_BUTTONS_YES_NO,
	    message);

    response = gtk_dialog_run (GTK_DIALOG (dialog));
    
    if (response == GTK_RESPONSE_YES)
	rv = TRUE;
    
    gtk_widget_destroy (dialog);
    
    g_free (message);
    
    return rv;
}


static void 
defui_default_braille_clicked	(GtkButton       *button,
                                 gpointer         user_data)
{
    if (*(gboolean*)user_data)
    {
	if (!defui_question_message (_("braille")))
	    return;
    }
    
    brlconf_load_default_settings (braille_setting);
    brlconf_setting_set (braille_setting);
    
    brlui_braille_device_value_add_to_widgets (braille_setting);
    brlui_translation_table_value_add_to_widgets (braille_setting);
    brlui_braille_style_value_add_to_widgets (braille_setting);
    brlui_cursor_setting_value_add_to_widgets (braille_setting);
    brlui_attribute_setting_value_add_to_widgets (braille_setting);
    brlui_braille_fill_char_value_add_to_widgets (braille_setting);
    brlui_status_cell_value_add_to_widgets (braille_setting);
    brlui_braille_key_mapping_value_add_to_widgets (braille_setting);
    if (*(gboolean*)user_data)
	gn_show_message(_("Braille default setting loaded."));    
}

static void
defui_default_speech_clicked (GtkButton       *button,
                              gpointer         user_data)
{
    if (*(gboolean*)user_data)
    {
	if (!defui_question_message (_("speech")))
	    return;
    }
    
    spconf_load_default_settings (speech_setting);
    spconf_setting_set (speech_setting);
    spui_speech_setting_value_add_to_widgets (speech_setting);
    if (*(gboolean*)user_data)
	gn_show_message (_("Speech default setting loaded."));
}

static void
defui_default_magnifier_clicked	(GtkButton       *button,
                                 gpointer         user_data)
{
    if (*(gboolean*)user_data)
    {
	if (!defui_question_message (_("magnifier")))
	    return;
    }

    if (magnifier_setting) 
    {
	magconf_setting_free (magnifier_setting);
	magnifier_setting = NULL;
    }
    magconf_setting_init (DEFAULT_MAGNIFIER_ID, TRUE);
    magconf_save_zoomer_in_schema (DEFAULT_MAGNIFIER_SCHEMA, magnifier_setting);
    magui_magnifier_setting_value_add_to_widgets (magnifier_setting);
    magui_magnification_options_value_add_to_widgets (magnifier_setting);
    if (*(gboolean*)user_data)
	gn_show_message(_("Magnifier default setting loaded."));
}

static void
defui_default_keyboard_clicked (GtkButton       *button,
                                gpointer         user_data)
{
    if (*(gboolean*)user_data)
    {
	if (!defui_question_message (_("keyboard")))
	    return;
    }

    kbconf_load_default_settings (keyboard_setting);
    kbconf_setting_set ();
    kbui_keyboard_settings_value_add_to_widgets (keyboard_setting);
    if (*(gboolean*)user_data)
	gn_show_message (_("Keyboard default setting loaded."));
}

static void
defui_default_braille_monitor_clicked (GtkButton       *button,
                            	       gpointer         user_data)
{
    if (*(gboolean*)user_data)
    {
	if (!defui_question_message (_("braille monitor")))
	    return;
    }

    bmconf_load_default_settings ();
    bmui_braille_monitor_values_add_to_widgets ();
    if (*(gboolean*)user_data)
	gn_show_message (_("Braille monitor default setting loaded."));
}

static void
defui_default_general_clicked (GtkButton       *button,
                               gpointer         user_data)
{
    if (*(gboolean*)user_data)
    {
	if (!defui_question_message (_("start up")))
	    return;
    }

    srcore_load_default_settings (general_setting);
    srcore_general_setting_set   (general_setting);
    genui_value_add_to_widgets 	 (general_setting);
    
    if (*(gboolean*)user_data)
	gn_show_message (_("Startup default setting loaded."));
}


static void
defui_default_cmdmap_clicked (GtkButton       *button,
                             gpointer         user_data)
{
    if (*(gboolean*)user_data)
    {
	if (!defui_question_message (_("command map")))
	    return;
    }
    
    cmdconf_remove_lists_from_gconf ();
    cmdconf_default_list (TRUE);
    cmdconf_changes_end_event ();
    if (*(gboolean*)user_data)
	gn_show_message (_("Command map default setting loaded."));
}

static void
defui_default_pres_clicked (GtkButton       *button,
                            gpointer         user_data)

{
    if (*(gboolean*)user_data)
    {
	if (!defui_question_message (_("presentation")))
	    return;
    }

    presconf_set_defaults ();
    if (*(gboolean*)user_data)
	gn_show_message (_("Presentation default setting loaded."));
}

static void
defui_default_scr_rev_clicked (GtkButton       *button,
                               gpointer         user_data)
{
    if (*(gboolean*)user_data)
    {
	if (!defui_question_message (_("screen review")))
	    return;
    }

    srcore_load_default_screen_review ();
    if (*(gboolean*)user_data)
	gn_show_message (_("Screen review default setting loaded."));
}

void
defui_load_all_default (void)
{
    gboolean bval = FALSE;
    
    defui_default_braille_clicked (NULL, (gpointer)&bval);
    defui_default_braille_monitor_clicked (NULL, (gpointer)&bval);
    defui_default_speech_clicked (NULL, (gpointer)&bval);
    defui_default_keyboard_clicked (NULL, (gpointer)&bval);
    defui_default_magnifier_clicked (NULL, (gpointer)&bval);
    defui_default_general_clicked (NULL, (gpointer)&bval);
    defui_default_cmdmap_clicked (NULL, (gpointer)&bval);
    defui_default_pres_clicked (NULL, (gpointer)&bval);
    defui_default_scr_rev_clicked (NULL, (gpointer)&bval);
}

static void
defui_default_all_clicked (GtkButton       *button,
                           gpointer         user_data)
{
    if (!defui_question_message (_("all modules")))
	return;
    defui_load_all_default ();
    gn_show_message (_("All default settings loaded."));
}

void
defui_default_close_clicked (GtkButton       *button,
                             gpointer         user_data)
{
    gtk_widget_hide (w_default_load);
}

void
defui_default_remove (GtkButton       *button,
                      gpointer         user_data)
{
    gtk_widget_hide (w_default_load);
    w_default_load = NULL;
}


/**
 *
 * Set event handlers and get a widgets used in this interface.
 * xml - glade interface XML pointer
 *
**/
void 
defui_set_handlers_load_default	(GladeXML *xml)
{
    static gboolean bval = TRUE;
    w_default_load = glade_xml_get_widget (xml, "w_default_load");
    
    glade_xml_signal_connect_data (xml,"on_w_default_load_remove",		
				    GTK_SIGNAL_FUNC (defui_default_remove),
				    (gpointer)&bval);
    glade_xml_signal_connect_data (xml,"on_bt_default_braille_clicked",		
				    GTK_SIGNAL_FUNC (defui_default_braille_clicked),
				    (gpointer)&bval);
    glade_xml_signal_connect_data (xml,"on_bt_default_speech_clicked",		
				    GTK_SIGNAL_FUNC (defui_default_speech_clicked),
				    (gpointer)&bval);
    glade_xml_signal_connect_data (xml,"on_bt_default_magnifier_clicked",		
				    GTK_SIGNAL_FUNC (defui_default_magnifier_clicked),
				    (gpointer)&bval);
    glade_xml_signal_connect_data (xml,"on_bt_default_braille_monitor_clicked",		
				    GTK_SIGNAL_FUNC (defui_default_braille_monitor_clicked),
				    (gpointer)&bval);
    glade_xml_signal_connect_data (xml,"on_bt_default_keyboard_clicked",		
				    GTK_SIGNAL_FUNC (defui_default_keyboard_clicked),
				    (gpointer)&bval);
    glade_xml_signal_connect_data (xml,"on_bt_default_general_clicked",		
				    GTK_SIGNAL_FUNC (defui_default_general_clicked),
				    (gpointer)&bval);
    glade_xml_signal_connect_data (xml,"on_bt_default_cmdmap_clicked",		
				    GTK_SIGNAL_FUNC (defui_default_cmdmap_clicked),
				    (gpointer)&bval);
    glade_xml_signal_connect_data (xml,"on_bt_default_presentation_clicked",		
				    GTK_SIGNAL_FUNC (defui_default_pres_clicked),
				    (gpointer)&bval);
    glade_xml_signal_connect_data (xml,"on_bt_default_screen_review_clicked",		
				    GTK_SIGNAL_FUNC (defui_default_scr_rev_clicked),
				    (gpointer)&bval);
    glade_xml_signal_connect_data (xml,"on_bt_default_all_clicked",			
				    GTK_SIGNAL_FUNC (defui_default_all_clicked),
				    (gpointer)&bval);
    glade_xml_signal_connect_data (xml,"on_bt_default_close_clicked",			
				    GTK_SIGNAL_FUNC (defui_default_close_clicked),
				    (gpointer)&bval);
}

/**
 *
 * Load Default Settings inetrface loader function
 *
**/
gboolean 
defui_load_default_load (GtkWidget *parent_window)
{
    if (!w_default_load)
    {
	GladeXML *xml;
	
	xml = gn_load_interface ("Load_Default/load_default.glade2", "w_default_load");
	sru_return_val_if_fail (xml, FALSE);
	defui_set_handlers_load_default (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for ( GTK_WINDOW (w_default_load),
				   GTK_WINDOW (parent_window));
	gtk_window_set_destroy_with_parent ( GTK_WINDOW (w_default_load), 
					TRUE);
    }
    else
	gtk_widget_show (w_default_load);
        
    return TRUE;
}


