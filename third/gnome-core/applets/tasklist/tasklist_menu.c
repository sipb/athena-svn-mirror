#include <config.h>
#include <gtk/gtk.h>
#include <gnome.h>
#include "tasklist_applet.h"
#include "pixmaps.h"

GtkWidget *get_popup_menu (TasklistTask *task);
void add_menu_item (gchar *name, GtkWidget *menu, MenuAction action, gchar **xpm);
gboolean cb_menu (GtkWidget *widget, gpointer data);
void cb_to_desktop (GtkWidget *widget, gpointer data);
void cb_menu_position (GtkMenu *menu, gint *x, gint *y, gpointer user_data);

extern TasklistConfig Config;
extern GtkWidget *area;
extern GtkWidget *applet;
TasklistTask *current_task;

/* Callback for menu positioning */
void
cb_menu_position (GtkMenu *menu, gint *x, gint *y, gpointer user_data)
{
	GtkRequisition mreq;
	gint wx, wy;
	TasklistTask *task;

	current_task = task = (TasklistTask *)user_data;

	gtk_widget_get_child_requisition (GTK_WIDGET (menu), &mreq);
	gdk_window_get_origin (area->window, &wx, &wy);

	switch (applet_widget_get_panel_orient (APPLET_WIDGET (applet))) {
	case ORIENT_UP:
		*x = wx + task->x;
		*y = wy - mreq.height + task->y;
		break;
	case ORIENT_DOWN:
		*x = wx + task->x;
		*y = wy + task->y + task->height;
		break;
	case ORIENT_LEFT:
		*y = wy + task->y;
		*x = wx - mreq.width + task->x;
		break;
	case ORIENT_RIGHT:
		*y = wy + task->y;
		*x = wx + task->width;
		printf ("right\n");
		break;
	}

}

/* Callback for menus */
gboolean
cb_menu (GtkWidget *widget, gpointer data)
{
	switch (GPOINTER_TO_INT (data)) {
	case MENU_ACTION_SHADE_UNSHADE:
		if (GWMH_TASK_SHADED (current_task->gwmh_task))
			gwmh_task_unset_gstate_flags (current_task->gwmh_task,
						      GWMH_STATE_SHADED);
		else
			gwmh_task_set_gstate_flags (current_task->gwmh_task,
						    GWMH_STATE_SHADED);
		break;
	case MENU_ACTION_STICK_UNSTICK:
		if (GWMH_TASK_STICKY (current_task->gwmh_task))
			gwmh_task_unset_gstate_flags (current_task->gwmh_task,
						      GWMH_STATE_STICKY);
		else
			gwmh_task_set_gstate_flags (current_task->gwmh_task,
						    GWMH_STATE_STICKY);
		break;
	case MENU_ACTION_KILL:
		if (Config.confirm_before_kill) {
			GtkWidget *dialog;
			gint retval;

			dialog = gnome_message_box_new(_("Warning! Unsaved changes will be lost!\nProceed?"),
						       GNOME_MESSAGE_BOX_WARNING,
						       GNOME_STOCK_BUTTON_YES,
						       GNOME_STOCK_BUTTON_NO,
						       NULL);
			gtk_widget_show(dialog);
			retval = gnome_dialog_run(GNOME_DIALOG(dialog));

			if (retval)
				return TRUE;

			gwmh_task_kill(current_task->gwmh_task);
		}
		else
			gwmh_task_kill (current_task->gwmh_task);
		break;
	case MENU_ACTION_SHOW_HIDE:
		if (GWMH_TASK_ICONIFIED (current_task->gwmh_task)) {
			gwmh_desk_set_current_area (current_task->gwmh_task->desktop,
						    current_task->gwmh_task->harea,
						    current_task->gwmh_task->varea);
			gwmh_task_show (current_task->gwmh_task);
		}
		else
			gwmh_task_iconify (current_task->gwmh_task);
		break;
	case MENU_ACTION_CLOSE:
		gwmh_task_close (current_task->gwmh_task);
		break;
		
	default:
		g_print ("Menu Callback: %d\n", GPOINTER_TO_INT (data));
	}
	return FALSE;
}

/* Open a popup menu with window operations */
void
menu_popup (TasklistTask *task, guint button, guint32 activate_time)
{
	if(task->menu)
		gtk_widget_destroy(task->menu);

	task->menu = get_popup_menu (task);

	gtk_signal_connect(GTK_OBJECT(task->menu), "destroy",
			   GTK_SIGNAL_FUNC(gtk_widget_destroyed),
			   &task->menu);

	gtk_menu_popup (GTK_MENU (task->menu), NULL, NULL,
			cb_menu_position, task,
			button, activate_time);
}

/* Add a menu item to the popup menu */
void 
add_menu_item (gchar *name, GtkWidget *menu, MenuAction action, gchar **xpm)
{
	GtkWidget *menuitem;
	GdkPixmap *pixmap;
	GtkWidget *label;
	GdkBitmap *mask;
	GtkWidget *gtkpixmap;

	menuitem = gtk_pixmap_menu_item_new ();
	label = gtk_label_new (name);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_container_add (GTK_CONTAINER (menuitem), label);
	if (xpm) {
		pixmap = gdk_pixmap_create_from_xpm_d (area->window, &mask, NULL, xpm);
		gtkpixmap = gtk_pixmap_new (pixmap, mask);
		gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (menuitem), gtkpixmap);
	}
	
	gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			    GTK_SIGNAL_FUNC (cb_menu), GINT_TO_POINTER (action));

	gtk_widget_show_all (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);

}

/* Called when "Send to desktop" is used */
void
cb_to_desktop (GtkWidget *widget, gpointer data)
{
	gwmh_task_set_desktop (current_task->gwmh_task, 
			       GPOINTER_TO_INT (data));
	gwmh_task_set_desktop (current_task->gwmh_task, 
			       GPOINTER_TO_INT (data));
	layout_tasklist ();
}

/* Create a popup menu */
GtkWidget 
*get_popup_menu (TasklistTask *task)
{
	GtkWidget *menu, *menuitem; /*, *desktop, *label, *gtkpixmap;*/
	/*GdkPixmap *pixmap;*/
	/*GdkBitmap *mask;*/
	/*GwmhDesk *desk_info;*/

	/*gchar *wsname;*/
	/*int i, curworkspace;*/

	menu = gtk_menu_new ();
	gtk_widget_show (menu);

	add_menu_item (GWMH_TASK_ICONIFIED (task->gwmh_task)
		       ? _("Restore") : _("Iconify"), 
		       menu, MENU_ACTION_SHOW_HIDE,
		       GWMH_TASK_ICONIFIED (task->gwmh_task)
		       ? tasklist_restore_xpm : tasklist_iconify_xpm);

	add_menu_item (GWMH_TASK_SHADED (task->gwmh_task)
		       ? _("Unshade") : _("Shade"), 
		       menu, MENU_ACTION_SHADE_UNSHADE,
		       GWMH_TASK_SHADED (task->gwmh_task)
		       ? tasklist_unshade_xpm: tasklist_shade_xpm);

	add_menu_item (GWMH_TASK_STICKY (task->gwmh_task)
		       ? _("Unstick") : _("Stick"), 
		       menu, MENU_ACTION_STICK_UNSTICK,
		       GWMH_TASK_STICKY (task->gwmh_task)
		       ? tasklist_unstick_xpm : tasklist_stick_xpm);
#if 0
	menuitem = gtk_pixmap_menu_item_new ();
	label = gtk_label_new (_("To desktop"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_container_add (GTK_CONTAINER (menuitem), label);
	pixmap = gdk_pixmap_create_from_xpm_d (area->window, &mask, NULL,
					       tasklist_send_to_desktop_xpm);
	gtkpixmap = gtk_pixmap_new (pixmap, mask);
	gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (menuitem), gtkpixmap);
	gtk_widget_show_all (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);

	if (!GWMH_TASK_STICKY (task->gwmh_task)) {
		desktop = gtk_menu_new ();
		gtk_widget_show (desktop);
		gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), desktop);
		
		desk_info = gwmh_desk_get_config ();
		curworkspace = desk_info->current_desktop;

		for (i=0; i<desk_info->n_desktops;i++) {
			if (desk_info->desktop_names[i])
				wsname = g_strdup_printf ("%s", desk_info->desktop_names[i]);
			else
				wsname = g_strdup_printf ("%d", i);
			menuitem = gtk_menu_item_new_with_label (wsname);
			gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
					    GTK_SIGNAL_FUNC (cb_to_desktop), i);
			if (i == curworkspace)
				gtk_widget_set_sensitive (menuitem, FALSE);
			gtk_widget_show (menuitem);
			gtk_menu_append (GTK_MENU (desktop), menuitem);
			g_free (wsname);
		}
	} else 
		gtk_widget_set_sensitive (menuitem, FALSE);

	menuitem = gtk_menu_item_new ();
	gtk_widget_show (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);
#endif
	add_menu_item (_("Close window"), menu, MENU_ACTION_CLOSE,
		       tasklist_close_xpm);

	menuitem = gtk_menu_item_new ();
	gtk_widget_show (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);
	
	add_menu_item (_("Kill app"), menu, MENU_ACTION_KILL, tasklist_kill_xpm);
	
	return menu;
}
