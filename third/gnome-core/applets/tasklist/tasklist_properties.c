#include <config.h>
#include "tasklist_applet.h"

/* The tasklist configuration */
extern TasklistConfig Config;

/* The tasklist properties configuration */
TasklistConfig PropsConfig;

/* The Property box */
GtkWidget *prop = NULL;

/* Callback for apply */
static void
cb_apply (GtkWidget *widget, gint page, gpointer data)
{

	/* Copy the Property struct back to the Config struct */
	memcpy (&Config, &PropsConfig, sizeof (TasklistConfig));

	/* Redraw everything */
	change_size (TRUE);
}

/* Callback for radio buttons */
static void
cb_radio_button (GtkWidget *widget, gint *data)
{

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
		*data = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget),
							      "number"));
		gnome_property_box_changed (GNOME_PROPERTY_BOX (prop));
	}
}

/* Callback for spin buttons */
static void
cb_spin_button (GtkWidget *widget, gint *data)
{
	gnome_property_box_changed (GNOME_PROPERTY_BOX (prop));
	
	*data = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
}

/* Callback for check buttons */
static void
cb_check_button (GtkWidget *widget, gboolean *data)
{
	gnome_property_box_changed (GNOME_PROPERTY_BOX (prop));

	*data = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

static void
cb_check_button_disable (GtkWidget *widget, GtkWidget *todisable)
{
	gboolean active;
	active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
	gtk_widget_set_sensitive (todisable, !active);
}

 
/* Create a spin button */
static GtkWidget *
create_spin_button (gchar *name,
		    gint *init_value,
		    gfloat min_value,
		    gfloat max_value,
		    gfloat page_value)
{
	GtkObject *adj;
	GtkWidget *spin;
	GtkWidget *hbox;
	GtkWidget *label;

	adj = gtk_adjustment_new (*init_value,
				  min_value,
				  max_value,
				  1,
				  page_value,
				  page_value);
	
	hbox = gtk_hbox_new (TRUE, GNOME_PAD_SMALL);

	spin = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1, 0);
	gtk_signal_connect (GTK_OBJECT (spin), "changed",
			    GTK_SIGNAL_FUNC (cb_spin_button), init_value);
						


	label = gtk_label_new (name);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);

	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), spin, FALSE, TRUE, 0);

	return hbox;
}

/* Create a radio button */
static GtkWidget *
create_radio_button (gchar *name, GSList **group, 
		     gint number, gint *change_value)
{
	GtkWidget *radiobutton;
	
	radiobutton = gtk_radio_button_new_with_label (*group, name);
	*group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobutton));

	if (number == *change_value)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton), TRUE);
	
	gtk_signal_connect (GTK_OBJECT (radiobutton), "toggled",
			    GTK_SIGNAL_FUNC (cb_radio_button), change_value);
	gtk_object_set_data (GTK_OBJECT (radiobutton), "number",
			     GINT_TO_POINTER (number));


	return radiobutton;
}

/* Create a check button */
static GtkWidget *
create_check_button (gchar *name, gboolean *change_value)
{
	GtkWidget *checkbutton;

	checkbutton = gtk_check_button_new_with_label (name);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton), *change_value);
	gtk_signal_connect (GTK_OBJECT (checkbutton), "toggled",
			    GTK_SIGNAL_FUNC (cb_check_button), change_value);
	return checkbutton;
}

/* Create the size page */
static void
create_size_page (void)
{
	GtkWidget *hbox,/* *table,*/ *frame, *vbox, *topbox;
	GSList *vertgroup = NULL, *horzgroup = NULL;
	GtkWidget *autobutton, *w;
	
	topbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_border_width (GTK_CONTAINER (topbox), GNOME_PAD_SMALL);
	
	autobutton = create_check_button (_("Follow panel size"),
					  &PropsConfig.follow_panel_size);
	gtk_box_pack_start (GTK_BOX (topbox),
			    autobutton,
			    FALSE, TRUE, 0);

	hbox = gtk_hbox_new (TRUE, GNOME_PAD_SMALL);
	gtk_box_pack_start_defaults (GTK_BOX (topbox), hbox);

	frame = gtk_frame_new (_("Horizontal"));

	gtk_box_pack_start_defaults (GTK_BOX (hbox), frame);
	
	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);

	gtk_box_pack_start (GTK_BOX (vbox),
			    create_spin_button (_("Tasklist width:"),
						&PropsConfig.horz_width,
						48,
						8192,
						10),
			    FALSE, TRUE, 0);
	w = create_spin_button (_("Rows of tasks:"),
				&PropsConfig.horz_rows,
				1,
				8,
				1);
	gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (autobutton), "toggled",
			   GTK_SIGNAL_FUNC (cb_check_button_disable),
			   w);
	cb_check_button_disable (autobutton, w);

	gtk_box_pack_start (GTK_BOX (vbox),
			    create_spin_button (_("Default task size:"),
						&PropsConfig.horz_taskwidth,
						48,
						350,
						10),
			    FALSE, TRUE, 0);


	gtk_box_pack_start (GTK_BOX (vbox),
			    create_radio_button (_("Tasklist width is fixed"),
						 &horzgroup, TRUE, &PropsConfig.horz_fixed),
			    FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox),
			    create_radio_button (_("Tasklist width is dynamic"), 
						 &horzgroup, FALSE, &PropsConfig.horz_fixed),
			    FALSE, TRUE, 0);
	
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	
	frame = gtk_frame_new (_("Vertical"));
	gtk_box_pack_start_defaults (GTK_BOX (hbox), frame);
	
	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);

	gtk_box_pack_start (GTK_BOX (vbox),
			    create_spin_button (_("Tasklist height:"),
						&PropsConfig.vert_height,
						48,
						1024*8,
						10),
			    FALSE, TRUE, 0);

	w = create_spin_button (_("Tasklist width:"),
				&PropsConfig.vert_width,
				48,
				512,
				10);
	gtk_box_pack_start (GTK_BOX (vbox), w, FALSE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (autobutton), "toggled",
			   GTK_SIGNAL_FUNC (cb_check_button_disable),
			   w);
	cb_check_button_disable (autobutton, w);

	gtk_box_pack_start (GTK_BOX (vbox),
			    create_radio_button (_("Tasklist height is fixed"),
						 &vertgroup, TRUE, &PropsConfig.vert_fixed),
			    FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox),
			    create_radio_button (_("Tasklist height is dynamic"), 
						 &vertgroup, FALSE, &PropsConfig.vert_fixed),
			    FALSE, TRUE, 0);

	gtk_container_add (GTK_CONTAINER (frame), vbox);

	gnome_property_box_append_page (GNOME_PROPERTY_BOX (prop), topbox,
					gtk_label_new (_("Size")));
}

static void
create_display_page (void)
{
	GtkWidget *vbox, *frame;
	GtkWidget *miscbox, *taskbox;
	/*GtkWidget *radio;*/
	/*GSList *taskgroup = NULL;*/

	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	
	frame = gtk_frame_new (_("Which tasks to show"));
	gtk_container_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), frame);

	taskbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (frame), taskbox);
	
	gtk_box_pack_start (GTK_BOX (taskbox),
			    create_check_button (_("Show normal applications"), &PropsConfig.show_normal),
			    FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (taskbox),
			    create_check_button (_("Show iconified (minimized) applications"), &PropsConfig.show_minimized),
			    FALSE, TRUE, 0);
			    
	gtk_box_pack_start (GTK_BOX (taskbox),
			    create_check_button (_("Show normal applications on all desktops"), &PropsConfig.all_desks_normal),
			    FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (taskbox),
			    create_check_button (_("Show iconified (minimized) applications on all desktops"), &PropsConfig.all_desks_minimized),
			    FALSE, TRUE, 0);

	frame = gtk_frame_new (_("Miscellaneous"));
	gtk_container_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), frame);
	
	miscbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (frame), miscbox);

	gtk_box_pack_start (GTK_BOX (miscbox),
			    create_check_button (_("Show mini icons"), &PropsConfig.show_mini_icons),
			    FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (miscbox),
			    create_check_button (_("Confirm before killing windows"), &PropsConfig.confirm_before_kill),
			    FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (miscbox),
			    create_check_button (_("Move iconified tasks to current workspace when restoring"), &PropsConfig.move_to_current),
			    FALSE, TRUE, 0);

	gnome_property_box_append_page (GNOME_PROPERTY_BOX (prop), vbox,
					gtk_label_new (_("Display")));
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "tasklist_applet",
					  "index.html#TASKLIST-APPLET-PROPERTIES" };
	gnome_help_display(NULL, &help_entry);
}

/* Display property dialog */
void
display_properties (void)
{
	if (prop != NULL)
	{
		gdk_window_show (prop->window);
		gdk_window_raise (prop->window);
		return;

	}
	/* Copy memory from the tasklist config 
	   to the tasklist properties config. */
	memcpy (&PropsConfig, &Config, sizeof (TasklistConfig));

	prop = gnome_property_box_new ();
	gtk_window_set_title (GTK_WINDOW (prop), _("Tasklist properties"));
	gtk_signal_connect (GTK_OBJECT (prop), "apply",
			    GTK_SIGNAL_FUNC (cb_apply), NULL);
	gtk_signal_connect (GTK_OBJECT (prop), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &prop);
	gtk_signal_connect (GTK_OBJECT (prop), "help",
			    GTK_SIGNAL_FUNC (phelp_cb), NULL);
	create_display_page ();
	create_size_page ();

	gtk_widget_show_all (prop);
}







