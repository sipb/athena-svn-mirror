/* findui.c
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
#include "findui.h"
#include "SRMessages.h"
#include <glade/glade.h>
#include "srintl.h"
#include "gnopiui.h"

#define FIND_STR(X) (X != NULL ? X : "")
#define CASE_SENSITIVE(X) (X == TRUE ? "CASE_SENSITIVE" : "NON_CASE_SENSITIVE")

typedef enum
{
    F_TEXT,
    F_ATTRIBUTE,
    F_GRAPHICS,
    F_INVALID
}FindType;


typedef enum
{
    F_BOLD,
    F_ITALIC,
    F_UNDERLINE,
    F_STRIKETHROUGH,
    F_SELECTED,
    F_NUM_OF_ATTR
} FindAttr;

typedef enum
{
    F_SCOPE_WINDOW,
    F_SCOPE_APPLICATION,
    F_SCOPE_DESKTOP,
    F_SCOPE_NUMBER
}FindScope;

typedef enum
{
    F_CRITERIA_AND,
    F_CRITERIA_OR,
    F_CRITERIA_NUMBER
}FindCriteria;


static GtkWidget *w_find = NULL;
static GtkWidget *rb_find_type 	    [ F_INVALID ];
static GtkWidget *rb_find_scope     [ F_SCOPE_NUMBER];
static GtkWidget *rb_search_criteria [ F_CRITERIA_NUMBER];
static GtkWidget *ck_find_attribute [ F_NUM_OF_ATTR ];
static GtkWidget *ck_case_sensitive;
static GtkWidget *tb_text;
static GtkWidget *tb_attribute;
static GtkWidget *tb_search_criteria;
static GtkWidget *et_find;
extern GtkWidget *w_assistive_preferences;

gchar*
fnui_find_add_atribute (FindAttr attr,
			gchar *str)
{
    gchar *retval = NULL;
    if (str)
    {
	switch (attr)
	{
	    case F_BOLD:retval = g_strdup_printf ("%s BOLD",str);break;
	    case F_ITALIC:retval = g_strdup_printf ("%s ITALIC",str);break;
	    case F_UNDERLINE:retval = g_strdup_printf ("%s UNDELINE",str);break;
	    case F_STRIKETHROUGH:retval = g_strdup_printf ("%s STRIKETROUGH",str);break;
	    case F_SELECTED:retval = g_strdup_printf ("%s SELECTED",str);break;
	    default:
		break;
	}
	g_free (str);
	str = NULL;
    }
    else
    {
	switch (attr)
	{
	    case F_BOLD:retval = g_strdup ("BOLD");break;
	    case F_ITALIC:retval = g_strdup ("ITALIC");break;
	    case F_UNDERLINE:retval = g_strdup ("UNDELINE");break;
	    case F_STRIKETHROUGH:retval = g_strdup ("STRIKETROUGH");break;
	    case F_SELECTED:retval = g_strdup ("SELECTED");break;
	    default:
		break;
	}
    }
    return retval;
}

gchar*
fnui_create_find_tag (FindType type, 
		      const gchar *text, 
		      FindScope scope)
{
    gchar *retval = NULL;
    gchar *r_scope  = NULL;
    const gchar *search_criteria [2] = {"AND" , "OR"};
    
    switch (scope)
    {
	case F_SCOPE_WINDOW:	
		r_scope = g_strdup ("WINDOW:");
		break;
	case F_SCOPE_APPLICATION:
		r_scope = g_strdup ("APPLICATION:");
		break;
	case F_SCOPE_DESKTOP:
		r_scope = g_strdup ("DESKTOP:");
		break;
	default:
		r_scope = NULL;
		break;
    }
    
    switch (type)
    {
	case F_TEXT:
	    {
		retval = g_strdup_printf ("%sTEXT:%s:%s", 
					    r_scope, 
					    CASE_SENSITIVE (
					    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_case_sensitive))),
					    g_strstrip ( (gchar*) FIND_STR (text)));
		break;
	    }
	case F_ATTRIBUTE:	
	    {
		gchar *attribute = NULL;
		FindAttr iter;
		for (iter = F_BOLD ; iter < F_NUM_OF_ATTR ; iter++)
		{
		    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_find_attribute [iter])))
			attribute = fnui_find_add_atribute (iter, attribute);
		}
		
		retval = g_strdup_printf ("%sATTRIBUTE:%s:%s", 
					    r_scope, 
					    search_criteria [
					    gtk_toggle_button_get_active (
					    GTK_TOGGLE_BUTTON (
					    rb_search_criteria [F_CRITERIA_OR]
					    ))], 
					    FIND_STR (attribute));		    
		
		g_free (attribute);
		break;
	    }
	case F_GRAPHICS:	
		retval = g_strdup_printf ("%sGRAPHICS::", r_scope);
		break;
	default: 
		retval = NULL;
		break;
    }
    
    g_free (r_scope);
    
    return retval;
}

void
fnui_find_set (void)
{
    gint type;
    FindScope scope;
	
    for (scope = F_SCOPE_WINDOW; scope < F_SCOPE_NUMBER; scope++)
    {
        if (gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON (rb_find_scope [scope])))
    	break;
    }

    for ( type = F_TEXT  ; type < F_INVALID ; type++)
    {
        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_find_type [type])))
        {
            gchar *tag = 
		fnui_create_find_tag (type, gtk_entry_get_text ( GTK_ENTRY (et_find)), scope);
	    srcore_find_text_set ( tag );
	    g_free (tag);
	    break;
	}
    }
}

static void
fnui_set_sensitive (FindType find_type)
{
    switch (find_type)
    {
	case F_TEXT:
	{
	    gtk_widget_set_sensitive (tb_attribute, FALSE);
	    gtk_widget_set_sensitive (tb_search_criteria, FALSE);
	    gtk_widget_set_sensitive (tb_text, TRUE);
	    break;
	}
	
	case F_ATTRIBUTE:
	{
	    gtk_widget_set_sensitive (tb_attribute, TRUE);
	    gtk_widget_set_sensitive (tb_search_criteria, TRUE);
	    gtk_widget_set_sensitive (tb_text, FALSE);
	    break;
	}

	case F_GRAPHICS:
	default:
	{
	    gtk_widget_set_sensitive (tb_attribute, FALSE);
	    gtk_widget_set_sensitive (tb_search_criteria, FALSE);
	    gtk_widget_set_sensitive (tb_text, FALSE);
	    break;
	}
    }
}

void
fnui_find_toggled   (GtkWidget       *button,
            	     gpointer         user_data)
{
    if (!strcmp((gchar*)user_data, "ATTRIBUTE")) 
        fnui_set_sensitive (F_ATTRIBUTE);
    if (!strcmp((gchar*)user_data, "GRAPHICS")) 
        fnui_set_sensitive (F_GRAPHICS);
    if (!strcmp((gchar*)user_data, "TEXT")) 
        fnui_set_sensitive (F_TEXT);
}


static void
fnui_response (GtkDialog *dialog,
	       gint       response_id,
	       gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_HELP)
    {
	gn_load_help ("gnopernicus-search-prefs");
	return;
    }
    if (response_id == GTK_RESPONSE_OK)
	fnui_find_set ();
    gtk_widget_hide ((GtkWidget*)dialog);
}


static gint
fnui_delete_emit_response_cancel (GtkDialog *dialog,
				  GdkEventAny *event,
				  gpointer data)
{
    gtk_dialog_response (GTK_DIALOG (dialog),
			 GTK_RESPONSE_CANCEL);
    return TRUE; /* Do not destroy */
}

void 
fnui_set_handlers_find (GladeXML *xml)
{
    w_find 				= glade_xml_get_widget (xml, "w_find");
    et_find				= glade_xml_get_widget (xml, "et_find");
    rb_find_type [F_TEXT] 		= glade_xml_get_widget (xml, "rb_text");
    rb_find_type [F_ATTRIBUTE]		= glade_xml_get_widget (xml, "rb_attribute");
    rb_find_type [F_GRAPHICS] 		= glade_xml_get_widget (xml, "rb_graphics");
    ck_find_attribute [F_BOLD]		= glade_xml_get_widget (xml, "ck_bold");
    ck_find_attribute [F_ITALIC]	= glade_xml_get_widget (xml, "ck_italic");
    ck_find_attribute [F_UNDERLINE]	= glade_xml_get_widget (xml, "ck_underline");
    ck_find_attribute [F_STRIKETHROUGH]	= glade_xml_get_widget (xml, "ck_strikethrough");
    ck_find_attribute [F_SELECTED]	= glade_xml_get_widget (xml, "ck_selected");
    rb_find_scope [F_SCOPE_WINDOW] 	= glade_xml_get_widget (xml, "rb_search_window");
    rb_find_scope [F_SCOPE_APPLICATION]	= glade_xml_get_widget (xml, "rb_search_application");
    rb_find_scope [F_SCOPE_DESKTOP] 	= glade_xml_get_widget (xml, "rb_search_desktop");
    rb_search_criteria [F_CRITERIA_AND] = glade_xml_get_widget (xml, "rb_search_and");
    rb_search_criteria [F_CRITERIA_OR] 	= glade_xml_get_widget (xml, "rb_search_or");
    tb_text 				= glade_xml_get_widget (xml, "table_text");
    tb_attribute 			= glade_xml_get_widget (xml, "table_attribute");
    tb_search_criteria 			= glade_xml_get_widget (xml, "table_search_criteria");
    ck_case_sensitive			= glade_xml_get_widget (xml, "ck_case_sensitive");

    g_signal_connect (w_find, "response",
		      G_CALLBACK (fnui_response), NULL);
    g_signal_connect (w_find, "delete_event",
                      G_CALLBACK (fnui_delete_emit_response_cancel), NULL);

    glade_xml_signal_connect_data (xml, "on_rb_attribute_toggled",		
			GTK_SIGNAL_FUNC(fnui_find_toggled), (gpointer)"ATTRIBUTE");
    glade_xml_signal_connect_data (xml, "on_rb_text_toggled",		
			GTK_SIGNAL_FUNC(fnui_find_toggled), (gpointer)"TEXT");
    glade_xml_signal_connect_data (xml, "on_rb_graphics_toggled",		
			GTK_SIGNAL_FUNC(fnui_find_toggled), (gpointer)"GRAPHICS");
}


void
fnui_value_add_to_widgets (void)
{
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_find_type [F_ATTRIBUTE]))) fnui_set_sensitive (F_ATTRIBUTE);
    else
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_find_type [F_TEXT]))) fnui_set_sensitive (F_TEXT);
    else
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_find_type [F_GRAPHICS]))) fnui_set_sensitive (F_GRAPHICS);
}
/**
 *
 * Find Settings interface loader function
 *
**/
gboolean 
fnui_load_find (void)
{
    if (!w_find)
    {
	GladeXML *xml;
	
	xml = gn_load_interface ("Find/find.glade2", "w_find");
	sru_return_val_if_fail (xml, FALSE);
	fnui_set_handlers_find (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_find),
			              GTK_WINDOW (w_assistive_preferences));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_find), TRUE);
    }
    else
	gtk_widget_show (w_find);
    

    fnui_value_add_to_widgets ();
    
    return TRUE;
}
