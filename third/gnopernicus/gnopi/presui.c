/* presui.c
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
#include "presui.h"
#include "SRMessages.h"
#include <glade/glade.h>
#include "srintl.h"
#include "gnopiui.h"
#include "libsrconf.h"

#define PRESENTATION_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define PRESENTATION_GET_OBJECT(component, name) \
  g_object_get_data (G_OBJECT (component), name)

static GtkWidget *w_presentation;


void
presui_presentation_changed (GtkWidget *widget,
			     gpointer  user_data)
{
    gchar *active = (gchar*)gtk_entry_get_text (GTK_ENTRY (widget));

    if (active && strlen (active) > 0)
	presconf_active_setting_set (active);
}
void
presui_presentation_fill_combo (GSList *list)
{
    GtkCombo *combo = (GtkCombo*)PRESENTATION_GET_OBJECT(w_presentation, "combo");
    GList    *tmp = NULL;
    
    if (!list) 
	tmp = g_list_append (tmp , "");	

    for ( ;list ; list = list->next)
	tmp = g_list_append (tmp, list->data);
	
    gtk_combo_set_popdown_strings (GTK_COMBO (combo), tmp);
    
    g_list_free (tmp);
}

static void
presui_add_value_to_widgets (void)
{
    GSList *list = presconf_all_settings_get ();
    GSList *tmp  = NULL;
    gchar  *active = presconf_active_setting_get ();
    GtkCombo *combo = (GtkCombo*)PRESENTATION_GET_OBJECT(w_presentation, "combo");

        
    presui_presentation_fill_combo (list);

    presconf_check_if_setting_in_list (list, active, &tmp);
    if (tmp)
    {
        sru_assert (tmp);
	gtk_entry_set_text (GTK_ENTRY (combo->entry), (gchar*)tmp->data);
    }
    g_free (active);
    
    for (tmp = list ; tmp ; tmp = tmp->next)
	g_free (tmp->data);	
    g_slist_free (list);
}

static void
presui_response_event (GtkDialog *dialog,
			gint       response_id,
			gpointer   user_data)
{
     if (response_id == GTK_RESPONSE_HELP)
     {
	gn_load_help ("gnopernicus-presentation-prefs");
	return;
     }

    gtk_widget_hide ((GtkWidget*)dialog);
}

static gint
presui_delete_emit_response_cancel (GtkDialog *dialog,
				  GdkEventAny *event,
				  gpointer data)
{
    gtk_dialog_response (GTK_DIALOG (dialog),
			 GTK_RESPONSE_CLOSE);
    return TRUE; /* Do not destroy */
}


static void 
presui_set_handlers (GladeXML *xml)
{    
    w_presentation 	= glade_xml_get_widget (xml, "w_presentation");
    
    PRESENTATION_HOOKUP_OBJECT(w_presentation, glade_xml_get_widget (xml, "cb_presentation"), "combo");
    g_signal_connect (w_presentation, "response",
		      G_CALLBACK (presui_response_event), NULL);
    g_signal_connect (w_presentation, "delete_event",
                      G_CALLBACK (presui_delete_emit_response_cancel), NULL);

    glade_xml_signal_connect (xml,"on_et_cb_presentation_changed",		
			    GTK_SIGNAL_FUNC (presui_presentation_changed));
}


gboolean 
presui_load_presentation (GtkWidget *parent_window)
{
    
        
    if (!w_presentation)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Presentation/presentation.glade2", "w_presentation");
	sru_return_val_if_fail (xml, FALSE);
	presui_set_handlers  (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for ( GTK_WINDOW (w_presentation),
				           GTK_WINDOW (parent_window));
				    
	gtk_window_set_destroy_with_parent ( GTK_WINDOW (w_presentation), 
					    	TRUE);
    }
    else
	gtk_widget_show (w_presentation);

    presui_add_value_to_widgets ();
    
    return TRUE;
}
