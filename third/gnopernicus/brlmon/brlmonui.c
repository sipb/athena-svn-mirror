/* brlmonui.c
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
#include "brlmonui.h"
#include "brlmon.h"
#include <gdk/gdk.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <gconf/gconf-value.h>
#include "SRMessages.h"
#include "srintl.h"
#include <glib.h>

#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>


#define STATUS_CELL_COUNT 	4

#define FIRST_POSITION 		0
#define INVALID_POSITION 	-1

#define DEFAULT_FONT_SIZE	18
#define FONT_COURIER   		"courier"
#define FONT_BRAILLE   		"Braille-HC"

#define DEFAULT_PANEL_WIDTH	500
#define DEFAULT_PANEL_HEIGHT	50

#define CELL_TEXT_LENGTH 	8

#define CELL_MARGINE 		8

#define MIN_FONT_SIZE		8

#define MIN_HEIGHT(FONT_SIZE)		2 * CELL_MARGINE + FONT_SIZE + 14

#define POSITION_TOP_STRING 	"TOP"
#define POSITION_BOTTOM_STRING 	"BOTTOM"
#define MODE_NORMAL_STRING	"NORMAL"
#define MODE_BRAILLE_STRING	"BRAILLE"
#define MODE_DUAL_STRING	"DUAL"


enum {
	STRUT_LEFT = 0,
	STRUT_RIGHT = 1,
	STRUT_TOP = 2,
	STRUT_BOTTOM = 3,
	STRUT_LEFT_START = 4,
	STRUT_LEFT_END = 5,
	STRUT_RIGHT_START = 6,
	STRUT_RIGHT_END = 7,
	STRUT_TOP_START = 8,
	STRUT_TOP_END = 9,
	STRUT_BOTTOM_START = 10,
	STRUT_BOTTOM_END = 11
};

typedef enum
{
    BRLMON_PANEL_TOP,
    BRLMON_PANEL_BOTTOM,
    BRLMON_PANEL_NONE
} BrlmonWindowPosition;

/* width and height that we have resized the window to */
static gint our_resize_width  = DEFAULT_PANEL_WIDTH;
static gint our_resize_height = DEFAULT_PANEL_HEIGHT;

static gint _screen_width;

static void	brlmon_changes_cb	(GConfClient	*client,
					guint		cnxn_id,
					GConfEntry	*entry,
					gpointer	user_data);

static void     brlmon_main_resize_window (GtkWidget* pWindow, 
					    gint width, 
					    gint height,
					    BrlmonWindowPosition window_position);

void brlmon_update_struts (gint height, GtkWidget *widget);

/**
 *
 * BrlMon main window widget.
 *
**/
static GtkWidget *w_brlmon;

/**
 *
 * Table of horizontal boxes
 *
**/
static GtkWidget *hbox [PANEL_LENGHT];
static GtkWidget *main_hbox;

/**
 *
 * Table of PANEL_LENGTH number frames
 *
**/
static GtkWidget *frame [PANEL_LENGHT];
static GtkWidget *frame_status [STATUS_CELL_COUNT];
static GtkWidget *label_status;
/**
 *
 * Tables of characters.
 *
**/
static GtkWidget *cell [PANEL_LENGHT];
static GtkWidget *cell_status [STATUS_CELL_COUNT];

/**
 *
 * panel_size - number of characters
 *
**/
static gint panel_size;
static gint panel_pos;
static gint brlmon_cell_frame_x_size;
static gint brlmon_cell_frame_y_size;

/**
 *
 * cursor_pos - cursor position on display
 *
**/
static gint cursor_pos;

/**
 *
 * old_pos - last cursor displayed position
 *
**/
static gint old_pos;

/**
 *
 * brlmon_typedot - display dot type
 *
**/
static gint brlmon_typedot;

/**
 *
 * Braille setting gconf client listener				
 *
**/
static GConfClient *brlmon_client = NULL;

       gint 	   brlmon_modetype = MODE_NORMAL;
static gboolean    brlmon_use_default_color = TRUE;
       GdkColor   *brlmon_colors;
static gint	   brlmon_font_size;

/**
 * brlmon_set_typedot
 *
 * @val: new dot type.
 *
 * Set dot type.
 *
 * return:
**/
void 
brlmon_set_typedot (DotType val)
{
    brlmon_typedot = val;
}

/**
 * brlmon_main_get_main_window
 *
 * Return a pointer of main window.
 *
 * return: pointer to main window.
**/
static GtkWidget* 
brlmon_main_get_main_window (void)
{
    return w_brlmon;
}


/**
 * brlmon_gconf_client_init
 *
 * Initialize gconf client.
 *
 * return: TRUE if succes.
**/
gboolean 
brlmon_gconf_client_init (void)
{
    GError *error = NULL;
        
    brlmon_client = gconf_client_get_default ();
             
    if (brlmon_client == NULL) 
    {
	sru_warning (_("Failed to init gconf client."));
	return FALSE;
    }
	             
    gconf_client_add_dir(brlmon_client,
	BRAILLE_MONITOR_PATH,GCONF_CLIENT_PRELOAD_NONE,&error);

    if (error != NULL)
    {
	sru_warning (_("Failed to add directory."));
	g_error_free (error);
	error = NULL;
	brlmon_client = NULL;
	return FALSE;
    }
    
    gconf_client_notify_add (brlmon_client, BRAILLE_MONITOR_PATH,
			    brlmon_changes_cb,
			    NULL,NULL,&error);
    if (error != NULL)
    {
        sru_warning (_("Failed to add notify."));
        g_error_free (error);
        error = NULL;
        return FALSE;
    }
		
    return TRUE;
}

static gboolean
brlmon_check_type (const gchar* key, GConfValue* val, GConfValueType t, GError** err)
{
    if (val->type != t)
    {
        g_set_error (err, GCONF_ERROR, GCONF_ERROR_TYPE_MISMATCH,
	  	   _("Expected key: %s"),
                   key);
	      
        return FALSE;
    }
    else
	return TRUE;
}

/**
 * brlmon_set_int
 *
 * @val: gint value what you need to set.
 * @key: key name of value
 *
 * Set value in gconf with the key keyname.
 *
 * return: TRUE if succes.
**/
gboolean
brlmon_set_int (gint val, const gchar *key)
{
    GError *error = NULL;
    gboolean  ret = TRUE;
    gchar *path;
    
    sru_return_val_if_fail (key != NULL, FALSE);
    sru_return_val_if_fail (brlmon_client != NULL, FALSE);
    
    path = gconf_concat_dir_and_key (BRAILLE_MONITOR_PATH, key);
    
    sru_return_val_if_fail (gconf_client_key_is_writable (brlmon_client, 
							  path, 
							  NULL), 
							  FALSE);
    
    ret  = gconf_client_set_int (brlmon_client, path, val, &error);

    if (error != NULL)
    {
	sru_warning (_("Failed to set value: %d"), val);
	g_error_free (error);
	error = NULL;
    }
    
    g_free(path);
    return ret;
}

/**
 * brlmon_get_int_with_default
 *
 * @key: key name of value
 * @def: default value for key.
 *
 * Get value from gconf. Set the default value if the key not exist.
 *
 * return: value from gconf or the default value.
**/
gint 
brlmon_get_int_with_default (const gchar *key, gint def)
{
    GError *error = NULL;
    GConfValue *value = NULL;
    gint ret_val;
    gchar *path = NULL;
    
    sru_return_val_if_fail (key != NULL, def);
    sru_return_val_if_fail (brlmon_client != NULL, def);

    path  = gconf_concat_dir_and_key (BRAILLE_MONITOR_PATH, key);
    value = gconf_client_get (brlmon_client, path, &error);
    
    ret_val = def;
    
    if (value != NULL && error == NULL)
    {
	if (brlmon_check_type (key, value, GCONF_VALUE_INT, &error))
    	    ret_val = gconf_value_get_int (value);
        else
	{
    	    sru_warning (_("Invalid type of key: %s."), key);
		
	    if (!brlmon_set_int (ret_val, key))
	        sru_warning (_("Failed to set int value: %d."), ret_val);
	}
	    
	
	gconf_value_free (value);
	g_free (path);
	
        return ret_val;
    }
    else
    {
	if (error != NULL)
	{
	    sru_warning (_("Failed to get value: %s."), key);
	    g_error_free (error);
	    error = NULL;
	}
	    
	if (!brlmon_set_int (def,key))
	    sru_warning (_("Failed to set int value: %d."), def);
		
	g_free (path);    
	
        return def;
    }    
}

/**
 * brlmon_set_bool
 *
 * @val: gboolean value what you need to set.
 * @key: key name of value
 *
 * Set value in gconf with the key keyname.
 *
 * return: TRUE if succes.
**/
gboolean
brlmon_set_bool (gboolean val, const gchar *key)
{
    GError *error = NULL;
    gboolean  ret = TRUE;
    gchar *path;
    
    sru_return_val_if_fail (key != NULL, FALSE);
    sru_return_val_if_fail (brlmon_client != NULL, FALSE);
    
    path = gconf_concat_dir_and_key (BRAILLE_MONITOR_PATH, key);
    
    sru_return_val_if_fail (gconf_client_key_is_writable (brlmon_client, 
							  path, 
							  NULL), 
							  FALSE);
    
    ret  = gconf_client_set_bool (brlmon_client, path, val, &error);

    if (error != NULL)
    {
	sru_warning (_("Failed to set value: %d."), val);
	g_error_free (error);
	error = NULL;
    }
    
    g_free(path);
    return ret;
}

/**
 * brlmon_get_bool_with_default
 *
 * @key: key name of value
 * @def: default value for key.
 *
 * Get value from gconf. Set the default value if the key not exist.
 *
 * return: value from gconf or the default value.
**/
gboolean
brlmon_get_bool_with_default (const gchar *key, gboolean def)
{
    GError *error = NULL;
    GConfValue *value = NULL;
    gboolean ret_val;
    gchar *path = NULL;
    
    sru_return_val_if_fail (key != NULL, def);
    sru_return_val_if_fail (brlmon_client != NULL, def);

    path  = gconf_concat_dir_and_key (BRAILLE_MONITOR_PATH, key);
    value = gconf_client_get (brlmon_client, path, &error);
    
    ret_val = def;
    
    if (value != NULL && error == NULL)
    {
	if (brlmon_check_type (key, value, GCONF_VALUE_BOOL, &error))
    	    ret_val = gconf_value_get_bool (value);
        else
	{
    	    sru_warning (_("Invalid type of key: %s."), key);
		
	    if (!brlmon_set_bool (ret_val, key))
	        sru_warning (_("Failed to set boolean value: %d."), ret_val);
	}
	    
	
	gconf_value_free (value);
	g_free (path);
	
        return ret_val;
    }
    else
    {
	if (error != NULL)
	    sru_warning (_("Failed to get value: %s."), key);
	    
	if (!brlmon_set_bool (def, key))
	    sru_warning (_("Failed to set boolean value: %d."),def);
		
	g_free (path);    
	
        return def;
    }    
}

/**
 * brlmon_set_string
 *
 * @val: text what you need to set.
 * @key: key name of value
 *
 * Set text in gconf with the key keyname.
 *
 * return: TRUE if succes.
**/
gboolean
brlmon_set_string (const gchar *val, const gchar *key)
{
    GError *error = NULL;
    gboolean ret  = TRUE;
    gchar *path   = NULL;
    
    g_return_val_if_fail (key != NULL, FALSE);
    g_return_val_if_fail (brlmon_client != NULL, FALSE);

    path = gconf_concat_dir_and_key (BRAILLE_MONITOR_PATH, key);
    
    g_return_val_if_fail (gconf_client_key_is_writable (brlmon_client,
							path,
							NULL), 
							FALSE);
    
    ret = gconf_client_set_string (brlmon_client, path, val, &error);

    if (error != NULL)
    {
	    sru_warning (_("Failed to set value: %s."), val);
	    g_error_free (error);
	    error = NULL;
    }
    
    g_free (path);
    
    return ret;
}

/**
 * brlmon_get_string_with_default
 *
 * @key: key name of value
 * @def: default text for key.
 *
 * Get value from gconf. Set the default text if the key not exist.
 *
 * return: text from gconf or the default text.
**/
gchar* 
brlmon_get_string_with_default (const gchar *key, gchar *def)
{
    GError *error = NULL;
    gchar *path   = NULL;
    gchar *retval = NULL;
    
    g_return_val_if_fail (key != NULL, def );
    g_return_val_if_fail (brlmon_client != NULL, def );
    path   = gconf_concat_dir_and_key (BRAILLE_MONITOR_PATH,key);
    retval = gconf_client_get_string (brlmon_client, path, &error);

    if (error)
    {
	sru_warning (_("Failed to get string value: %s."), key);
	g_error_free(error);
	error = NULL;
	if (!brlmon_set_string (def,key))
	    sru_warning (_("Failed to set string value: %s."), def);
	
	g_free (path);
	
	return g_strdup (def);
    }
    
    if (!retval)
    {
	if (!brlmon_set_string (def, key))
	    sru_warning (_("Failed to set string value: %s."), def);
	return g_strdup (def);
    }
    g_free (path);

    return g_strdup (retval);
}

/**
 * brlmon_get_mode_type_from_string
 *
 * @mode:
 *
 * Return modetype from string type.
 *
 * return: modetype.
**/

/*FIXME when brlmon will be rewritten.*/
ModeType
brlmon_get_mode_type_from_string (const gchar *mode)
{
    gchar 	*tmp;
    ModeType 	modetype;
    
    tmp = g_ascii_strup (mode, -1);
    
    if (!strcmp (tmp, MODE_NORMAL_STRING))  modetype = MODE_NORMAL;
	else
    if (!strcmp (tmp, MODE_BRAILLE_STRING)) modetype = MODE_NORMAL;
	else
    if (!strcmp (tmp, MODE_DUAL_STRING))    modetype = MODE_NORMAL;
	else
    {
	sru_warning (_("Invalid modetype: %s."), mode);		    
	modetype = MODE_NORMAL;
    }
    
    g_free (tmp);
    
    return modetype;
}
/*FIXME END*/

/**
 * brlmon_get_font_size
 *
 * return: Size of font.
**/
static gint
brlmon_get_font_size (void)
{
    brlmon_font_size = brlmon_get_int_with_default (BRAILLE_MONITOR_FONT_SIZE_GCONF_KEY, 
					    DEFAULT_FONT_SIZE);
    return brlmon_font_size < MIN_FONT_SIZE ? MIN_FONT_SIZE : brlmon_font_size;
}

/**
 * brlmon_change_font
 *
 * @view: view at which change the font.
 * @font_name: font name.
 *
 * Change at the specified font for the specified view.
 *
 * return:
**/
static void 
brlmon_change_font (GtkWidget 	*view,
		    const gchar *font_name)
{
    PangoFontDescription *font_desc = NULL;
    gchar *font = NULL;
        
    font = g_strdup_printf ("%s %d", font_name, brlmon_font_size);
    
    font_desc = pango_font_description_from_string (font);
    
    g_free (font);
    
    if (font_desc)
    {
	gtk_widget_modify_font (view, font_desc);
	pango_font_description_free (font_desc);
    }
    else
	sru_warning (_("%s font not installed."), font_name);
}

#define NO_OF_DOT_TYPES  3
#define COLOR_PARSE_ERROR_TEXT N_("Can not parse color string.")
/**
 * brlmon_load_colors 
 *  
 * Load colors.
 *
 * return: TRUE if succes.
**/
gboolean
brlmon_load_colors (void)
{
    gboolean *succes;    
    gchar    *string_color; 
    
    brlmon_colors = (GdkColor*) g_new0 (GdkColor , NO_OF_DOT_TYPES);
    succes = (gboolean*) g_new0 (gboolean , NO_OF_DOT_TYPES);
    
    brlmon_use_default_color = 
	brlmon_get_bool_with_default (BRAILLE_MONITOR_USE_THEME_GCONF_KEY, 
				      brlmon_use_default_color);

    string_color = 
	brlmon_get_string_with_default (BRAILLE_MONITOR_DOT78_GCONF_KEY, 
					DEFAULT_DOT78_COLOR);
					
    if (!gdk_color_parse (string_color, &brlmon_colors[0]))
    	sru_warning (_(COLOR_PARSE_ERROR_TEXT));
    g_free (string_color);
    
    string_color = 
	brlmon_get_string_with_default (BRAILLE_MONITOR_DOT7_GCONF_KEY, 
					DEFAULT_DOT7_COLOR);
    if (!gdk_color_parse (string_color, &brlmon_colors[1]))
    	sru_warning (_(COLOR_PARSE_ERROR_TEXT));
    g_free (string_color);
    
    string_color = 
	brlmon_get_string_with_default (BRAILLE_MONITOR_DOT8_GCONF_KEY, 
					DEFAULT_DOT8_COLOR);
    if (!gdk_color_parse (string_color, &brlmon_colors[2]))
    	sru_warning (_(COLOR_PARSE_ERROR_TEXT));
    g_free (string_color);

    if (gdk_colormap_alloc_colors (gdk_colormap_get_system (), 
			          brlmon_colors, NO_OF_DOT_TYPES, 
			          FALSE, TRUE, succes))
	return FALSE;

    return TRUE;	
}
/**
 * brlmon_set_color
 *
 * Set color for text, depended from dot type.
 *
 * return:
**/
static void 
brlmon_set_color (GtkWidget *widget)
{
    if (brlmon_use_default_color)
      {
	GtkStateType state;
	switch (brlmon_typedot)
	  {
	  case DOT78:
	    state = GTK_STATE_SELECTED;
	  case DOT7:
	    state = GTK_STATE_PRELIGHT;
	  case DOT8:
	    state = GTK_STATE_ACTIVE;
	  default:
	    state = GTK_STATE_NORMAL;
	    break;
	  }
	gtk_widget_modify_text (widget, state, NULL);
	gtk_widget_modify_text (label_status, state, NULL);
	return;
    }

    switch (brlmon_typedot)
    {
	case DOTNONE:
	    gtk_widget_modify_text (widget, GTK_STATE_NORMAL, NULL);
	    break;	
	
	case DOT78:
	    gtk_widget_modify_text (widget, GTK_STATE_NORMAL, &brlmon_colors[0]);
	    break;
	
	case DOT7:
	    gtk_widget_modify_text (widget, GTK_STATE_NORMAL, &brlmon_colors[1]);
	    break;
	    
	default:	
	    gtk_widget_modify_text (widget, GTK_STATE_NORMAL, &brlmon_colors[2]);
	    break;
    }  
}

/**
 * brlmon_update_text_color
 *
 * Update text color for all cell.
 *
 * return:
**/
static void
brlmon_update_text_color (void)
{
    gint i;
    
    for(i = 0 ; i < panel_size ; i++)
	brlmon_set_color (cell[i]);

    for (i = 0 ; i < STATUS_CELL_COUNT; i++)
	brlmon_set_color (cell_status[i]);
}

/**
 * brlmon_cursor_pos_clean
 *
 * Move cursor in 1st cell
 *
 * return:
**/
void 
brlmon_cursor_pos_clean (void)
{
    cursor_pos = FIRST_POSITION;
}

/**
 * brlmon_old_pos_clean
 *
 * Clean old position.
 *
 * return:
**/
void 
brlmon_old_pos_clean (void)
{
    old_pos = INVALID_POSITION;
}

/**
 * brlmon_cursor_pos
 *
 * @pos: position on display
 *
 * Set cursor position on display
 *
 * return:
**/
void 
brlmon_cursor_pos (gint pos)
{ 
    if (pos < FIRST_POSITION || pos >= panel_size)
	return;
    
    if (old_pos > INVALID_POSITION)
	gtk_frame_set_shadow_type (GTK_FRAME (frame[old_pos]), GTK_SHADOW_OUT);
	
    gtk_frame_set_shadow_type (GTK_FRAME ( frame[pos] ), GTK_SHADOW_IN);
	
    if (brlmon_modetype == MODE_DUAL)
    {
	if (old_pos > INVALID_POSITION)
	    gtk_frame_set_shadow_type (GTK_FRAME (frame[panel_size + old_pos]), 
				       GTK_SHADOW_OUT);
	
	gtk_frame_set_shadow_type (GTK_FRAME (frame[panel_size + pos]), 
				   GTK_SHADOW_IN);
    }
    
    old_pos = pos;
}

/**
 * brlmon_clean_panel_with_size
 *
 * @size: number of cells in display.
 *
 * Clean a braille monitor display with size number of cells.
 *
 * return:
**/
static void 
brlmon_clean_panel_with_size (gint size)
{
    GtkTextBuffer *buffer;
    gint i;
    
    for(i = 0 ; i < size ; i++)
    {
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (cell[i]));
	gtk_text_buffer_set_text (buffer, " ", 1);
	brlmon_set_typedot (DOTNONE);
	brlmon_set_color  (cell[i]);
	brlmon_change_font (cell[i], FONT_COURIER);
	gtk_frame_set_shadow_type (GTK_FRAME (frame[i]), GTK_SHADOW_OUT);
    }
    
    for(i = 0; i < STATUS_CELL_COUNT ; i++)
    {
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (cell_status[i]));
	gtk_text_buffer_set_text (buffer, " ", 1);
	brlmon_change_font (cell_status[i], FONT_COURIER);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_status[i]), GTK_SHADOW_OUT);
    }
    
}

/**
 * brlmon_clean_panel
 *
 * Clean display.
 *
 * return:
**/
void 
brlmon_clean_panel (void)
{
    switch(brlmon_modetype)
    {
	case MODE_NORMAL:
	case MODE_BRAILLE:
	     brlmon_clean_panel_with_size (panel_size);
	    break;
	case MODE_DUAL:
	     brlmon_clean_panel_with_size (2 * panel_size);
	    break;
    }
}


/**
 * brlmon_print_text_on_status
 *
 * @text: status text to show
 *
 * Show status on status display
 *
 * return:
**/
static void 
brlmon_print_text_on_status (gchar *text)
{
    GtkTextBuffer *buffer;
    gchar str[CELL_TEXT_LENGTH];
    gint  i;
    
    sru_return_if_fail (text);
    sru_return_if_fail (g_utf8_strlen (text,-1) == STATUS_CELL_COUNT);
                
    for (i = 0 ; i < STATUS_CELL_COUNT; i++)
    {
	g_utf8_strncpy (str, text, 1);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (cell_status[i]));
	gtk_text_buffer_set_text (buffer, str, -1);
	brlmon_set_color (cell_status[i]);
	text = g_utf8_find_next_char (text, NULL);
	    
	switch (brlmon_modetype)
	{
	    case MODE_DUAL:
	    case MODE_NORMAL:
		brlmon_change_font (cell_status[i], FONT_COURIER);
		break;
	    case MODE_BRAILLE:
		brlmon_change_font (cell_status[i], FONT_BRAILLE);
		break;
        }
    }
}


/**
 * brlmon_print_text_from_cur_pos_on_display
 *
 * @text: text to show from current position
 *
 * Show text on display from current position
 *
 * return:
**/
static void 
brlmon_print_text_from_cur_pos_on_display (gchar *text)
{
    gint len;
    gchar str[32];
    gint i;
    gint pos;
    GtkTextBuffer *buffer;
    
    sru_return_if_fail (text);
    sru_return_if_fail (g_utf8_validate (text, -1, NULL));
    len = g_utf8_strlen (text,-1);
    len = (cursor_pos + len < panel_size) ? len : (panel_size - cursor_pos);
    
    pos = cursor_pos;
    
    for (i = 0 ; i < len; i ++)
    {
	g_utf8_strncpy (str, text, 1);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (cell[i + pos]));
	gtk_text_buffer_set_text (buffer, str, -1);
	brlmon_set_color (cell[i+pos]);
	    
	if (brlmon_modetype == MODE_NORMAL)
	    brlmon_change_font (cell[i+pos], FONT_COURIER);
	else
	    brlmon_change_font (cell[i+pos], FONT_BRAILLE);
	    
	cursor_pos++;
	text = g_utf8_find_next_char (text, NULL);
    }
	
    brlmon_typedot = DOTNONE;
}


/**
 * brlmon_print_text_from_cur_pos_dual_mode
 *
 * @text: text to show
 *
 * Show text on display in dual mode
 *
 * return:
**/
static void 
brlmon_print_text_from_cur_pos_dual_mode (gchar *text)
{
    gint len;
    gchar str[CELL_TEXT_LENGTH];
    gint i;
    gint pos;
    GtkTextBuffer *buffer;

    sru_return_if_fail (text);    
    sru_return_if_fail (g_utf8_validate (text, -1, NULL));
    len = g_utf8_strlen (text,-1);
    len = (cursor_pos + len < panel_size) ? len : (panel_size - cursor_pos);
    
    pos = cursor_pos;
    
    for (i = 0 ; i < len; i ++)
    {
	g_utf8_strncpy (str, text, 1);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (cell[i + pos]));
	gtk_text_buffer_set_text (buffer, str, -1);
	brlmon_set_color (cell[i+pos]);
	brlmon_change_font (cell[i+pos], FONT_COURIER);
	cursor_pos++;
	    
	buffer = 
	    gtk_text_view_get_buffer (GTK_TEXT_VIEW (cell[i + pos + panel_size]));
	gtk_text_buffer_set_text (buffer, str, -1);
	brlmon_set_color (cell[i + pos + panel_size]);
	brlmon_change_font (cell[i + pos + panel_size], FONT_BRAILLE);
	text = g_utf8_find_next_char (text, NULL);
    }
	
    brlmon_typedot = DOTNONE;
}

/**
 * brlmon_print_text_from_cur_pos
 *
 * @text: text to show
 * @role: type of text
 * Show text on braille monitor display. It is generic show function
 *
 * return:
**/
void 
brlmon_print_text_from_cur_pos (gchar *text, DisplayRole role)
{
    switch(role)
    {
	case ROLE_MAIN:
	case ROLE_OTHER:
		if (brlmon_modetype != MODE_DUAL)	
		    brlmon_print_text_from_cur_pos_on_display (text);
		else			
		    brlmon_print_text_from_cur_pos_dual_mode (text);
	    break;
	case ROLE_STATUS:
		brlmon_print_text_on_status (text);
	    break;
    }
}


/**
 * brlmon_destroy_cell_table
 *
 * @modify_size: if the size of display need to changed
 * @change_mode: if the mode type need to changed
 *
 * Destroy the display.
 *
 * return:
**/
static void
brlmon_destroy_cell_table (gboolean modify_size,
			   gboolean change_mode)
{
    GtkWidget *temp_window = NULL;
    ModeType old_type = brlmon_modetype;
    gint line;
    gint column;
    
    line   = brlmon_get_int_with_default (BRAILLE_MONITOR_LINE_GCONF_KEY,   DEFAULT_LINE);    
    column = brlmon_get_int_with_default (BRAILLE_MONITOR_COLUMN_GCONF_KEY, DEFAULT_COLUMN);    
    
    if (change_mode)
    {
	gchar *mode;
	mode = brlmon_get_string_with_default (BRAILLE_MONITOR_MODE_GCONF_KEY, 
					       DEFAULT_MODE);    
	brlmon_modetype = brlmon_get_mode_type_from_string (mode);
	g_free (mode);
    }
    
    if (modify_size ||
       (change_mode && 
       (
    	((old_type == MODE_BRAILLE || old_type == MODE_NORMAL)  && brlmon_modetype == MODE_DUAL) ||
	(old_type == MODE_DUAL))))
    {
	gtk_widget_hide_all (main_hbox);
	temp_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_reparent (main_hbox, temp_window);			    
	gtk_widget_destroy (temp_window);
	gtk_widget_destroyed (temp_window, &temp_window );
	main_hbox = NULL;
	brlmon_create_text_area (line, column);
    }
    
    brlmon_clean_panel ();
    
    brlmon_cursor_pos_clean ();	    
    
    brlmon_refresh_display ();
}


/**
 * brlmon_create_status
 *
 * Create "text status" cells
 *
 * return:
**/
static void 
brlmon_create_status (void)
{
    GtkWidget *vbox_status;    
    GtkWidget *hbox_status;
    gint i;

    vbox_status = gtk_vbox_new (FALSE, 0);
    gtk_box_set_homogeneous (GTK_BOX (vbox_status),TRUE);
    gtk_widget_ref (vbox_status);
    gtk_object_set_data_full (GTK_OBJECT (brlmon_main_get_main_window ()), 
				"vbox", vbox_status,
	                        (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vbox_status);
    gtk_box_pack_start (GTK_BOX (main_hbox), vbox_status, TRUE, TRUE, 5);
    
    label_status = gtk_label_new (_("Status"));
    gtk_widget_show (label_status);
    /*Set label size */
    gtk_widget_set_usize (label_status, 10, 10); 
    gtk_box_pack_start (GTK_BOX (vbox_status), label_status, TRUE, TRUE, 0);
    gtk_label_set_justify (GTK_LABEL (label_status), GTK_JUSTIFY_LEFT);
        
    /* create hbox for status cells */
    hbox_status = gtk_hbox_new (TRUE, 0);
    gtk_widget_ref  (hbox_status);
    gtk_widget_show (hbox_status);
    gtk_box_pack_start (GTK_BOX (vbox_status), hbox_status, TRUE, TRUE, 0);
    
    for(i = 0 ;i < STATUS_CELL_COUNT; i++)
    {
	/* create frame container for cells */
	frame_status[i] = gtk_frame_new (NULL);
	gtk_widget_ref (frame_status[i]);
	/* set size of frame */
	gtk_widget_set_usize (frame_status[i], brlmon_cell_frame_x_size, brlmon_cell_frame_y_size);
	gtk_object_set_data_full (GTK_OBJECT (main_hbox), "frame1", frame_status[i],
                        		(GtkDestroyNotify) gtk_widget_unref);
	gtk_frame_set_shadow_type (GTK_FRAME(frame_status[i]),GTK_SHADOW_OUT);
	gtk_widget_show (frame_status[i]);
	gtk_box_pack_start (GTK_BOX (hbox_status), frame_status[i], TRUE, TRUE, 0);

	/* create status cells */
	cell_status[i] = gtk_text_view_new();
	gtk_widget_ref (cell_status[i]);
	gtk_text_view_set_editable (GTK_TEXT_VIEW(cell_status[i]),FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(cell_status[i]),FALSE);
/*	
	gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (cell_status[i]), 1);
	gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (cell_status[i]), 1);
*/
	gtk_object_set_data_full (GTK_OBJECT (main_hbox), "c_status", cell_status[i],
                        		(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (cell_status[i]);
	gtk_container_add (GTK_CONTAINER (frame_status[i]), cell_status[i]);
    }
}




/**
 * brlmon_create_lines
 *
 * @line: number of lines.
 *
 * Create line number of horizontal boxes.
 *
 * return:
**/
static void 
brlmon_create_lines (gint line)
{
    GtkWidget *vbox;    
    gint i;
        
    main_hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (main_hbox);
    gtk_container_add (GTK_CONTAINER (brlmon_main_get_main_window ()), 
			main_hbox);
    
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_set_homogeneous (GTK_BOX (vbox),TRUE);
    gtk_widget_ref (vbox);
    gtk_object_set_data_full (GTK_OBJECT (main_hbox), "vbox", vbox,
	                        (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (vbox);
    gtk_box_pack_start (GTK_BOX (main_hbox), vbox, TRUE, FALSE, 0);
    
    for(i = line - 1 ; i > -1 ; i--)
    {
	/* create horizontal box for all cell lines */
	hbox[i] = gtk_hbox_new (TRUE, 0);
	gtk_box_set_homogeneous (GTK_BOX (hbox[i]),TRUE);
	gtk_widget_ref (hbox[i]);
	gtk_object_set_data_full (GTK_OBJECT (main_hbox), "main_hbox", hbox[i],
                        	(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbox[i]);
	gtk_box_pack_start (GTK_BOX (vbox), hbox[i], TRUE, FALSE, 0);
    }    
}

/**
 * brlmon_create_item
 *
 * @line: number of lines.
 * @column: number of cells in line.
 *
 * Create cell table with "line" - row and "column" -column
 *
 * return:
**/
static void 
brlmon_create_item (gint line, gint column)
{
    gint i;
    
    for(i = 0 ; i < column ; i++,panel_pos++)
    {
	/* create cell frame */
	frame[panel_pos] = gtk_frame_new (NULL);
	gtk_widget_ref (frame[panel_pos]);
	/* set destroy signal*/
	gtk_object_set_data_full (GTK_OBJECT (main_hbox), "frame", frame[panel_pos],
                        		(GtkDestroyNotify) gtk_widget_unref);
	/* set type of SHADOW */
	gtk_frame_set_shadow_type (GTK_FRAME(frame[panel_pos]),GTK_SHADOW_OUT);
	/* set frame size */
	gtk_widget_set_usize (frame[panel_pos], brlmon_cell_frame_x_size, brlmon_cell_frame_y_size);
	gtk_widget_show (frame[panel_pos]);
	gtk_box_pack_start (GTK_BOX (hbox[line]), frame[panel_pos], TRUE, FALSE, 0); 

	/* create cell */
	cell[panel_pos] = gtk_text_view_new();
	gtk_widget_ref (cell[panel_pos]);

	/* set editable and visible property of text cell */
	gtk_text_view_set_editable (GTK_TEXT_VIEW(cell[panel_pos]),FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(cell[panel_pos]),FALSE);

	/* set margine of text cell */
	gtk_text_view_set_left_margin  (GTK_TEXT_VIEW (cell[panel_pos]), 2);
	gtk_text_view_set_right_margin (GTK_TEXT_VIEW (cell[panel_pos]), 2);
	
	gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (cell[panel_pos]), 2);
	/* set destroy signal*/
	gtk_object_set_data_full (GTK_OBJECT (main_hbox), "c", cell[panel_pos],
                        		(GtkDestroyNotify) gtk_widget_unref);
	/* show cell and and at to container */
	gtk_widget_show (cell[panel_pos]);
	gtk_container_add (GTK_CONTAINER (frame[panel_pos]), cell[panel_pos]);
    }
}

/**
 * brlmon_get_screen_size
 *
 * @x_screen: widht of screen.
 * @y_screen: height of screen
 *
 * Return size of screen.
 *
 * return:
**/
void
brlmon_get_screen_size (gint *x_screen, 
			gint *y_screen)
{
    *x_screen = gdk_screen_width();
    *y_screen = gdk_screen_height();
}

/**
 * brlmon_return_panel_position
 *
 * Return the position for panel from gconf.
 *
 * return: position.
**/
BrlmonWindowPosition
brlmon_return_panel_position (void)
{
    gchar *position;
    BrlmonWindowPosition pos;
    position = 
	brlmon_get_string_with_default (BRAILLE_MONITOR_PANEL_POSITION_GCONF_KEY, 
					DEFAULT_BRAILLE_MONITOR_PANEL_POSITION);
    
    if (!strcmp (position, POSITION_BOTTOM_STRING))
	pos = BRLMON_PANEL_BOTTOM;
    else
	pos = BRLMON_PANEL_TOP;
	
    g_free (position);
    
    return pos;
}

/**
 * brlmon_create_text_area
 *
 * @line:
 * @column:
 *
 * Create display table.
 *
 * return:
**/
void 
brlmon_create_text_area (gint line, gint column)
{
    gint i;
    gint height, width;
    gint x_screen, y_screen;
    
    brlmon_font_size = brlmon_get_font_size ();
    
    if (line * column > PANEL_LENGHT / 2)
    {
        column	= DEFAULT_COLUMN;
        line	= DEFAULT_LINE;
	brlmon_set_int (line,  BRAILLE_MONITOR_LINE_GCONF_KEY);    
	brlmon_set_int (column,   BRAILLE_MONITOR_COLUMN_GCONF_KEY);    
	sru_message (_("Too many cells on display. [Max %d]"), PANEL_LENGHT / 2);
        sru_message (_("Load default display size. Line: %d Column: %d."), line, column);
    }
            
    panel_size = column * line;
    
    if (brlmon_modetype == MODE_DUAL) 
	line = line * 2;
    
    brlmon_get_screen_size (&x_screen, &y_screen);    

    brlmon_cell_frame_x_size = (gint)(x_screen / (column + STATUS_CELL_COUNT));
    brlmon_cell_frame_y_size = brlmon_font_size + 2 * CELL_MARGINE;
    
    width  = x_screen;
    height = (line * (brlmon_font_size + 2 * CELL_MARGINE) < MIN_HEIGHT(brlmon_font_size)) ? 
	      MIN_HEIGHT(brlmon_font_size) : 
	      line * (brlmon_font_size + 2 * CELL_MARGINE);

    if (height > y_screen)
	height = y_screen;    
    
    brlmon_main_resize_window ( brlmon_main_get_main_window(), width, height, 
				brlmon_return_panel_position ());
    
    /* create lines */
    brlmon_create_lines (line);
        
    panel_pos = FIRST_POSITION;
    
    /* fill lines */    
    for(i = line - 1 ; i > INVALID_POSITION ; i--) 
	brlmon_create_item (i, column);
    
    /* create status cells */
    brlmon_create_status ();    
}

/**
 * brlmon_main_resize_window
 *
 * @pwindow:
 * @width:
 * @height:
 * @window_position:
 *
 * Resize and move the window.
 *
 * return:
**/
void 
brlmon_main_resize_window  (GtkWidget* pwindow, 
			    gint width, 
			    gint height,
			    BrlmonWindowPosition window_position)
{
	GdkGeometry hints;
	gint left;
	gint top;
	gint screenx;
	gint screeny;

	/* ensure the window is at least this big */
	if (width < DEFAULT_PANEL_WIDTH)
	    width = DEFAULT_PANEL_WIDTH;

	if (height < MIN_HEIGHT(brlmon_font_size))
	    height = MIN_HEIGHT(brlmon_font_size);

	/* calculate the upper left corner of the window */
	switch (window_position) 
	{ /* if we're a dock */
	    case BRLMON_PANEL_BOTTOM:
		top = gdk_screen_height ();
		left = 0;
	    	width = gdk_screen_width ();
	     break;
	    case BRLMON_PANEL_TOP:
		top = 0;
		left = 0;
	    	width = gdk_screen_width ();
	    break;
	    default:
		top = 0;
		left = 0;
		width = gdk_screen_width ();
	    break;
	}

	if (left < 0)
	    left = 0;

	if (top < 0)
	    top = 0;

	/* make sure the window does not go off screen */
	brlmon_get_screen_size (&screenx, &screeny);

	if ((width + left) > screenx)
	    left = screenx - width;

	if ((height + top) > screeny)
	    top = screeny - height;
	
	hints.min_width  = width;
	hints.min_height = height;
	hints.max_width  = width;
	hints.max_height = height;
	

	/* set geometry hints. Constrains resizing */
	gdk_window_set_geometry_hints ((GdkWindow*)pwindow->window, &hints,
					GDK_HINT_MIN_SIZE | 
					GDK_HINT_MAX_SIZE );

	/* move/resize the main window */
	gdk_window_move_resize ((GdkWindow*)pwindow->window, left, top, width, height);
	
	/* store these values */
	our_resize_width = width;
	our_resize_height = height;

	brlmon_update_struts (our_resize_height, pwindow);
}

/**
 * brlmon_main_set_wm_dock
 * 
 * @is_dock:
 *
 * Made window DOCK or NORMAL type.
 *
 * return:
**/
void
brlmon_main_set_wm_dock (gboolean is_dock)
{
    Atom atom_type[1], atom_window_type;
    GtkWidget *widget = brlmon_main_get_main_window ();
    
    gdk_error_trap_push ();
  
    atom_window_type = gdk_x11_get_xatom_by_name ("_NET_WM_WINDOW_TYPE");
  
    if (is_dock) 
	  atom_type[0] = gdk_x11_get_xatom_by_name ("_NET_WM_WINDOW_TYPE_DOCK");
    else
	  atom_type[0] = gdk_x11_get_xatom_by_name ("_NET_WM_WINDOW_TYPE_NORMAL");


    XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window), 
		   GDK_WINDOW_XWINDOW (widget->window), 
		   atom_window_type,
		   XA_ATOM, 32, PropModeReplace,
		   (guchar *) &atom_type, 1);
		   
    gdk_error_trap_pop ();
}

/**
 * brlmon_discard_focus_filter
 *
 * Event callback for discard focus event.
 *
 * return:
**/
GdkFilterReturn
brlmon_discard_focus_filter (GdkXEvent *xevent,
			     GdkEvent  *event,
			     gpointer  data)
{
  XEvent *xev = (XEvent *)xevent;	

  if (xev->xany.type == ClientMessage &&
      (Atom) xev->xclient.data.l[0] == gdk_x11_atom_to_xatom (
	      gdk_atom_intern ("WM_TAKE_FOCUS", False)))
    {
      return GDK_FILTER_REMOVE;
    }
  else
    {
      return GDK_FILTER_CONTINUE;
    }
}

/**
 * brlmon_on_window1_destroy
 *
 * Window destroy event callback function..
 *
 * return:
**/
void
brlmon_on_window_destroy   (GtkObject       *object,
                            gpointer         user_data)
{
}

/*
 * Update brlmon's strut allocation
 */
void
brlmon_update_struts (gint height, GtkWidget *widget)
{
	Atom atom_strut = gdk_x11_get_xatom_by_name ("_NET_WM_STRUT");
	Atom atom_strut_partial = gdk_x11_get_xatom_by_name ("_NET_WM_STRUT_PARTIAL");
	guint32 struts[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	BrlmonWindowPosition position = brlmon_return_panel_position ();

	if (position == BRLMON_PANEL_TOP) /*( brlmon_is_top)*/
	{
		struts[STRUT_TOP] = height;
		struts[STRUT_TOP_START] = 0;
		struts[STRUT_TOP_END] = _screen_width - 1;
	}
	else if (position == BRLMON_PANEL_BOTTOM)
	{
		struts[STRUT_BOTTOM] = height;
		struts[STRUT_BOTTOM_START] = 0;
		struts[STRUT_BOTTOM_END] = _screen_width - 1;
	}

	gdk_error_trap_push ();
	/* this means that we are responsible for placing ourselves appropriately on screen */
	if (widget && widget->window)
	{
		XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window), 
				 GDK_WINDOW_XWINDOW (widget->window), 
				 atom_strut,
				 XA_CARDINAL, 32, PropModeReplace,
				 (guchar *) &struts, 4);
		XChangeProperty (GDK_WINDOW_XDISPLAY (widget->window), 
				 GDK_WINDOW_XWINDOW (widget->window), 
				 atom_strut_partial,
				 XA_CARDINAL, 32, PropModeReplace,
				 (guchar *) &struts, 12);
	}
	gdk_error_trap_pop ();
}

/**
 * brlmon_on_window_realize
 *
 * @widget: window pointer.
 * @user_data: type of window (normal or dock).
 *
 * Window realize event callback function.
 *
 * return:
**/
void
brlmon_on_window_realize (GtkWidget       *widget,
                           gpointer         user_data)
{
    XWMHints wm_hints;
    Atom wm_window_protocols[3];
    gboolean is_dock = (gboolean) user_data;
  
    wm_window_protocols[0] = gdk_x11_get_xatom_by_name ("WM_DELETE_WINDOW");
    wm_window_protocols[1] = gdk_x11_get_xatom_by_name ("_NET_WM_PING");
    wm_window_protocols[2] = gdk_x11_get_xatom_by_name ("WM_TAKE_FOCUS");

    /* fill hint struct with to NO get keyboard inputs*/  
    wm_hints.flags = InputHint;
    wm_hints.input = False;
      
    gdk_error_trap_push ();

    /* apply hint for window */
    XSetWMHints (GDK_WINDOW_XDISPLAY (widget->window),
	         GDK_WINDOW_XWINDOW (widget->window), &wm_hints);
    
    XSetWMProtocols (GDK_WINDOW_XDISPLAY (widget->window),
		   GDK_WINDOW_XWINDOW (widget->window), wm_window_protocols, 3);

    gdk_error_trap_pop ();

    if (is_dock) 
    {
	brlmon_main_set_wm_dock (TRUE);
    }
    else 
    {
	  if (widget->window) 
	  {
		  gdk_window_set_decorations (widget->window, GDK_DECOR_ALL | 
					      GDK_DECOR_MINIMIZE | GDK_DECOR_MAXIMIZE);
		  gdk_window_set_functions (widget->window, GDK_FUNC_MOVE | GDK_FUNC_RESIZE);
	  }
    }
    
    brlmon_update_struts (our_resize_height, widget);

    /* stick window for all desktop screen */
    gtk_window_stick (GTK_WINDOW (widget));

    gdk_window_add_filter (widget->window,
			   brlmon_discard_focus_filter,
			   NULL);
}

/**
 * brlmon_main_create_window
 * 
 * is_dock: type of window.
 *
 * Create a window widget.
 *
 * return: pointer for the new created window.
**/
GtkWidget* 
brlmon_main_create_window (gboolean is_dock)
{
	GtkWidget *window1;
	_screen_width = gdk_screen_get_width (gdk_screen_get_default ()); 
        /* FIXME: is brlmon always on the default screen? */
						     
	window1 = g_object_connect (gtk_widget_new (gtk_window_get_type (),
				"user_data", NULL,
				"can_focus", FALSE,
				"type", GTK_WINDOW_TOPLEVEL,
				"window-position", GTK_WIN_POS_CENTER,
				"title","Braille Monitor",
				"allow_grow", TRUE,
				"allow_shrink", TRUE,
				"border_width", 0,
				NULL),
			     "signal::realize", brlmon_on_window_realize, is_dock,
			     "signal::destroy", brlmon_on_window_destroy, NULL,
			     NULL);

	gtk_window_set_default_size (GTK_WINDOW (window1), our_resize_width, our_resize_height);

	return window1;
}

/**
 * brlmon_load_interface
 *
 * Create window and show it.
 *
 * return:
**/
gboolean 
brlmon_load_interface (void)
{    
    w_brlmon = brlmon_main_create_window (TRUE);
    
    gtk_widget_show (w_brlmon);
                    
    return TRUE;
}

/**
 * brlmon_changes_cb
 *
 * Gconf value change callback function.
 *
 * return:
**/
void
brlmon_changes_cb			(GConfClient	*client,
					guint		cnxn_id,
					GConfEntry	*entry,
					gpointer	user_data)
{        
    gchar *key = NULL;

    if (!entry || 
	!entry->value)
	return;
	    
    key = g_path_get_basename ((gchar*) gconf_entry_get_key (entry));

    if (!strcmp (BRAILLE_MONITOR_PANEL_POSITION_GCONF_KEY, key))
    {
	brlmon_main_resize_window  (brlmon_main_get_main_window (), 
				    our_resize_width,
				    our_resize_height,
				    brlmon_return_panel_position ()); 
    }
    else
    if (!strcmp (BRAILLE_MONITOR_MODE_GCONF_KEY, key))
    {
	brlmon_destroy_cell_table (FALSE, TRUE);
    }
    else
    if (!strcmp (BRAILLE_MONITOR_DOT78_GCONF_KEY, key) ||
	!strcmp (BRAILLE_MONITOR_DOT7_GCONF_KEY, key)  ||
	!strcmp (BRAILLE_MONITOR_DOT8_GCONF_KEY, key))
    {
	brlmon_update_text_color ();
    }
    else
    if (!strcmp (BRAILLE_MONITOR_LINE_GCONF_KEY, key) ||
	!strcmp (BRAILLE_MONITOR_COLUMN_GCONF_KEY, key))
    {
	brlmon_destroy_cell_table (TRUE, FALSE);
    }
    else
    if (!strcmp (BRAILLE_MONITOR_USE_THEME_GCONF_KEY, key))
    {
	if (entry->value->type == GCONF_VALUE_BOOL)
	{
	    brlmon_use_default_color = gconf_value_get_bool (entry->value);;
	    brlmon_update_text_color ();
	}
    }
    else
    if (!strcmp (BRAILLE_MONITOR_FONT_SIZE_GCONF_KEY    , key))
    {
	if (entry->value->type == GCONF_VALUE_INT)
	{
	    brlmon_destroy_cell_table (TRUE, FALSE);
	}
    }

    g_free (key);
}
