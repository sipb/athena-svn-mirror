/* scrui.c
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
#include "scrui.h"
#include "screen-review.h"
#include "SRMessages.h"
#include "libsrconf.h"
#include <glade/glade.h>
#include "srintl.h"
#include "gnopiui.h"
#include "coreconf.h"

static GtkWidget 	*w_screen_review;
static GtkWidget	*ck_horiz_leading,
			*ck_horiz_embedded,
			*ck_horiz_trailing,
			*ck_all_horizontal,
			*ck_vert_leading,
			*ck_vert_embedded,
			*ck_vert_trailing,
			*ck_all_vertical,
			*ck_vert_count,
			*ck_all;

gboolean lock = FALSE;

static glong
srcui_read_value (void)
{
    glong value = 0;
    
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_horiz_leading)))
	value = value | SRW_ALIGNF_HSP_ADD_LEADING;    
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_horiz_embedded)))
	value = value | SRW_ALIGNF_HSP_ADD_EMBEDDED;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_horiz_trailing)))
    	value = value | SRW_ALIGNF_HSP_ADD_TRAILING;

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_vert_leading)))
	value = value | SRW_ALIGNF_VSP_ADD_LEADING;    
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_vert_embedded)))
	value = value | SRW_ALIGNF_VSP_ADD_EMBEDDED;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_vert_trailing)))
    	value = value | SRW_ALIGNF_VSP_ADD_TRAILING;

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_vert_count)))
	value = value | SRW_ALIGNF_VSP_COUNT_LINES;

    return value;
}

static void
srcui_write_value (gint value)
{
    lock = TRUE;        
    
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_all), (value == SRW_ALIGNF_ALL));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_all_vertical), (SRW_ALIGNF_VSP_ADD == (value & SRW_ALIGNF_VSP_ADD)));        
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_all_horizontal), (SRW_ALIGNF_HSP_ADD == (value & SRW_ALIGNF_HSP_ADD)));	
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_horiz_leading),  value & SRW_ALIGNF_HSP_ADD_LEADING);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_horiz_embedded), value & SRW_ALIGNF_HSP_ADD_EMBEDDED);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_horiz_trailing), value & SRW_ALIGNF_HSP_ADD_TRAILING);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_vert_leading),   value & SRW_ALIGNF_VSP_ADD_LEADING);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_vert_embedded),  value & SRW_ALIGNF_VSP_ADD_EMBEDDED);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_vert_trailing),  value & SRW_ALIGNF_VSP_ADD_TRAILING);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_vert_count),     value & SRW_ALIGNF_VSP_COUNT_LINES);
    gtk_widget_set_sensitive (GTK_WIDGET (ck_vert_count), (value & SRW_ALIGNF_VSP_ADD));    
    
    lock = FALSE;
}

static void
scrui_response (GtkDialog *dialog,
		gint       response_id,
		gpointer   user_data)
{
    switch (response_id)
    {
        case GTK_RESPONSE_OK: 
	    srcore_screen_review_set (srcui_read_value ());
	    gtk_widget_hide ((GtkWidget*)dialog);
        break;
	case GTK_RESPONSE_HELP:
	    gn_load_help ("gnopernicus-screenreview-prefs");
	break;
        default:
	    gtk_widget_hide ((GtkWidget*)dialog);
        break;
    }
}

static gint
scrui_delete_emit_response_cancel (GtkDialog *dialog,
				    GdkEventAny *event,
				    gpointer data)
{
    gtk_dialog_response (GTK_DIALOG (dialog),
			 GTK_RESPONSE_CANCEL);
    return TRUE; /* Do not destroy */
}

static void
scrui_all_item_toggled (GtkWidget *widget,
			gpointer  user_data)
{
    gboolean toggle_value =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
	
    if (lock) 
	return;
    
    if (!strcmp ((gchar*)user_data,"ALL_HORIZ"))
    {
	glong value;
	
	lock = TRUE;
	
	value = srcui_read_value ();
	
	if (toggle_value)
	    value = value | SRW_ALIGNF_HSP_ADD;
	else
	    value = value & (~SRW_ALIGNF_HSP_ADD);
	srcui_write_value (value);

	lock = FALSE;
    }
    else
    if (!strcmp ((gchar*)user_data,"ALL_VERTIC"))
    {
	glong value;
	
	lock = TRUE;
	
	value = srcui_read_value ();
	
	if (toggle_value)
	    value = value | SRW_ALIGNF_VSP_ADD;
	else
	    value = value & (~SRW_ALIGNF_VSP_ADD);
	    
	srcui_write_value (value);

	lock = FALSE;
    }
    else
    if (!strcmp ((gchar*)user_data,"ALL"))
    {
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_all_vertical),
				     toggle_value); 
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_all_horizontal),
				     toggle_value); 
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_vert_count),
				     toggle_value); 
    }
    else
    {
	lock = TRUE;
	
	srcui_write_value ( srcui_read_value ());
	
	lock = FALSE;
    }
}

static void 
scrui_set_handlers (GladeXML *xml)
{    
    w_screen_review = glade_xml_get_widget (xml, "w_screen_review");

    ck_horiz_leading  = glade_xml_get_widget (xml, "ck_horiz_leading");
    ck_horiz_embedded = glade_xml_get_widget (xml, "ck_horiz_embedded");
    ck_horiz_trailing = glade_xml_get_widget (xml, "ck_horiz_trailing");
    ck_all_horizontal = glade_xml_get_widget (xml, "ck_all_horizontal");
    
    ck_vert_leading  = glade_xml_get_widget (xml, "ck_vert_leading");
    ck_vert_embedded = glade_xml_get_widget (xml, "ck_vert_embedded");
    ck_vert_trailing = glade_xml_get_widget (xml, "ck_vert_trailing");
    ck_all_vertical  = glade_xml_get_widget (xml, "ck_all_vertical");
    ck_vert_count    = glade_xml_get_widget (xml, "ck_vert_count");
    ck_all 	     = glade_xml_get_widget (xml, "ck_all");
            
    g_signal_connect (w_screen_review, "response",
		      G_CALLBACK (scrui_response), NULL);
    g_signal_connect (w_screen_review, "delete_event",
                      G_CALLBACK (scrui_delete_emit_response_cancel), NULL);

    glade_xml_signal_connect_data (xml,"on_ck_all_horizontal_toggled",			
			    GTK_SIGNAL_FUNC (scrui_all_item_toggled),
			    (gpointer)"ALL_HORIZ");
    glade_xml_signal_connect_data (xml,"on_ck_all_vertical_toggled",			
			    GTK_SIGNAL_FUNC (scrui_all_item_toggled),
			    (gpointer)"ALL_VERTIC");
    glade_xml_signal_connect_data (xml,"on_ck_all_toggled",			
			    GTK_SIGNAL_FUNC (scrui_all_item_toggled),
			    (gpointer)"ALL");

    glade_xml_signal_connect_data (xml,"on_ck_horiz_leading_toggled",			
			    GTK_SIGNAL_FUNC (scrui_all_item_toggled),
			    (gpointer)"HFLAGS");

    glade_xml_signal_connect_data (xml,"on_ck_horiz_embedded_toggled",			
			    GTK_SIGNAL_FUNC (scrui_all_item_toggled),
			    (gpointer)"HFLAGS");

    glade_xml_signal_connect_data (xml,"on_ck_horiz_trailing_toggled",			
			    GTK_SIGNAL_FUNC (scrui_all_item_toggled),
			    (gpointer)"HFLAGS");

    glade_xml_signal_connect_data (xml,"on_ck_vert_leading_toggled",			
			    GTK_SIGNAL_FUNC (scrui_all_item_toggled),
			    (gpointer)"VFLAGS");

    glade_xml_signal_connect_data (xml,"on_ck_vert_embedded_toggled",			
			    GTK_SIGNAL_FUNC (scrui_all_item_toggled),
			    (gpointer)"VFLAGS");

    glade_xml_signal_connect_data (xml,"on_ck_vert_trailing_toggled",			
			    GTK_SIGNAL_FUNC (scrui_all_item_toggled),
			    (gpointer)"VFLAGS");

    glade_xml_signal_connect_data (xml,"on_ck_vert_count_toggled",			
			    GTK_SIGNAL_FUNC (scrui_all_item_toggled),
			    (gpointer)"VFLAGS");

}

gboolean 
scrui_load_screen_review (GtkWidget *parent_window)
{
    if (!w_screen_review)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Screen_Review/screen_review.glade2", "w_screen_review");
	sru_return_val_if_fail (xml, FALSE);
	scrui_set_handlers  (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_screen_review),
				      GTK_WINDOW (parent_window));
				    
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_screen_review), TRUE);
    }
    else
	gtk_widget_show (w_screen_review);
                    
	
    srcui_write_value ( srcore_screen_review_get () );
    
    return TRUE;
}
