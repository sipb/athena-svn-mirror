#include "tasklist_applet.h"

/* The applet */
extern GtkWidget *applet;

/* The configuration */
TasklistConfig Config;

gboolean write_config (gpointer data,
		   const gchar *privcfgpath,
		   const gchar *globcfgpath)
{
	gnome_config_push_prefix (privcfgpath 
				  ? privcfgpath
				  : APPLET_WIDGET (applet)->privcfgpath);

	gnome_config_set_bool ("tasklist/follow_panel_size",
			       Config.follow_panel_size);

	gnome_config_set_bool ("tasklist/horz_fixed",
			       Config.horz_fixed);
	gnome_config_set_int ("tasklist/horz_width", 
			      Config.horz_width);
	gnome_config_set_int ("tasklist/horz_rows", 
			      Config.horz_rows);
	gnome_config_set_int ("tasklist/horz_taskwidth",
			      Config.horz_taskwidth);

	gnome_config_set_bool ("tasklist/vert_fixed",
			       Config.vert_fixed);	
	gnome_config_set_int ("tasklist/vert_height", 
			      Config.vert_height);
	gnome_config_set_int ("tasklist/vert_width",
			      Config.vert_width);

	
	gnome_config_set_bool ("tasklist/show_mini_icons",
			       Config.show_mini_icons);
	gnome_config_set_bool ("tasklist/show_normal",
			       Config.show_normal);
	gnome_config_set_bool ("tasklist/show_minimized",
			       Config.show_minimized);

	gnome_config_set_bool ("tasklist/all_desks_normal",
			       Config.all_desks_normal);
	gnome_config_set_bool ("tasklist/all_desks_minimized",
			       Config.all_desks_minimized);

	gnome_config_set_bool ("tasklist/confirm_before_kill",
			       Config.confirm_before_kill);
	gnome_config_set_bool ("tasklist/move_to_current",
			       Config.move_to_current);
	gnome_config_sync ();
	
	gnome_config_pop_prefix ();
	return FALSE;
}

void read_config (void)
{
	gnome_config_push_prefix (APPLET_WIDGET (applet)->privcfgpath);

	Config.follow_panel_size = gnome_config_get_bool ("tasklist/follow_panel_size=true");

	Config.horz_fixed = gnome_config_get_bool ("tasklist/horz_fixed=true");
	/* if the screen is not too wide, make it default to 300 */
	if (gdk_screen_width () <= 800)
		Config.horz_width = gnome_config_get_int ("tasklist/horz_width=300");
	else
		Config.horz_width = gnome_config_get_int ("tasklist/horz_width=450");
	Config.horz_rows = gnome_config_get_int ("tasklist/horz_rows=2");
	Config.horz_taskwidth = gnome_config_get_int ("tasklist/horz_taskwidth=150");
	Config.vert_fixed = gnome_config_get_bool ("tasklist/vert_fixed=true");
	Config.vert_width = gnome_config_get_int ("tasklist/vert_width=48");
	Config.vert_height = gnome_config_get_int ("tasklist/vert_height=300");

	Config.confirm_before_kill = gnome_config_get_bool ("tasklist/confirm_before_kill=true");
	
	Config.show_mini_icons = gnome_config_get_bool ("tasklist/show_mini_icons=true");
	Config.show_normal = gnome_config_get_bool ("tasklist/show_normal=true");
	Config.show_minimized = gnome_config_get_bool ("tasklist/show_minimized=true");
	Config.all_desks_normal = gnome_config_get_bool ("tasklist/all_desks_normal=false");
	Config.all_desks_minimized = gnome_config_get_bool ("tasklist/all_desks_minimized=false");
	Config.move_to_current = gnome_config_get_bool ("tasklist/move_to_current=false");
	
	gnome_config_pop_prefix ();
}


