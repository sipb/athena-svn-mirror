/* magui.c
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
#include "magui.h"
#include "SRMessages.h"
#include <glade/glade.h>
#include "srintl.h"
#include "gnopiui.h"

#define MAGUI_ALIGNMENT_NONE		"none"
#define MAGUI_ALIGNMENT_CENTERED	"centered"
#define MAGUI_ALIGNMENT_AUTO		"auto"
#define MAGUI_ALIGNMENT_MIN		"min"
#define MAGUI_ALIGNMENT_MAX		"max"

enum {
    MAG_FOCUS_TRACKING_NONE = 0,
    MAG_FOCUS_TRACKING_CENTERED,
    MAG_FOCUS_TRACKING_AUTO,
    
    MAG_FOCUS_TRACKING_NUMBER
};
struct
{
    gchar *description;
    gchar *key;
} magui_smoothing[] =
{
    {N_("none"),	"none"},
    {N_("bilinear"), 	"bilinear"}
};

typedef struct
{
    gchar *description;
    gchar *key;
} TrackingNameStruct;

TrackingNameStruct magui_mouse_tracking [] = 
{
    {N_("push"), 		"mouse-push"},
    {N_("centered"),		"mouse-centered"},
    {N_("proportional"),	"mouse-proportional"},
    {N_("none"),		"none"}
};

typedef enum
{
    ZOOMER_COLUMN,
    NO_OF_COLUMNS
} ZoomerListStruct;

static gchar *magui_zoomer_list[]=
{
    N_("default"),
    NULL
};

extern Magnifier *magnifier_setting;
static Magnifier *mag_setting_clone_set = NULL;
static Magnifier *mag_setting_clone_opt = NULL;

static gchar    *selected_zoomer;
static guint 	crosswire_color_old = 0;

struct 
{
    gint size_x;
    gint size_y;
}display_size;

#define ZOMMER_BOUND(X,Y) ( X > Y ? Y : X )

GtkWidget	*w_magnifier_settings,
		*w_magnification_options;
GtkWidget	*ck_invert,
		*ck_crosswire_clip,
		*ck_crosswire_color_invert,
		*ck_cursor_on_off,
		*ck_crosswire,
		*ck_lock_factor,
		*ck_panning,
	 	*ck_cursor_mag;
GtkWidget	*et_source,
		*et_target,
		*et_zoomer_new;
GtkWidget	*tv_zoomer_list;
GtkWidget	*cb_smoothing,
		*cb_mouse_tracking;
GtkWidget	*lb_cursor_size,
		*lb_cursor_color,
		*lb_crosswire_size,
		*lb_crosswire_color;
GtkWidget	*sp_factorx,
		*sp_factory,
		*sp_border_size,
 		*sp_cursor_size,
		*sp_crosswire_size;
GtkWidget    	*sp_zp_left,
		*sp_zp_top,
		*sp_zp_width,
		*sp_zp_height;
GtkWidget	*cp_border_color,
		*cp_crosswire_color,
		*cp_cursor_color;
GtkWidget	*rb_focus_tracking[MAG_FOCUS_TRACKING_NUMBER];

static void 
magui_cursor_off (gboolean val)
{
    gboolean cursor_size = val;
    gboolean crosswire_clip = val;
	
    gtk_widget_set_sensitive ( GTK_WIDGET (ck_cursor_mag), val);
    gtk_widget_set_sensitive ( GTK_WIDGET (cp_cursor_color), val);
    gtk_widget_set_sensitive ( GTK_WIDGET (lb_cursor_color), val);
    
    if (val)
	cursor_size = magnifier_setting->cursor_scale;
		
    gtk_widget_set_sensitive (GTK_WIDGET (sp_cursor_size), cursor_size);
    gtk_widget_set_sensitive (GTK_WIDGET (lb_cursor_size), cursor_size);
	
    if (val)
    	crosswire_clip = magnifier_setting->crosswire;
	
    gtk_widget_set_sensitive ( GTK_WIDGET (ck_crosswire_clip), crosswire_clip);
}

static void
magui_cursor_magnification_off (gboolean val) 
{
    gtk_widget_set_sensitive ( GTK_WIDGET (sp_cursor_size), val);
    gtk_widget_set_sensitive ( GTK_WIDGET (lb_cursor_size), val);
}

static void 
magui_crosswire_off (gboolean val)
{
	
    gtk_widget_set_sensitive ( GTK_WIDGET (ck_crosswire_clip), val);
    gtk_widget_set_sensitive ( GTK_WIDGET (sp_crosswire_size), val);
    gtk_widget_set_sensitive ( GTK_WIDGET (cp_crosswire_color), val);
    gtk_widget_set_sensitive ( GTK_WIDGET (lb_crosswire_size), val);
    gtk_widget_set_sensitive ( GTK_WIDGET (lb_crosswire_color), val);
    gtk_widget_set_sensitive ( GTK_WIDGET (ck_crosswire_color_invert), val);    
    
    if (val)
	val = magnifier_setting->cursor_state;
	
    gtk_widget_set_sensitive ( GTK_WIDGET (ck_crosswire_clip), val);
}

static void
magui_crosswire_color_invert_off (gboolean val)
{
    gtk_widget_set_sensitive ( GTK_WIDGET (cp_crosswire_color), val);
    gtk_widget_set_sensitive ( GTK_WIDGET (lb_crosswire_color), val);   
}

#if 0
void 
magui_save_magnifier_setting_value (Magnifier *magnifier_setting)
{    
    guint8 r,g,b,a; 
    
    sru_return_if_fail (magnifier_setting);
    
    magnifier_setting->crosswire = 
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_crosswire));

    magnifier_setting->crosswire_clip = 
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_crosswire_clip));

    magnifier_setting->crosswire_size =	    
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_crosswire_size));


    gnome_color_picker_get_i8   (GNOME_COLOR_PICKER (cp_crosswire_color), 
				    &r, &g, &b, &a);
				    
    magnifier_setting->crosswire_color = 0xff - a;
    magnifier_setting->crosswire_color = (magnifier_setting->crosswire_color << 8) + r;
    magnifier_setting->crosswire_color = (magnifier_setting->crosswire_color << 8) + g;
    magnifier_setting->crosswire_color = (magnifier_setting->crosswire_color << 8) + b;

    magnifier_setting->cursor_state = 
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_cursor_on_off));

    magnifier_setting->cursor_scale = 
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_cursor_mag));
	
    magnifier_setting->cursor_size =	    
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_cursor_size));


    gnome_color_picker_get_i8   (GNOME_COLOR_PICKER (cp_cursor_color), 
				    &r, &g, &b, &a);

    magnifier_setting->cursor_color = 0xff - a;
    magnifier_setting->cursor_color = (magnifier_setting->cursor_color << 8) + r;
    magnifier_setting->cursor_color = (magnifier_setting->cursor_color << 8) + g;
    magnifier_setting->cursor_color = (magnifier_setting->cursor_color << 8) + b;    

    magconf_save_zoomer_in_schema (DEFAULT_MAGNIFIER_SCHEMA, magnifier_setting);
}
#endif

void
magui_magnification_settings_response (GtkDialog *dialog,
				       gint       response_id,
				       gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_CLOSE) 
    {
    }
    else
    if (response_id == GTK_RESPONSE_HELP) 
    {
    	gn_load_help ("gnopernicus-magnifier-prefs");
	return;
    }
    else
    {
	if (mag_setting_clone_set) 
	    magconf_save_zoomer_in_schema (DEFAULT_MAGNIFIER_SCHEMA, 
					   mag_setting_clone_set);
    }
    
    gtk_widget_hide (w_magnifier_settings);
    
    if (mag_setting_clone_set)
    {
        magconf_setting_free (mag_setting_clone_set);
        mag_setting_clone_set = NULL;
    }
}

void
magui_magnifier_settings_remove (GtkButton       *button,
                                gpointer         user_data)
{
    gtk_widget_hide (w_magnifier_settings);
    w_magnifier_settings = NULL;
    if (mag_setting_clone_set) 
    {
	magconf_save_zoomer_in_schema ( DEFAULT_MAGNIFIER_SCHEMA, 
					mag_setting_clone_set);
	magconf_setting_free (mag_setting_clone_set);
	mag_setting_clone_set = NULL;
    }
}


void
magui_options_clicked (GtkButton       *button,
                       gpointer         user_data)
{
    magui_load_magnification_options_interface ();
}

void
magui_cursor_on_off_toggled (GtkWidget *widget,
			    gpointer data)
{
    gboolean bval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_cursor_on_off)); 	    
    if (magnifier_setting->cursor_state != bval)
    {
	magnifier_setting->cursor_state = bval;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_BOOL,
				    MAGNIFIER_CURSOR,
				    (gpointer)&magnifier_setting->cursor_state);
	
	magui_cursor_off (magnifier_setting->cursor_state);
    }
}

void
magui_cursor_magnification_toggled (GtkWidget *widget,
				    gpointer data)
{
    gboolean bval = 
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_cursor_mag)); 	    
    if (magnifier_setting->cursor_scale != bval)
    {
	magnifier_setting->cursor_scale = bval;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_BOOL,
				    MAGNIFIER_CURSOR_SCALE,
				    (gpointer)&magnifier_setting->cursor_scale);

	magui_cursor_magnification_off (!magnifier_setting->cursor_scale);
    }
}

void
magui_cursor_size_value_changed(GtkWidget *widget,
			        gpointer data)
{
    gint ival = 
	gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_cursor_size));
    if (magnifier_setting->cursor_size != ival)
    {
	magnifier_setting->cursor_size = ival;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_INT,
				    MAGNIFIER_CURSOR_SIZE,
				    (gpointer)&magnifier_setting->cursor_size);    
    }
}

void
magui_crosswire_toggled(GtkWidget *widget,
			gpointer data)
{
    gboolean bval = 
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_crosswire)); 	    
    if (magnifier_setting->crosswire != bval)
    {
	magnifier_setting->crosswire = bval;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_BOOL,
				    MAGNIFIER_CROSSWIRE,
				    (gpointer)&magnifier_setting->crosswire);
	magui_crosswire_off (magnifier_setting->crosswire);
    }
}

void
magui_crosswire_clip_toggled(GtkWidget *widget,
			    gpointer data)
{
    gboolean bval = 
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_crosswire_clip)); 	    
    if (magnifier_setting->crosswire_clip != bval)
    {
	magnifier_setting->crosswire_clip = bval;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_BOOL,
				    MAGNIFIER_CROSSWIRE_CLIP,
				    (gpointer)&magnifier_setting->crosswire_clip);
    }
}

void
magui_crosswire_size_value_changed(GtkWidget *widget,
				  gpointer data)
{
    gint ival = 
	gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_crosswire_size));
    if (magnifier_setting->crosswire_size != ival)
    {
	magnifier_setting->crosswire_size = ival;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_INT,
				    MAGNIFIER_CROSSWIRE_SIZE,
				    (gpointer)&magnifier_setting->crosswire_size);    
    }
}

void
magui_cursor_color_color_set(GtkWidget *w,
			    guint r,
			    guint g,
			    guint b, 
			    guint a,
			    gpointer data)
{
    guint8 r8,g8,b8,a8;
    guint ret_color = 0;
    gnome_color_picker_get_i8   (GNOME_COLOR_PICKER (cp_cursor_color), 
				    &r8, &g8, &b8, &a8);

    ret_color = 0xff - a8;
    ret_color = (ret_color << 8) + r8;
    ret_color = (ret_color << 8) + g8;
    ret_color = (ret_color << 8) + b8;
    
    if (magnifier_setting->cursor_color != ret_color)
    {
	magnifier_setting->cursor_color = ret_color;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_INT,
				    MAGNIFIER_CURSOR_COLOR,
				    (gpointer)&magnifier_setting->cursor_color);
    }
}

void
magui_crosswire_color_color_set (GtkWidget *w,
				guint r,
				guint g,
				guint b, 
				guint a,
				gpointer data)
{
    guint8 r8,g8,b8,a8;
    guint ret_color = 0;
    gnome_color_picker_get_i8   (GNOME_COLOR_PICKER (cp_crosswire_color), 
				    &r8, &g8, &b8, &a8);

    ret_color = 0xff - a8;
    ret_color = (ret_color << 8) + r8;
    ret_color = (ret_color << 8) + g8;
    ret_color = (ret_color << 8) + b8;
    
    if (magnifier_setting->crosswire_color != ret_color)
    {
	if (ret_color == 0)
	{
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_crosswire_color_invert),
				        TRUE); 
	}
	else
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_crosswire_color_invert),
				        FALSE); 

	magnifier_setting->crosswire_color = ret_color;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_INT,
				    MAGNIFIER_CROSSWIRE_COLOR,
				    (gpointer)&magnifier_setting->crosswire_color);
    }
}

void
magui_zoomer_add_clicked(GtkWidget *widget,
			gpointer data)
{
    GtkTreeModel 	 *model	    =
	    gtk_tree_view_get_model ( GTK_TREE_VIEW (tv_zoomer_list));
    GtkTreeSelection *selection = 
	    gtk_tree_view_get_selection ( GTK_TREE_VIEW (tv_zoomer_list));
    GtkTreeIter iter;
	
    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
	return;
    
    if (GTK_IS_LIST_STORE (model))
    {
	gtk_tree_model_get (model, &iter,
        		    ZOOMER_COLUMN, &selected_zoomer,
        		    -1);
	magui_load_magnification_options_interface ();
    }
}

void
magui_zoomer_remove_clicked(GtkWidget *widget,
			    gpointer data)
{
    sru_message (_("Can NOT remove zoomer because multiple zoomers are not supported in this version."));
}

static GtkTreeModel*
magui_create_model (void)
{
    GtkListStore *store;
    GtkTreeIter iter;
    gint i = 0;
      
    store = gtk_list_store_new (NO_OF_COLUMNS, 
				G_TYPE_STRING);
    
    while (magui_zoomer_list[i])
    {
	gtk_list_store_append ( GTK_LIST_STORE (store), &iter);
	gtk_list_store_set ( GTK_LIST_STORE (store), &iter, 
		    	    ZOOMER_COLUMN, magui_zoomer_list[i],
		    	    -1);
	i++;
    }
    
    return GTK_TREE_MODEL (store);
}

void
magui_crosswire_color_invert_toggled (GtkWidget *widget,
				    gpointer data)
{
    guint8 r,g,b,a; 
    gboolean bval = 
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_crosswire_color_invert)); 	    
    
    if (bval)
    {
	gnome_color_picker_get_i8   (GNOME_COLOR_PICKER (cp_crosswire_color), 
				    &r, &g, &b, &a);
				    
	crosswire_color_old = 0xff - a;
	crosswire_color_old = (crosswire_color_old << 8) + r;
	crosswire_color_old = (crosswire_color_old << 8) + g;
	crosswire_color_old = (crosswire_color_old << 8) + b;

	if ( crosswire_color_old != 0 )
	{
	    r = g = b = 0;    
	    a = 0xff;
	    gnome_color_picker_set_i8 ( GNOME_COLOR_PICKER (cp_crosswire_color), 
					r, g, b, a);
	    magnifier_setting->crosswire_color = 0;
	    magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
					GCONF_VALUE_INT,
					MAGNIFIER_CROSSWIRE_COLOR,
					(gpointer)&magnifier_setting->crosswire_color);
	    magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
					GCONF_VALUE_INT,
					MAGNIFIER_CROSSWIRE_COLOR_INVERT,
					(gpointer)&crosswire_color_old);
	}
	magui_crosswire_color_invert_off (magnifier_setting->crosswire_color);
    }
    else
    {
	gnome_color_picker_get_i8   (GNOME_COLOR_PICKER (cp_crosswire_color), 
				    &r, &g, &b, &a);
				    
	if ( (r == 0) && (g == 0) && (b == 0) && (0xff - a == 0))
	{
	    crosswire_color_old = magconf_get_color_old (DEFAULT_MAGNIFIER_SCHEMA,
							MAGNIFIER_CROSSWIRE_COLOR_INVERT);
	    b = crosswire_color_old & 0xff;
	    g = (crosswire_color_old >> 8) & 0xff;
	    r = (crosswire_color_old >> 16) & 0xff;
	    a = 0xff - ((crosswire_color_old >> 24) & 0xff);

	    gnome_color_picker_set_i8 ( GNOME_COLOR_PICKER (cp_crosswire_color), 
					r, g, b, a);
					
	    magnifier_setting->crosswire_color = crosswire_color_old;
	    magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
					GCONF_VALUE_INT,
					MAGNIFIER_CROSSWIRE_COLOR,
					(gpointer)&magnifier_setting->crosswire_color);
	}    
	magui_crosswire_color_invert_off (magnifier_setting->crosswire_color);
    }    
    
}

static void
magui_row_activated_cb (GtkTreeView       *tree_view,
                	GtkTreePath       *path,
			GtkTreeViewColumn *column)
{
    GtkTreeModel 	 *model	    =
	    gtk_tree_view_get_model ( GTK_TREE_VIEW (tv_zoomer_list));
    GtkTreeSelection *selection = 
	    gtk_tree_view_get_selection ( GTK_TREE_VIEW (tv_zoomer_list));
    GtkTreeIter iter;
	
    if (!gtk_tree_selection_get_selected (selection, NULL, &iter))
	return;
    sru_return_if_fail (GTK_IS_LIST_STORE (model));
    gtk_tree_model_get (model, 		&iter,
            		ZOOMER_COLUMN,  &selected_zoomer,
            		-1);
    magui_load_magnification_options_interface ();
}

static gint
magui_delete_emit_response_cancel (GtkDialog *dialog,
				   GdkEventAny *event,
				   gpointer data)
{
    gtk_dialog_response (GTK_DIALOG (dialog),
			 GTK_RESPONSE_CANCEL);
    return TRUE; /* Do not destroy */
}

void
magui_set_handlers_magnifier_settings (GladeXML *xml)
{
    GtkTreeModel 	*model;
    GtkCellRenderer 	*cell_renderer;
    GtkTreeSelection 	*selection;
    GtkTreeViewColumn 	*column;
    GtkTreeIter 	iter;

    w_magnifier_settings = glade_xml_get_widget (xml, "w_magnifier_settings");

    tv_zoomer_list 	= glade_xml_get_widget (xml, "tv_zoomer_list");
    et_zoomer_new 	= glade_xml_get_widget (xml, "et_zoomer_new");
    
    ck_cursor_on_off 	= glade_xml_get_widget (xml, "ck_cursor_on_off");
    ck_cursor_mag 	= glade_xml_get_widget (xml, "ck_cursor_magnification");
    cp_cursor_color 	= glade_xml_get_widget (xml, "cp_cursor_color");
    sp_cursor_size 	= glade_xml_get_widget (xml, "sp_cursor_size");
    
    ck_crosswire_clip	= glade_xml_get_widget (xml, "ck_crosswire_clip");
    ck_crosswire 	= glade_xml_get_widget (xml, "ck_crosswire");
    sp_crosswire_size 	= glade_xml_get_widget (xml, "sp_crosswire_size");
    cp_crosswire_color 	= glade_xml_get_widget (xml, "cp_crosswire_color");
    ck_crosswire_color_invert = glade_xml_get_widget (xml, "ck_crosswire_color_invert");

    lb_cursor_size	= glade_xml_get_widget (xml, "lb_cursor_size");
    lb_cursor_color	= glade_xml_get_widget (xml, "lb_cursor_color");
    lb_crosswire_size	= glade_xml_get_widget (xml, "lb_crosswire_size");
    lb_crosswire_color	= glade_xml_get_widget (xml, "lb_crosswire_color");


    gtk_spin_button_set_range ( GTK_SPIN_BUTTON (sp_cursor_size), MIN_CURSOR_SIZE, MAX_CURSOR_SIZE);
    gtk_spin_button_set_range ( GTK_SPIN_BUTTON (sp_crosswire_size), MIN_CROSSWIRE_SIZE, MAX_CROSSWIRE_SIZE);
    
    glade_xml_signal_connect (xml, "on_w_magnifier_settings_remove",		
			    GTK_SIGNAL_FUNC (magui_magnifier_settings_remove));

    g_signal_connect (w_magnifier_settings, "response",
		      G_CALLBACK (magui_magnification_settings_response), NULL);
    g_signal_connect (w_magnifier_settings, "delete_event",
                      G_CALLBACK (magui_delete_emit_response_cancel), NULL);

    glade_xml_signal_connect (xml, "on_bt_options_clicked",			
			    GTK_SIGNAL_FUNC (magui_options_clicked)); 
			    
    glade_xml_signal_connect (xml, "on_bt_zoomer_add_clicked",			
			    GTK_SIGNAL_FUNC (magui_zoomer_add_clicked)); 
    glade_xml_signal_connect (xml, "on_bt_zoomer_remove_clicked",			
			    GTK_SIGNAL_FUNC (magui_zoomer_remove_clicked)); 

    glade_xml_signal_connect (xml, "on_ck_cursor_on_off_toggled",	
			    GTK_SIGNAL_FUNC (magui_cursor_on_off_toggled));
    glade_xml_signal_connect (xml, "on_ck_cursor_magnification_toggled",	
			    GTK_SIGNAL_FUNC (magui_cursor_magnification_toggled));
    glade_xml_signal_connect (xml, "on_sp_cursor_size_value_changed",		
			    GTK_SIGNAL_FUNC (magui_cursor_size_value_changed));
    glade_xml_signal_connect(xml,"on_cp_cursor_color_color_set",		
			    GTK_SIGNAL_FUNC(magui_cursor_color_color_set));

				
    glade_xml_signal_connect (xml, "on_ck_crosswire_toggled",		
			    GTK_SIGNAL_FUNC (magui_crosswire_toggled));
    glade_xml_signal_connect (xml, "on_ck_crosswire_color_invert_toggled",		
			    GTK_SIGNAL_FUNC (magui_crosswire_color_invert_toggled));
    glade_xml_signal_connect (xml, "on_ck_crosswire_clip_toggled",		
			    GTK_SIGNAL_FUNC (magui_crosswire_clip_toggled));
    glade_xml_signal_connect (xml, "on_sp_crosswire_size_value_changed",		
			    GTK_SIGNAL_FUNC (magui_crosswire_size_value_changed));
    glade_xml_signal_connect(xml,"on_cp_crosswire_color_color_set",		
			    GTK_SIGNAL_FUNC(magui_crosswire_color_color_set));
    
    model 	= magui_create_model ();
    
    gtk_tree_view_set_model (GTK_TREE_VIEW (tv_zoomer_list), model);
    
    g_signal_connect (tv_zoomer_list, "row_activated", 
		      G_CALLBACK (magui_row_activated_cb), model);

	    
    g_object_unref (G_OBJECT (model));

    cell_renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes   (_("Zoomers"),
    							cell_renderer,
							"text", ZOOMER_COLUMN,
							NULL);	
/*    gtk_tree_view_column_set_sort_column_id (column, ZOOMER_COLUMN);*/
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv_zoomer_list), column);
        
    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (tv_zoomer_list));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);    
    
    if (gtk_tree_model_get_iter_first ( model, &iter ))
    {
	gtk_tree_selection_select_iter (selection, &iter);
    }
}

/**
 * Magnifier APIs
**/
gboolean
magui_load_magnifier_settings_interface (GtkWidget *parent_window)
{
    if (!w_magnifier_settings)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Magnifier_Settings/magnifier_settings.glade2", "w_magnifier_settings");
	sru_return_val_if_fail (xml, FALSE);
	magui_set_handlers_magnifier_settings (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_magnifier_settings),
				      GTK_WINDOW (parent_window));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_magnifier_settings), 
					    TRUE);
    }
    else
	gtk_widget_show (w_magnifier_settings);
        
    if (magnifier_setting) 
	magconf_setting_free (magnifier_setting);
    
    magnifier_setting = magconf_load_zoomer_from_schema (DEFAULT_MAGNIFIER_SCHEMA, 
							 DEFAULT_MAGNIFIER_ID);    
    
    mag_setting_clone_set = magconf_setting_clone (magnifier_setting);
    
    magui_magnifier_setting_value_add_to_widgets (magnifier_setting);
    
    return TRUE;
}

gboolean
magui_set_crosswire (Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnifier_settings) 
	return FALSE;

    magui_crosswire_off (magnifier_setting->crosswire);
    
    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (ck_crosswire), 
				    magnifier_setting->crosswire);
    
    return TRUE;
}

gboolean
magui_set_crosswire_clip (Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnifier_settings) 
	return FALSE;
        
    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (ck_crosswire_clip), 
				    magnifier_setting->crosswire_clip);
    
    return TRUE;				    
}

gboolean
magui_set_crosswire_size (Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnifier_settings) 
	return FALSE;
        
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON (sp_crosswire_size), 
				magnifier_setting->crosswire_size);
    
    return TRUE;
}

gboolean
magui_set_crosswire_color (Magnifier *magnifier_setting)
{
    guint8 r,g,b,a; 
    
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnifier_settings) 
	return FALSE;
        
    
    b = magnifier_setting->crosswire_color & 0xff;
    g = (magnifier_setting->crosswire_color >> 8) & 0xff;
    r = (magnifier_setting->crosswire_color >> 16) & 0xff;
    a = 0xff - ((magnifier_setting->crosswire_color >> 24) & 0xff);
    
    gnome_color_picker_set_i8 ( GNOME_COLOR_PICKER (cp_crosswire_color), 
				r, g, b, a);

    if (!magnifier_setting->crosswire_color)
    {
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
				     (ck_crosswire_color_invert),
				     TRUE); 
	gtk_widget_set_sensitive     (GTK_WIDGET
				      (cp_crosswire_color),
				      FALSE);				     
    }
    else
    {
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
				     (ck_crosswire_color_invert),
				     FALSE); 
    }
    
    return TRUE;
}

gboolean
magui_set_cursor (Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnifier_settings) 
	return FALSE;

    magui_cursor_off (magnifier_setting->cursor_state);
    
    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (ck_cursor_on_off), 
				    magnifier_setting->cursor_state);

    return TRUE;
}

gboolean
magui_set_cursor_mag (Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnifier_settings) 
	return FALSE;

    magui_cursor_magnification_off (magnifier_setting->cursor_scale);
    
    gtk_toggle_button_set_active (  GTK_TOGGLE_BUTTON (ck_cursor_mag), 
				    magnifier_setting->cursor_scale);
    return TRUE;
}

gboolean
magui_set_cursor_size (Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnifier_settings) 
	return FALSE;

    gtk_spin_button_set_value ( GTK_SPIN_BUTTON (sp_cursor_size), 
				magnifier_setting->cursor_size);
    return TRUE;
}

gboolean
magui_set_cursor_color (Magnifier *magnifier_setting)
{
    guint8 r,g,b,a; 
    
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnifier_settings) 
	return FALSE;

    b = magnifier_setting->cursor_color & 0xff;
    g = (magnifier_setting->cursor_color >> 8) & 0xff;
    r = (magnifier_setting->cursor_color >> 16) & 0xff;
    a = 0xff - ((magnifier_setting->cursor_color >> 24) & 0xff);
    
    gnome_color_picker_set_i8 ( GNOME_COLOR_PICKER (cp_cursor_color), 
				r, g, b, a);

    return TRUE;
}

gboolean
magui_magnifier_setting_value_add_to_widgets (Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnifier_settings) 
	return FALSE;
    
    magui_set_crosswire (magnifier_setting);
    
    magui_set_crosswire_clip (magnifier_setting);

    magui_set_crosswire_size (magnifier_setting);

    magui_set_crosswire_color (magnifier_setting);

    magui_set_cursor (magnifier_setting);

    magui_set_cursor_mag (magnifier_setting);

    magui_set_cursor_size (magnifier_setting);
    
    magui_set_cursor_color (magnifier_setting);

    return TRUE;
}

/**
		Cursor option UI.
**/
#if 0
void
magui_magnification_options_save_changed (void)
{
    guint8 r,g,b,a; 
    gchar *tmp;
    gint iter;
    	
    magnifier_setting->border_width = 
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_border_size));
	    
    gnome_color_picker_get_i8   (GNOME_COLOR_PICKER (cp_border_color), 
				    &r, &g, &b, &a);
				    
    magnifier_setting->border_color = 0xff - a;
    magnifier_setting->border_color = (magnifier_setting->border_color << 8) + r;
    magnifier_setting->border_color = (magnifier_setting->border_color << 8) + g;
    magnifier_setting->border_color = (magnifier_setting->border_color << 8) + b;

    (magnifier_setting->zp).left =	    
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_zp_left));

    (magnifier_setting->zp).top =	    
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_zp_top));

    (magnifier_setting->zp).height =	    
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_zp_height));

    (magnifier_setting->zp).width =	    
	    gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_zp_width));
	
    if (magnifier_setting->target) 
	    g_free (magnifier_setting->target);

    if (magnifier_setting->source) 
	    g_free (magnifier_setting->source);
	    
    magnifier_setting->target = g_strdup (
				    gtk_entry_get_text ( 
				    GTK_ENTRY (et_target))
					);
    magnifier_setting->source = g_strdup (
				    gtk_entry_get_text ( 
				    GTK_ENTRY (et_source))
				    );

    tmp = (gchar*)gtk_entry_get_text (GTK_ENTRY 
				     (GTK_COMBO 
				     (cb_smoothing)->entry
				     ));

    if (tmp && strcmp (magnifier_setting->smoothing, tmp))
    {
	for (iter = 0; iter < G_N_ELEMENTS (magui_smoothing); iter++)
	{
	    if (strcmp (magui_smoothing[iter].description, tmp) == 0)
	    {
		g_free (magnifier_setting->smoothing);
		magnifier_setting->smoothing = 
		    g_strdup (magui_smoothing[iter].key);
		break;
	    }
	}
    }	 

    tmp = (gchar*)gtk_entry_get_text (GTK_ENTRY 
				     (GTK_COMBO 
				     (cb_mouse_tracking)->entry)
				     );
				     
    if (tmp && strcmp (magnifier_setting->mouse_tracking, tmp))
    {
	for (iter = 0; iter < G_N_ELEMENTS (magui_mouse_tracking); iter++)
	{
	    if (strcmp (magui_mouse_tracking[iter].description, tmp) == 0)
	    {
		g_free (magnifier_setting->mouse_tracking);
		magnifier_setting->mouse_tracking = 
		    g_strdup (magui_mouse_tracking[iter].description);
		break;
	    }
	}
    }	 
    
    magnifier_setting->panning = 
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_panning));
	
    magnifier_setting->zoomfactor_lock = 
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_lock_factor));
	    
    magnifier_setting->zoomfactorx	= 
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (sp_factorx));

    g_message ("zoom factor x = %f", magnifier_setting->zoomfactorx);
	
    if (magnifier_setting->zoomfactor_lock)
    {
	magnifier_setting->zoomfactory	= magnifier_setting->zoomfactorx;
    }
    else
    {
	magnifier_setting->zoomfactory	= 
	    gtk_spin_button_get_value (GTK_SPIN_BUTTON (sp_factory));
    }
    
    magnifier_setting->invert = 
	    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_invert));

	
    magconf_save_zoomer_in_schema (DEFAULT_MAGNIFIER_SCHEMA, magnifier_setting);
}
#endif

static gboolean
magui_check_for_valid_screen (const gchar *new_val)
{
    GdkDisplay *disp = NULL;
		
    if (!new_val)
		return FALSE;
    
    disp = gdk_display_open (new_val);
    if (disp)
    {
	/*gdk_display_close (disp); waiting for bug #85715*/
	return TRUE;
    }
    return FALSE;
}

static void 
magui_set_source_screen ()
{
    const gchar *sval = 
	gtk_entry_get_text ( GTK_ENTRY (et_source));
		
    if (magui_check_for_valid_screen (sval))
    {
        g_free (magnifier_setting->source);
        magnifier_setting->source = g_strdup (sval);

        magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
 				     GCONF_VALUE_STRING,
				     MAGNIFIER_SOURCE,
				     (gpointer)magnifier_setting->source);
    }
    else	
    {
    	gdk_beep ();
    	gtk_entry_set_text (GTK_ENTRY (et_source), 
			    g_strdup (magnifier_setting->source));
    }
}

static void
magui_set_target_screen ()
{
    const gchar *sval = 
	gtk_entry_get_text ( GTK_ENTRY (et_target));
	
    if (magui_check_for_valid_screen (sval))
    {
	g_free (magnifier_setting->target);
	magnifier_setting->target = g_strdup (sval);
	
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				     GCONF_VALUE_STRING,
				     MAGNIFIER_TARGET,
				     (gpointer)magnifier_setting->target);
    }
    else	
    {
	gdk_beep ();
    	gtk_entry_set_text (GTK_ENTRY (et_target), 
    			    g_strdup (magnifier_setting->source));
    }
}	

void
magui_magnification_options_response (GtkDialog *dialog,
				      gint       response_id,
				      gpointer   user_data)
{
    if (response_id == GTK_RESPONSE_APPLY)
    {
    	magui_set_source_screen ();
	magui_set_target_screen ();
	return;
    } 
    else
    if (response_id == GTK_RESPONSE_OK)
    {
	magui_set_source_screen ();
	magui_set_target_screen ();
    }
    else
    if (response_id == GTK_RESPONSE_CLOSE) 
    {
        if (mag_setting_clone_opt)
            {
                magconf_setting_free (mag_setting_clone_opt);
		mag_setting_clone_opt = NULL;
	    }
	/*magui_magnification_options_save_changed ();*/
    }
    else
    if (response_id == GTK_RESPONSE_HELP) 
    {
    	gn_load_help ("gnopernicus-magnifier-prefs");
	return;
    }
    else
    {
	if (mag_setting_clone_opt)
	{
	    magconf_copy_option_part 	(magnifier_setting, 
					 mag_setting_clone_opt);
	    magconf_setting_free (mag_setting_clone_opt);
	    mag_setting_clone_opt = NULL;
	}
    }
    gtk_widget_hide (w_magnification_options);
}

void
magui_magnification_options_remove  (GtkButton       *button,
                                     gpointer         user_data)
{
    if (mag_setting_clone_opt)
    {
	magconf_copy_option_part (magnifier_setting, 
				 mag_setting_clone_opt);
	magconf_setting_free (mag_setting_clone_opt);
	mag_setting_clone_opt = NULL;
    }
    gtk_widget_hide (w_magnification_options);
    w_magnification_options = NULL;
}


void
magui_color_border_color_set (GtkWidget *w,
			    guint r,
			    guint g,
			    guint b, 
			    guint a,
			    gpointer data)
{
    guint8 r8,g8,b8,a8;
    guint ret_color = 0;
    gnome_color_picker_get_i8   (GNOME_COLOR_PICKER (cp_border_color), 
				    &r8, &g8, &b8, &a8);

    ret_color = 0xff - a8;
    ret_color = (ret_color << 8) + r8;
    ret_color = (ret_color << 8) + g8;
    ret_color = (ret_color << 8) + b8;
    
    if (magnifier_setting->border_color != ret_color)
    {
	magnifier_setting->border_color = ret_color;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_INT,
				    MAGNIFIER_BORDER_COLOR,
				    (gpointer)&magnifier_setting->border_color);
    }
}

void
magui_border_size_value_changed (GtkWidget *widget,
				gpointer user_data)
{
    gint ival = 
	gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_border_size));
    if (magnifier_setting->border_width != ival)
    {
	magnifier_setting->border_width = ival;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_INT,
				    MAGNIFIER_BORDER_WIDTH,
				    (gpointer)&magnifier_setting->border_width);    
    }
}	

void
magui_zp_left_value_changed (GtkWidget *widget,
			    gpointer user_data)
{
    gint ival = 
	gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_zp_left));
    if ((magnifier_setting->zp).left != ival)
    {
	(magnifier_setting->zp).left = ival;
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON (sp_zp_width),
		(magnifier_setting->zp).left, display_size.size_x);
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_INT,
				    MAGNIFIER_ZP_LEFT,
				    (gpointer)&(magnifier_setting->zp).left);    
    }
}	

void
magui_zp_top_value_changed (GtkWidget *widget,
			    gpointer user_data)
{
    gint ival = 
	gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_zp_top));
    if ((magnifier_setting->zp).top != ival)
    {
	(magnifier_setting->zp).top = ival;
	gtk_spin_button_set_range ( GTK_SPIN_BUTTON (sp_zp_height),
		(magnifier_setting->zp).top, display_size.size_y);

	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_INT,
				    MAGNIFIER_ZP_TOP,
				    (gpointer)&(magnifier_setting->zp).top);    
    }
}	

void
magui_zp_height_value_changed (GtkWidget *widget,
			       gpointer user_data)
{
    gint ival = 
	gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_zp_height));
    if ((magnifier_setting->zp).height != ival)
    {
	(magnifier_setting->zp).height = ival;
        gtk_spin_button_set_range ( GTK_SPIN_BUTTON (sp_zp_top),
		 DEFAULT_SCREEN_MIN_SIZE,(magnifier_setting->zp).height);

	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_INT,
				    MAGNIFIER_ZP_HEIGHT,
				    (gpointer)&(magnifier_setting->zp).height);    
    }
}	

void
magui_zp_width_value_changed (GtkWidget *widget,
			      gpointer user_data)
{
    gint ival = 
	gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (sp_zp_width));

    if ((magnifier_setting->zp).width != ival)
    {
	(magnifier_setting->zp).width = ival;
	gtk_spin_button_set_range ( GTK_SPIN_BUTTON (sp_zp_left),
		 DEFAULT_SCREEN_MIN_SIZE,(magnifier_setting->zp).width);

	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_INT,
				    MAGNIFIER_ZP_WIDTH,
				    (gpointer)&(magnifier_setting->zp).width);    
    }
}	

void
magui_mouse_tracking_value_changed(GtkWidget *widget,
				  gpointer data)
{
    gchar *tmp = NULL;
    gint iter;
    tmp = (gchar*)gtk_entry_get_text (GTK_ENTRY 
				     (GTK_COMBO 
				     (cb_mouse_tracking)->entry)
				     );
				     
    if (!tmp || strlen (tmp) == 0)
	return;
	
    for (iter = 0; iter < G_N_ELEMENTS (magui_mouse_tracking); iter++)
    {
	if (strcmp (_(magui_mouse_tracking[iter].description), tmp) == 0)
	{
	    if (strcmp (magui_mouse_tracking[iter].key,
			magnifier_setting->mouse_tracking))
	    {
		    g_free (magnifier_setting->mouse_tracking);
		    magnifier_setting->mouse_tracking = 
			g_strdup (magui_mouse_tracking[iter].key);
		    magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
						GCONF_VALUE_STRING,
						MAGNIFIER_MOUSE_TRACKING,
						(gpointer)magnifier_setting->mouse_tracking);

		    break;
	    }
	}
    }	 

}

void
magui_smoothing_value_changed(GtkWidget *widget,
			    gpointer data)
{
    gchar *tmp = NULL;
    gint iter;
    tmp = (gchar*)gtk_entry_get_text (GTK_ENTRY 
				     (GTK_COMBO 
				     (cb_smoothing)->entry
				     ));

    if (tmp)
    {
	for (iter = 0; iter < G_N_ELEMENTS (magui_smoothing); iter++)
	{
	    if (strcmp (_(magui_smoothing[iter].description), tmp) == 0)
	    {	
		if (magui_smoothing[iter].key != magnifier_setting->smoothing)
		{
		    g_free (magnifier_setting->smoothing);
		    magnifier_setting->smoothing = 
			g_strdup (magui_smoothing[iter].key);
		    magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
					    GCONF_VALUE_STRING,
					    MAGNIFIER_SMOOTHING,
					    (gpointer)magnifier_setting->smoothing);
		
		    break;
		}
	    }
	}
    }	 

}

void
magui_invert_toggled(GtkWidget *widget,
		    gpointer data)
{
    gboolean bval = 
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_invert));
    if (magnifier_setting->invert != bval)
    {
	magnifier_setting->invert = bval;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_BOOL,
				    MAGNIFIER_INVERT,
				    (gpointer)&magnifier_setting->invert);
    }
}


void
magui_panning_toggled(GtkWidget *widget,
		    gpointer data)
{
    gboolean bval = 
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_panning));
    if (magnifier_setting->panning != bval)
    {
	magnifier_setting->panning = bval;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_BOOL,
				    MAGNIFIER_PANNING,
				    (gpointer)&magnifier_setting->panning);
    }
}


void
on_change_sp1_value	(GtkWidget       *widget,
        	        gpointer         user_data)
{

    gdouble dval =
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (sp_factorx));
	
    if (dval == magnifier_setting->zoomfactorx) 
	return;

    if (magnifier_setting->zoomfactor_lock)
    {
	magnifier_setting->zoomfactorx = dval;
	    		
	magnifier_setting->zoomfactory = 
	    magnifier_setting->zoomfactorx ;

	gtk_spin_button_set_value ( GTK_SPIN_BUTTON (sp_factory), 
				magnifier_setting->zoomfactory);		
	
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_FLOAT,
				    MAGNIFIER_ZOOM_FACTOR_Y,
				    (gpointer)&magnifier_setting->zoomfactory);
	
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_FLOAT,
				    MAGNIFIER_ZOOM_FACTOR_X,
				    (gpointer)&magnifier_setting->zoomfactorx);
    }
    else
    {
	magnifier_setting->zoomfactorx = dval;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_FLOAT,
				    MAGNIFIER_ZOOM_FACTOR_X,
				    (gpointer)&magnifier_setting->zoomfactorx);
    }
}

void
on_change_sp2_value	(GtkWidget       *widget,
        	        gpointer         user_data)
{
    gdouble dval =
	gtk_spin_button_get_value (GTK_SPIN_BUTTON (sp_factory));
	
    if (dval == magnifier_setting->zoomfactory) 
	return;
    
    if (magnifier_setting->zoomfactor_lock)
    {
	magnifier_setting->zoomfactorx = dval;
		
	magnifier_setting->zoomfactory = 
	    magnifier_setting->zoomfactorx ;
		
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (sp_factorx), 
		magnifier_setting->zoomfactorx);		
	
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_FLOAT,
				    MAGNIFIER_ZOOM_FACTOR_Y,
				    (gpointer)&magnifier_setting->zoomfactory);
	
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_FLOAT,
				    MAGNIFIER_ZOOM_FACTOR_X,
				    (gpointer)&magnifier_setting->zoomfactorx);
    }
    else
    {
	magnifier_setting->zoomfactory = dval;
	magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				    GCONF_VALUE_FLOAT,
				    MAGNIFIER_ZOOM_FACTOR_Y,
				    (gpointer)&magnifier_setting->zoomfactory);

    }
}

void
magui_change_lock_factor(GtkWidget       *widget,
    	        	gpointer         user_data)
{
    gboolean bval = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (ck_lock_factor));     
    
    if (magnifier_setting->zoomfactor_lock == bval) 
	return;
    
    magnifier_setting->zoomfactor_lock = bval;
	
    magconf_zoomfactor_lock_set (magnifier_setting->zoomfactor_lock);
    
    magnifier_setting->zoomfactorx = 
	magnifier_setting->zoomfactory = (  magnifier_setting->zoomfactory + 
					    magnifier_setting->zoomfactorx) / 2.0;
				
    gtk_spin_button_set_value   (GTK_SPIN_BUTTON (sp_factorx), 
				magnifier_setting->zoomfactorx);

    gtk_spin_button_set_value 	(GTK_SPIN_BUTTON (sp_factory), 
				magnifier_setting->zoomfactory);				

    magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				GCONF_VALUE_BOOL,
				MAGNIFIER_ZOOM_FACTOR_LOCK,
				(gpointer)&magnifier_setting->zoomfactor_lock);
	
    magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				GCONF_VALUE_FLOAT,
				MAGNIFIER_ZOOM_FACTOR_Y,
				(gpointer)&magnifier_setting->zoomfactory);
	
    magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				GCONF_VALUE_FLOAT,
				MAGNIFIER_ZOOM_FACTOR_X,
				(gpointer)&magnifier_setting->zoomfactorx);				
}

static void
magui_rb_focus_tracking_toggled (GtkWidget	*widget,
				gpointer 	user_data)
{
    g_free (magnifier_setting->alignment_x);
    g_free (magnifier_setting->alignment_y);
    
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_focus_tracking[MAG_FOCUS_TRACKING_NONE])))
    {
    	magnifier_setting->alignment_x = g_strdup (MAGUI_ALIGNMENT_NONE);
	magnifier_setting->alignment_y = g_strdup (MAGUI_ALIGNMENT_NONE);
    }
    else
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb_focus_tracking[MAG_FOCUS_TRACKING_CENTERED])))
    {
	magnifier_setting->alignment_x = g_strdup (MAGUI_ALIGNMENT_CENTERED);
	magnifier_setting->alignment_y = g_strdup (MAGUI_ALIGNMENT_CENTERED);
    }
    else
    {
    	magnifier_setting->alignment_x = g_strdup (MAGUI_ALIGNMENT_AUTO);
	magnifier_setting->alignment_y = g_strdup (MAGUI_ALIGNMENT_AUTO);
    }
    
    magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				 GCONF_VALUE_STRING,
				 MAGNIFIER_ALIGNMENT_X,
				(gpointer)magnifier_setting->alignment_x);				
    magconf_save_item_in_schema (DEFAULT_MAGNIFIER_SCHEMA,
				 GCONF_VALUE_STRING,
				 MAGNIFIER_ALIGNMENT_Y,
				(gpointer)magnifier_setting->alignment_x);				
}

			
void
magui_set_handlers_magnification_options(GladeXML *xml)
{    
    w_magnification_options 	= glade_xml_get_widget (xml, "w_mag_option");
    sp_border_size 		= glade_xml_get_widget (xml, "sp_border_size");
    cp_border_color		= glade_xml_get_widget (xml, "cp_border_color");

    ck_invert 		= glade_xml_get_widget (xml, "ck_invert");
    ck_panning 		= glade_xml_get_widget (xml, "ck_panning");
    cb_smoothing 	= glade_xml_get_widget (xml, "cb_smoothing");
    cb_mouse_tracking 	= glade_xml_get_widget (xml, "cb_mouse_tracking");

    sp_factorx 	  = glade_xml_get_widget (xml, "sp_zoomfactor_x");
    sp_factory 	  = glade_xml_get_widget (xml, "sp_zoomfactor_y");
    ck_lock_factor = glade_xml_get_widget (xml, "ck_lock_factor");

    sp_zp_left 	  = glade_xml_get_widget (xml, "sp_zp_left");
    sp_zp_top 	  = glade_xml_get_widget (xml, "sp_zp_top");
    sp_zp_width	  = glade_xml_get_widget (xml, "sp_zp_width");
    sp_zp_height  = glade_xml_get_widget (xml, "sp_zp_height");

    et_source  = glade_xml_get_widget (xml, "et_source");
    et_target  = glade_xml_get_widget (xml, "et_target");
	
    rb_focus_tracking[MAG_FOCUS_TRACKING_NONE] = glade_xml_get_widget (xml, "rb_focus_tracking_none");
    rb_focus_tracking[MAG_FOCUS_TRACKING_CENTERED] = glade_xml_get_widget (xml, "rb_focus_tracking_centered");
    rb_focus_tracking[MAG_FOCUS_TRACKING_AUTO] = glade_xml_get_widget (xml, "rb_focus_tracking_auto");

    gtk_spin_button_set_range ( GTK_SPIN_BUTTON (sp_factorx), MIN_ZOOM_FACTOR, MAX_ZOOM_FACTOR);
    gtk_spin_button_set_range ( GTK_SPIN_BUTTON (sp_factory), MIN_ZOOM_FACTOR, MAX_ZOOM_FACTOR);

    magconf_get_display_size (&display_size.size_x, &display_size.size_y);
    
    gtk_spin_button_set_range (GTK_SPIN_BUTTON (sp_zp_left),  DEFAULT_SCREEN_MIN_SIZE, display_size.size_x);
    gtk_spin_button_set_range (GTK_SPIN_BUTTON (sp_zp_top),   DEFAULT_SCREEN_MIN_SIZE, display_size.size_y);
    gtk_spin_button_set_range (GTK_SPIN_BUTTON (sp_zp_width), DEFAULT_SCREEN_MIN_SIZE, display_size.size_x);
    gtk_spin_button_set_range (GTK_SPIN_BUTTON (sp_zp_height),DEFAULT_SCREEN_MIN_SIZE, display_size.size_y);
    
    gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (cb_smoothing)->entry), FALSE);
    gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (cb_mouse_tracking)->entry), FALSE);
    
    
    g_signal_connect (w_magnification_options, "response",
		      G_CALLBACK (magui_magnification_options_response), NULL);
    g_signal_connect (w_magnification_options, "delete_event",
                      G_CALLBACK (magui_delete_emit_response_cancel), NULL);

    glade_xml_signal_connect (xml, "on_w_mag_option_remove",		
			    GTK_SIGNAL_FUNC (magui_magnification_options_remove));
    glade_xml_signal_connect(xml,"on_color_border_color_set",		
			    GTK_SIGNAL_FUNC(magui_color_border_color_set));
			    
    glade_xml_signal_connect (xml, "on_sp_border_size_value_changed",		
			    GTK_SIGNAL_FUNC (magui_border_size_value_changed));
    glade_xml_signal_connect (xml, "on_sp_zp_height_value_changed",		
			    GTK_SIGNAL_FUNC (magui_zp_height_value_changed));
    glade_xml_signal_connect (xml, "on_sp_zp_width_value_changed",		
			    GTK_SIGNAL_FUNC (magui_zp_width_value_changed));
    glade_xml_signal_connect (xml, "on_sp_zp_top_value_changed",		
			    GTK_SIGNAL_FUNC (magui_zp_top_value_changed));
    glade_xml_signal_connect (xml, "on_sp_zp_left_value_changed",		
			    GTK_SIGNAL_FUNC (magui_zp_left_value_changed));
    
    glade_xml_signal_connect (xml, "on_ck_panning_toggled",	
			    GTK_SIGNAL_FUNC (magui_panning_toggled));

    glade_xml_signal_connect (xml, "on_ck_invert_toggled",	
			    GTK_SIGNAL_FUNC (magui_invert_toggled));

    glade_xml_signal_connect (xml, "on_et_smoothing_changed",	
			    GTK_SIGNAL_FUNC (magui_smoothing_value_changed));

    glade_xml_signal_connect (xml, "on_et_mouse_tracking_changed",	
			    GTK_SIGNAL_FUNC (magui_mouse_tracking_value_changed));
			    
    glade_xml_signal_connect (xml, "on_sp_zoomfactor_x_value_changed",	
			    GTK_SIGNAL_FUNC (on_change_sp1_value));
    glade_xml_signal_connect (xml, "on_sp_zoomfactor_y_value_changed",	
			    GTK_SIGNAL_FUNC (on_change_sp2_value));
    glade_xml_signal_connect (xml, "on_ck_lock_factor_toggled",			
			    GTK_SIGNAL_FUNC (magui_change_lock_factor));
				
    glade_xml_signal_connect (xml, "on_rb_focus_tracking_none_clicked",
				GTK_SIGNAL_FUNC (magui_rb_focus_tracking_toggled));
    glade_xml_signal_connect (xml, "on_rb_focus_tracking_centered_clicked",
				GTK_SIGNAL_FUNC (magui_rb_focus_tracking_toggled));
    glade_xml_signal_connect (xml, "on_rb_focus_tracking_auto_clicked",
				GTK_SIGNAL_FUNC (magui_rb_focus_tracking_toggled));
}


gboolean
magui_load_magnification_options_interface ()
{
    gchar *zoomer_title;

    if (!w_magnification_options)
    {
	GladeXML *xml;
	xml = gn_load_interface ("Magnifier_Settings/magnifier_settings.glade2", "w_mag_option");
	sru_return_val_if_fail (xml, FALSE);
	magui_set_handlers_magnification_options (xml);
	g_object_unref (G_OBJECT (xml));
	gtk_window_set_transient_for (GTK_WINDOW (w_magnification_options),
				      GTK_WINDOW (w_magnifier_settings));
				    
	gtk_window_set_destroy_with_parent (GTK_WINDOW (w_magnification_options), TRUE);
    }
    else
	gtk_widget_show (w_magnification_options);
    
    mag_setting_clone_opt = magconf_setting_clone (magnifier_setting);
    
    magui_magnification_options_value_add_to_widgets (magnifier_setting);

    zoomer_title = g_strdup_printf (_("Zoomer Options (%s)"), selected_zoomer);
    gtk_window_set_title (GTK_WINDOW (w_magnification_options), zoomer_title);
    g_free (zoomer_title);
    
    g_free (selected_zoomer);
    selected_zoomer = NULL;
    
    return TRUE;
}

gboolean
magui_set_border_size (Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnification_options) 
	return FALSE;


    gtk_spin_button_set_value ( GTK_SPIN_BUTTON (sp_border_size), 
				magnifier_setting->border_width);

    return TRUE;
}

gboolean
magui_set_border_color (Magnifier *magnifier_setting)
{
    guint8 r,g,b,a; 
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnification_options) 
	return FALSE;
    
    b = (magnifier_setting->border_color & 0xff);
    g = (magnifier_setting->border_color >> 8) & 0xff;
    r = (magnifier_setting->border_color >> 16) & 0xff;
    a = 0xff - ((magnifier_setting->border_color >> 24) & 0xff);

    gnome_color_picker_set_i8 ( GNOME_COLOR_PICKER (cp_border_color), 
				r, g, b, a);
    return TRUE;
}


gboolean
magui_set_zp_left (Magnifier *magnifier_setting)
{
    if (!w_magnification_options) 
	return FALSE;
    sru_return_val_if_fail (magnifier_setting, FALSE);
    (magnifier_setting->zp).left = 
	ZOMMER_BOUND((magnifier_setting->zp).left, display_size.size_x);
    gtk_spin_button_set_value 	(GTK_SPIN_BUTTON (sp_zp_left),
				(magnifier_setting->zp).left);

    return TRUE;
}

gboolean
magui_set_zp_top (Magnifier *magnifier_setting)
{
    if (!w_magnification_options) 
	return FALSE;
    sru_return_val_if_fail (magnifier_setting, FALSE);
    (magnifier_setting->zp).top = 
	ZOMMER_BOUND((magnifier_setting->zp).top, display_size.size_y);
    gtk_spin_button_set_value 	(GTK_SPIN_BUTTON (sp_zp_top),
				(magnifier_setting->zp).top);
    return TRUE;
}

gboolean
magui_set_zp_height (Magnifier *magnifier_setting)
{
    if (!w_magnification_options) 
	return FALSE;
    sru_return_val_if_fail (magnifier_setting, FALSE);
    (magnifier_setting->zp).height = 
	ZOMMER_BOUND((magnifier_setting->zp).height, display_size.size_y);
    gtk_spin_button_set_value 	(GTK_SPIN_BUTTON (sp_zp_height),
				(magnifier_setting->zp).height);
    
    return TRUE;
}

gboolean
magui_set_zp_width (Magnifier *magnifier_setting)
{
    if (!w_magnification_options) 
	return FALSE;
    sru_return_val_if_fail (magnifier_setting, FALSE);
    (magnifier_setting->zp).width = 
	ZOMMER_BOUND((magnifier_setting->zp).width, display_size.size_x);
    gtk_spin_button_set_value 	(GTK_SPIN_BUTTON (sp_zp_width),
				(magnifier_setting->zp).width);
    return TRUE;
}

gboolean
magui_set_source (Magnifier *magnifier_setting)
{
    if (!w_magnification_options) 
	return FALSE;
    sru_return_val_if_fail (magnifier_setting, FALSE);
    gtk_entry_set_text (GTK_ENTRY (et_source),
			magnifier_setting->source);

    return TRUE;
}


gboolean
magui_set_target (Magnifier *magnifier_setting)
{
    if (!w_magnification_options) 
	return FALSE;
    sru_return_val_if_fail (magnifier_setting, FALSE);
    gtk_entry_set_text (GTK_ENTRY (et_target),
			magnifier_setting->target);

    return TRUE;
}


gboolean
magui_set_zoomfactor_x(Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnification_options) 
	return FALSE;

    gtk_spin_button_set_value   (GTK_SPIN_BUTTON (sp_factorx),
				magnifier_setting->zoomfactorx);
    return TRUE;
}

gboolean
magui_set_zoomfactor_y(Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnification_options) 
	return FALSE;

    gtk_spin_button_set_value 	(GTK_SPIN_BUTTON (sp_factory),
				magnifier_setting->zoomfactory);
    return TRUE;
}


gboolean
magui_set_zoomfactor_lock(Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnification_options) 
	return FALSE;
        
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_lock_factor),
				  magnifier_setting->zoomfactor_lock); 

    if (magnifier_setting-> zoomfactor_lock)
	magnifier_setting-> zoomfactory = magnifier_setting->zoomfactorx;
    
    magui_set_zoomfactor_x (magnifier_setting);
    
    magui_set_zoomfactor_y (magnifier_setting);
    
    return TRUE;
}


gboolean
magui_set_smoothing (Magnifier *magnifier_setting)
{
    gchar *smoothing;

    sru_return_val_if_fail (magnifier_setting, FALSE);

    if (!w_magnification_options) 
	return FALSE;

    smoothing = magnifier_setting->smoothing ? magnifier_setting->smoothing : DEFAULT_MAGNIFIER_SMOOTHING;

    if (smoothing)
    {
	gint iter;
	for (iter= 0 ; iter < G_N_ELEMENTS (magui_smoothing) ; ++iter)
	{
	    if (!strcmp (magui_smoothing[iter].key, smoothing))
	    {
		gtk_entry_set_text  (GTK_ENTRY 
				    (GTK_COMBO 
				    (cb_smoothing)->entry), 
				    _(magui_smoothing[iter].description));
		break;
	    }
	}
    }
    else
	sru_assert_not_reached ();

    return TRUE;
}

gboolean
magui_set_mouse_tracking (Magnifier *magnifier_setting)
{    
    gchar *tracking;
    
    sru_return_val_if_fail (magnifier_setting, FALSE);
    
    if (!w_magnification_options) 
	return FALSE;
    
    tracking = magnifier_setting->mouse_tracking ? magnifier_setting->mouse_tracking : DEFAULT_MAGNIFIER_MOUSE_TRACKING;
    if (tracking)
    {
	gint iter;
	for (iter= 0 ; iter < G_N_ELEMENTS (magui_mouse_tracking) ; ++iter)
	{
	    if (!strcmp (magui_mouse_tracking[iter].key, tracking))
	    {
		gtk_entry_set_text  (GTK_ENTRY 
				    (GTK_COMBO 
				    (cb_mouse_tracking)->entry), 
				    _(magui_mouse_tracking[iter].description));
		break;
	    }
	}
    }
    else
	sru_assert_not_reached ();
    
    return TRUE;
}






gboolean
magui_set_focus_tracking (Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    
    if (!w_magnification_options) 
	return FALSE;
	
    if (!strcmp (magnifier_setting->alignment_x, MAGUI_ALIGNMENT_NONE) &&
	!strcmp (magnifier_setting->alignment_x, MAGUI_ALIGNMENT_NONE))	
    {
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_focus_tracking[MAG_FOCUS_TRACKING_NONE]), TRUE);
    }
    else
    if (!strcmp (magnifier_setting->alignment_x, MAGUI_ALIGNMENT_CENTERED) &&
	!strcmp (magnifier_setting->alignment_x, MAGUI_ALIGNMENT_CENTERED))	
    {
    	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_focus_tracking[MAG_FOCUS_TRACKING_CENTERED]), TRUE);
    }
    else
    {
    	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_focus_tracking[MAG_FOCUS_TRACKING_AUTO]), TRUE);
    }			
    
    return TRUE;
}

gboolean
magui_set_invert (Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnification_options) 
	return FALSE;
        
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_invert), 
				 magnifier_setting->invert);

    return TRUE;
}

gboolean
magui_set_panning (Magnifier *magnifier_setting)
{
    sru_return_val_if_fail (magnifier_setting, FALSE);
    if (!w_magnification_options) 
	return FALSE;
        
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ck_panning), 
				 magnifier_setting->panning);

    return TRUE;
}

gboolean
magui_magnification_options_value_add_to_widgets (Magnifier *magnifier_setting)
{
    if (!w_magnification_options) 
	return FALSE;
    sru_return_val_if_fail (magnifier_setting, FALSE);
    
    magui_set_border_size (magnifier_setting);
    
    magui_set_border_color (magnifier_setting);
    
    magui_set_zp_left (magnifier_setting);
    
    magui_set_zp_top (magnifier_setting);
    
    magui_set_zp_height (magnifier_setting);
    
    magui_set_zp_width (magnifier_setting);
    
    magui_set_source (magnifier_setting);
    
    magui_set_target (magnifier_setting);
    
    magui_set_zoomfactor_lock (magnifier_setting);
        
    magui_set_invert (magnifier_setting);
    
    magui_set_panning (magnifier_setting);
    
    magui_set_smoothing (magnifier_setting);
			    
    magui_set_mouse_tracking (magnifier_setting);
	
	magui_set_focus_tracking (magnifier_setting);

    return TRUE;
}
