/* bmui.c
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
#include "bmui.h"
#include "gnopiui.h"
#include "SRMessages.h"
#include <glade/glade.h>
#include "srintl.h"

enum
{
    BRLMON_DOT7,
    BRLMON_DOT8,
    BRLMON_DOT78,
    BRLMON_NUMBER_OF_DOT_TYPES
};

typedef enum
{
    BRLMON_MODE_NORMAL,
    BRLMON_MODE_BRAILLE,
    BRLMON_MODE_DUAL,
    BRLMON_NUMBER_OF_MODE_TYPES
}BrlmonModeType;

typedef enum
{
    BRLMON_POSITION_TOP,
    BRLMON_POSITION_BOTTOM,
    BRLMON_NUMBER_OF_POSITION_TYPES
}BrlmonPositionType;
    
GtkWidget *w_braille_monitor;
static GtkWidget *cp_dots [BRLMON_NUMBER_OF_DOT_TYPES];
static GtkWidget *ck_theme_color;
static GtkWidget *sp_line;
static GtkWidget *sp_column;
static GtkWidget *rb_positions[BRLMON_NUMBER_OF_POSITION_TYPES];
static GtkWidget *rb_modes [BRLMON_NUMBER_OF_MODE_TYPES];
static GtkWidget *sp_font_size;
static GdkColor  colors [BRLMON_NUMBER_OF_DOT_TYPES];


static GdkColor  	  colors_backup [BRLMON_NUMBER_OF_DOT_TYPES];
static BrlmonPositionType position_backup;
static BrlmonModeType     mode_backup;
static gint 		  line_backup;
static gint 		  column_backup;
static gint 		  font_size_backup;
static gboolean           theme_color_backup;


#define POSITION_TOP_STRING 	"TOP"
#define POSITION_BOTTOM_STRING 	"BOTTOM"
#define MODE_NORMAL_STRING	"NORMAL"
#define MODE_BRAILLE_STRING	"BRAILLE"
#define MODE_DUAL_STRING	"DUAL"

#define LINE_MIN_NUMBER		1
#define LINE_MAX_NUMBER		3

#define COLUMN_MIN_NUMBER	1
#define COLUMN_MAX_NUMBER	40

#define MIN_FONT_SIZE		8
#define MAX_FONT_SIZE		30

/**
* bmui_get_string_from_position
*
* @position: BrlmonPositionType type;
*
* Return string of BrlmonPositionType (to_string).
*
* returns: string.
**/
const gchar*
bmui_get_string_from_position (BrlmonPositionType position)
{
    if (position == BRLMON_POSITION_TOP)
	return POSITION_TOP_STRING;
    return POSITION_BOTTOM_STRING;
}

/**
* bmui_get_string_from_mode_type
*
* @mode: BrlmonModeType type;
*
* Return string of BrlmonModeType (to_string).
*
* returns: string.
**/
const gchar*
bmui_get_string_from_mode_type (BrlmonModeType mode)
{
    switch (mode)
    {
	case BRLMON_MODE_BRAILLE:
	    return MODE_BRAILLE_STRING;
	case BRLMON_MODE_DUAL:
	    return MODE_DUAL_STRING;
	case BRLMON_MODE_NORMAL:
	default:
	    return MODE_NORMAL_STRING;
    }
}

/**
* bmui_save_values
*
* Save UI values to gconf.
*
* returns:
**/
void 
bmui_save_values (void)
{	
    gchar *dot7;
    gchar *dot8;
    gchar *dot78;
    
    gint iter;

    gnome_color_picker_get_i16 (GNOME_COLOR_PICKER (cp_dots[BRLMON_DOT78]),   &colors[0].red, &colors[0].green, &colors[0].blue, 0);
    gnome_color_picker_get_i16 (GNOME_COLOR_PICKER (cp_dots[BRLMON_DOT7]),    &colors[1].red, &colors[1].green, &colors[1].blue, 0);
    gnome_color_picker_get_i16 (GNOME_COLOR_PICKER (cp_dots[BRLMON_DOT8]),    &colors[2].red, &colors[2].green, &colors[2].blue, 0);
     
    dot78   = g_strdup_printf ("#%04x%04x%04x", colors[0].red,colors[0].green,colors[0].blue);
    dot7    = g_strdup_printf ("#%04x%04x%04x", colors[1].red,colors[1].green,colors[1].blue);
    dot8    = g_strdup_printf ("#%04x%04x%04x", colors[2].red,colors[2].green,colors[2].blue);

    bmconf_colors_set (dot7, dot8, dot78);
    
    g_free (dot78);
    g_free (dot7);
    g_free (dot8);

    bmconf_use_theme_color_set (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_theme_color)));

    bmconf_size_set (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_line)), 
		     gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_column)));
		     
    bmconf_font_size_set (gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_font_size)));
		     
    for (iter = BRLMON_POSITION_TOP; iter < BRLMON_NUMBER_OF_POSITION_TYPES; iter++)
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_positions[iter])))
	    break;
    bmconf_position_set (bmui_get_string_from_position (iter));
    
    for (iter = BRLMON_MODE_NORMAL; iter < BRLMON_NUMBER_OF_MODE_TYPES; iter++)
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_modes[iter])))
	    break;
    bmconf_display_mode_set (bmui_get_string_from_mode_type (iter));
}

/**
* bmui_restore_values
*
* Restore old UI values to gconf.
*
* returns:
**/
void 
bmui_restore_values (void)
{	
    gchar *dot7;
    gchar *dot8;
    gchar *dot78;
         
    dot78   = g_strdup_printf ("#%04x%04x%04x", colors_backup[0].red, colors_backup[0].green, colors_backup[0].blue);
    dot7    = g_strdup_printf ("#%04x%04x%04x", colors_backup[1].red, colors_backup[1].green, colors_backup[1].blue);
    dot8    = g_strdup_printf ("#%04x%04x%04x", colors_backup[2].red, colors_backup[2].green, colors_backup[2].blue);

    bmconf_colors_set (dot7, dot8, dot78);
    
    g_free (dot78);
    g_free (dot7);
    g_free (dot8);

    bmconf_size_set (line_backup, column_backup); 
    bmconf_font_size_set (font_size_backup); 
    bmconf_use_theme_color_set (theme_color_backup);
    bmconf_position_set (bmui_get_string_from_position (position_backup));
    bmconf_display_mode_set (bmui_get_string_from_mode_type (mode_backup));
}


static void
bmui_response_braille_monitor (GtkDialog *dialog,
			       gint       response_id,
			       gpointer   user_data)
{
    switch (response_id)
    {
	case GTK_RESPONSE_HELP:
	    gn_load_help ("gnopernicus-brlmon-prefs");
	    break;
	case GTK_RESPONSE_CANCEL:
	    bmui_restore_values ();
	    gtk_widget_hide ((GtkWidget*)dialog);
	    break;
	case GTK_RESPONSE_OK:
	    gtk_widget_hide ((GtkWidget*)dialog);
	case GTK_RESPONSE_APPLY:
	    bmui_save_values ();
	    break;
	default: 
	    gtk_widget_hide ((GtkWidget*)dialog);
	    break;
    }
}


static gint
bmui_delete_emit_response_cancel (GtkDialog *dialog,
				   GdkEventAny *event,
				   gpointer data)
{
    gtk_dialog_response (GTK_DIALOG (dialog),
			 GTK_RESPONSE_CANCEL);
    return TRUE; /* Do not destroy */
}

/**
* bmui_load_colors
*
* Load text colors.
*
* returns:
**/
#define COLOR_PARSE_ERROR_STRING N_("Can not parse color string.")
void
bmui_load_colors (void)
{
    gchar *dot7;
    gchar *dot8; 
    gchar *dot78; 
    
    bmconf_colors_get (&dot7, &dot8, &dot78);
    
    if (!gdk_color_parse (dot7, &colors[0]))
    	sru_warning (_(COLOR_PARSE_ERROR_STRING));
    if (!gdk_color_parse (dot78, &colors[1]))
    	sru_warning (_(COLOR_PARSE_ERROR_STRING));
    if (!gdk_color_parse (dot8, &colors[2]))
    	sru_warning (_(COLOR_PARSE_ERROR_STRING));

    g_free (dot7);
    g_free (dot8);
    g_free (dot78);
}

/**
* bmui_get_mode_type_from_string
*
* @mode: string type;
*
* Return BrlmonModeType from string.
*
* returns: position in BrlmonModeType
**/
BrlmonModeType
bmui_get_mode_type_from_string (const gchar *mode)
{
    gchar 		*tmp;
    BrlmonModeType 	modetype;
    
    tmp = g_ascii_strup (mode, -1);
    
    if (!strcmp (tmp, MODE_NORMAL_STRING))  modetype = BRLMON_MODE_NORMAL;
	else
    if (!strcmp (tmp, MODE_BRAILLE_STRING)) modetype = BRLMON_MODE_BRAILLE;
	else
    if (!strcmp (tmp, MODE_DUAL_STRING))    modetype = BRLMON_MODE_DUAL;
	else
    {
	sru_warning (_("Invalid modetype: %s."), mode);		    
	modetype = BRLMON_MODE_NORMAL;
    }
    
    g_free (tmp);
    
    return modetype;
}

/**
* bmui_get_position_from_string
*
* @position: string type;
*
* Return BrlmonPositionType from string.
*
* returns: position in BrlmonPositionType
**/
BrlmonPositionType
bmui_get_position_from_string (const gchar *position)
{
    gchar 	*tmp;
    BrlmonPositionType 	pos;
    
    tmp = g_ascii_strup (position, -1);
    
    if (!strcmp (tmp, POSITION_TOP_STRING))  pos = BRLMON_POSITION_TOP;
	else
    if (!strcmp (tmp, POSITION_BOTTOM_STRING)) pos = BRLMON_POSITION_BOTTOM;
	else
    {
	sru_warning (_("Invalid position: %s."), position);		    
	pos = BRLMON_POSITION_TOP;
    }
    
    g_free (tmp);
    
    return pos;
}

/**
* bmui_values_add_to_widgets
*
* Add values to widgets.
*
* returns: 
**/
void
bmui_braille_monitor_values_add_to_widgets (void)
{    
    gchar *mode = bmconf_display_mode_get ();
    gchar *position = bmconf_position_get ();
    gint iter;
    
    if (!w_braille_monitor)
	return;

    bmconf_size_get (&line_backup, &column_backup);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (sp_line), 	(gdouble)line_backup);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (sp_column), 	(gdouble)column_backup);
    
    font_size_backup = bmconf_font_size_get ();
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (sp_font_size), 	(gdouble)font_size_backup);
    
    mode_backup = bmui_get_mode_type_from_string (mode);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_modes[mode_backup]), TRUE);
    
    position_backup = bmui_get_position_from_string (position);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_positions[position_backup]), TRUE);

    theme_color_backup = bmconf_use_theme_color_get ();
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_theme_color), theme_color_backup);
    
    bmui_load_colors ();
    
    gnome_color_picker_set_i16 (GNOME_COLOR_PICKER (cp_dots[BRLMON_DOT7]),    colors[1].red, colors[1].green, colors[1].blue, 0);
    gnome_color_picker_set_i16 (GNOME_COLOR_PICKER (cp_dots[BRLMON_DOT78]),   colors[0].red, colors[0].green, colors[0].blue, 0);
    gnome_color_picker_set_i16 (GNOME_COLOR_PICKER (cp_dots[BRLMON_DOT8]),    colors[2].red, colors[2].green, colors[2].blue, 0);    

    for (iter = 0 ; iter < BRLMON_NUMBER_OF_DOT_TYPES; iter++)
	colors_backup[iter] = colors[iter];
	
    g_free (mode);
    g_free (position);
}

/**
* bmui_set_handlers_braille_monitor_settings
* @xml: Glade XML pointer
*
* Get the pointers from all widgets and assign a signals.
*
* returns: 
**/
static void
bmui_set_handlers_braille_monitor_settings (GladeXML *xml)
{
    cp_dots[BRLMON_DOT7]     = glade_xml_get_widget (xml,"cp_dot7");
    cp_dots[BRLMON_DOT78]    = glade_xml_get_widget (xml,"cp_dot78");
    cp_dots[BRLMON_DOT8]     = glade_xml_get_widget (xml,"cp_dot8");
    
    sp_line     = glade_xml_get_widget (xml,"sp_line");
    sp_column   = glade_xml_get_widget (xml,"sp_column");
    
    rb_positions[BRLMON_POSITION_TOP]     	= glade_xml_get_widget (xml,"rb_top");
    rb_positions[BRLMON_POSITION_BOTTOM]	= glade_xml_get_widget (xml,"rb_bottom");
    
    rb_modes[BRLMON_MODE_NORMAL]  = glade_xml_get_widget (xml,"rb_mode_normal");
    rb_modes[BRLMON_MODE_BRAILLE] = glade_xml_get_widget (xml,"rb_mode_braille");
    rb_modes[BRLMON_MODE_DUAL]    = glade_xml_get_widget (xml,"rb_mode_dual");
    
    ck_theme_color = glade_xml_get_widget (xml,"ck_use_theme_color");	
        
    sp_font_size  = glade_xml_get_widget (xml,"sp_font_size");

    gtk_spin_button_set_range (GTK_SPIN_BUTTON (sp_line), 	LINE_MIN_NUMBER,   LINE_MAX_NUMBER);
    gtk_spin_button_set_range (GTK_SPIN_BUTTON (sp_column), 	COLUMN_MIN_NUMBER, COLUMN_MAX_NUMBER);
    
    gtk_spin_button_set_range (GTK_SPIN_BUTTON (sp_font_size), 	MIN_FONT_SIZE, MAX_FONT_SIZE);
    
    g_signal_connect (w_braille_monitor, "response",
		      G_CALLBACK (bmui_response_braille_monitor), NULL);
    g_signal_connect (w_braille_monitor, "delete_event",
                      G_CALLBACK (bmui_delete_emit_response_cancel), NULL);        
}


/**
* bmui_load_braille_monitor_settings
* @parent_window: Parent window.
*
* The Braille Monitor setting UI load function.
*
* returns: TRUE if success.
**/
gboolean 
bmui_load_braille_monitor_settings (GtkWidget *parent_window)
{
    if (!w_braille_monitor)
    {
	GladeXML *xml;	
	xml = gn_load_interface ("Braille_Monitor_Settings/braille_monitor_settings.glade2", NULL);
	sru_return_val_if_fail (xml, FALSE);
	w_braille_monitor  = glade_xml_get_widget (xml, "w_braille_monitor");
	bmui_set_handlers_braille_monitor_settings (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_braille_monitor),
	    			      GTK_WINDOW (parent_window));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_braille_monitor), TRUE);
    }
    else
	gtk_widget_show (w_braille_monitor);
    	
    bmui_braille_monitor_values_add_to_widgets ();
    
    return TRUE;
}
