/*
 * gnome-file-selection widget
 *
 * Author:
 *   Ettore Perazzoli <ettore@comm2000.it>
 */
#include <glib.h>

#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-util.h>
#include <libgnomeui/gnome-icon-list.h>
#include <libgnomeui/gnome-preferences.h>
#include <libgnomeui/gnome-stock.h>
#include <libgnomeui/gnome-uidefs.h>

#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "gicon.h"

#include "libgnomevfs/gnome-vfs.h"

#include "gnome-file-selection.h"
#include "gnome-file-selection-history.h"


/* GnomeIconList parameters.  */
#define ICON_LIST_SPACING          64
#define ICON_LIST_SEPARATORS       " /-_."
#define ICON_LIST_ROW_SPACING      2
#define ICON_LIST_COL_SPACING      2
#define ICON_LIST_ICON_BORDER      2
#define ICON_LIST_TEXT_SPACING     2

#define DIRECTORY_LIST_WIDTH       180
#define DIRECTORY_LIST_HEIGHT      300
#define FILE_LIST_WIDTH            ((ICON_LIST_SPACING \
                                     + ICON_LIST_COL_SPACING) * 5)
#define FILE_LIST_HEIGHT           300

#define DIRECTORY_LIST_ICON_SIZE   20

#define HISTORY_SIZE               256


static GnomeDialogClass *parent_class = NULL;

#if 0
static const gchar *gnome_file_selection_filter_key = "gnome-file-selection-filter-key";
#endif

struct _GnomeFileSelectionPrivate {
	GnomeFileSelectionHistory *history;

	guint combo_selection_changed_id;

	/* This contains the file list for the icon list, which (unluckily!)
	   does not let us query the icon texts. List of char *'s */
	GList *file_list;

	GnomeVFSAsyncHandle *async_handle;

	gboolean populating_in_progress : 1;
};


/* Utility functions.  */

static void
clean_file_list (GnomeFileSelectionPrivate *priv)
{
	GList *l;

	while ((l = priv->file_list)) {
		g_free (l->data);
		priv->file_list = g_list_remove (priv->file_list, l->data);
	}
}


static GtkWidget *
create_scrolled_window (void)
{
	GtkWidget *scrolled_window;

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				       GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_widget_show (scrolled_window);

	return scrolled_window;
}

static void
add_directory_to_clist (GtkWidget *clist, gchar *name)
{
	static gchar *text[2] =
	{NULL, NULL};
	GdkImlibImage *directory_icon;
	gint row;

	text[1] = name;
	row = gtk_clist_append (GTK_CLIST (clist), text);

	directory_icon = gicon_get_directory_icon ();

	/* Imlib should be smart enough to make it fast...  */
	gdk_imlib_render (directory_icon,
			 DIRECTORY_LIST_ICON_SIZE, DIRECTORY_LIST_ICON_SIZE);

	gtk_clist_set_pixmap (GTK_CLIST (clist), row, 0,
			     gdk_imlib_move_image (directory_icon),
			     gdk_imlib_move_mask (directory_icon));
}

static void
add_file_to_icon_list (GnomeVFSFileInfo *info,
		       GtkWidget *icon_list)
{
	GdkImlibImage *image;

	image = gicon_get_icon_for_file (info->name, TRUE);

	gnome_icon_list_append_imlib (GNOME_ICON_LIST (icon_list), image,
				      info->name);
}

#if 0
static void
add_file_to_clist (GnomeVFSFileInfo *info,
		   GtkWidget *clist)
{
	gchar tmp[256];
	gchar *text[3];

	text[0] = info->name;
	g_snprintf (tmp, 256, "%lu", (gulong) info->size);
	text[1] = tmp;
	text[2] = ctime (&info->mtime);

	gtk_clist_append (GTK_CLIST (clist), text);
}
#endif


static void
populate_callback (GnomeVFSAsyncHandle *handle,
		   GnomeVFSResult result,
		   GList *list,
		   guint entries_read,
		   gpointer callback_data)
{
	GnomeFileSelection *fs;
	GnomeVFSFileInfo *info;
	GList *element;

	g_return_if_fail (GNOME_IS_FILE_SELECTION (callback_data));

	fs = GNOME_FILE_SELECTION (callback_data);

	for (element = list; element != NULL; element = element->next) {
		info = element->data;

	        if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY
	            && ! GNOME_VFS_FILE_INFO_SYMLINK (info)) {
			add_directory_to_clist (fs->directory_clist,
						info->name);
		} else {
			add_file_to_icon_list (info, fs->file_icon_list);
			fs->priv->file_list = g_list_append (fs->priv->file_list,
							     g_strdup (info->name));
		}
	}

	if (result == GNOME_VFS_ERROR_EOF) {
		gtk_clist_thaw (GTK_CLIST (fs->directory_clist));
		gtk_clist_thaw (GTK_CLIST (fs->file_clist));
		gnome_icon_list_thaw (GNOME_ICON_LIST (fs->file_icon_list));
		fs->priv->populating_in_progress = FALSE;
	}
	
}

static void
start_populating (GnomeFileSelection *fs)
{
	g_return_if_fail (fs != NULL);
	g_return_if_fail (GNOME_IS_FILE_SELECTION (fs));

	if (fs->priv->populating_in_progress)
		return;
	fs->priv->populating_in_progress = TRUE;

	gtk_clist_freeze (GTK_CLIST (fs->directory_clist));
	gtk_clist_freeze (GTK_CLIST (fs->file_clist));
	gnome_icon_list_freeze (GNOME_ICON_LIST (fs->file_icon_list));

	gtk_clist_clear (GTK_CLIST (fs->directory_clist));
	gtk_clist_clear (GTK_CLIST (fs->file_clist));
	gnome_icon_list_clear (GNOME_ICON_LIST (fs->file_icon_list));
	gtk_adjustment_set_value (GNOME_ICON_LIST (fs->file_icon_list)->adj, 0);

	gtk_entry_set_text (GTK_ENTRY (fs->selection_entry), "");

	gnome_vfs_async_load_directory (&fs->priv->async_handle,
					fs->directory,
					(GNOME_VFS_FILE_INFO_GET_MIME_TYPE
					 | GNOME_VFS_FILE_INFO_FORCE_FAST_MIME_TYPE
					 | GNOME_VFS_FILE_INFO_FOLLOW_LINKS),
					GNOME_VFS_DIRECTORY_FILTER_NONE,
					0,
					NULL,
					1,
					populate_callback,
					fs);
}

static void
update_toolbar (GnomeFileSelection *fs)
{
	gtk_widget_set_sensitive
		(fs->back_button,
		 gnome_file_selection_history_can_back (fs->priv->history));
	gtk_widget_set_sensitive
		(fs->forward_button,
		 gnome_file_selection_history_can_forward (fs->priv->history));
	gtk_widget_set_sensitive
		(fs->up_button, strcmp (fs->directory, "/") != 0);
}

static void
update_directory_combo_list (GnomeFileSelection *fs)
{
	GtkWidget *combo_list;
	GList *popdown_list;
	gchar *p1, *p2;
	gchar *entry_text;

	/* We don't want to change the original entry text...  But unluckily
	   `gtk_combo_set_popdown_strings ()' overrides it, so we have to use
	   this *very* ugly kludge.  */
	entry_text = g_strdup (gtk_entry_get_text
			      (GTK_ENTRY (GTK_COMBO (fs->directory_combo)->entry)));

	popdown_list = NULL;

	g_return_if_fail (*fs->directory == G_DIR_SEPARATOR);

	combo_list = GTK_COMBO (fs->directory_combo)->list;
	gtk_signal_handler_block (GTK_OBJECT (combo_list),
				  fs->priv->combo_selection_changed_id);

	popdown_list = g_list_prepend (popdown_list, g_strdup (G_DIR_SEPARATOR_S));

	p1 = fs->directory + 1;
	while (1) {
		gchar *s;
		guint len;

		p2 = strchr (p1, G_DIR_SEPARATOR);
		if (p2 == NULL)
			break;

		len = p2 - fs->directory;
		s = g_malloc (len + 2);
		memcpy (s, fs->directory, len);
		s[len] = G_DIR_SEPARATOR;
		s[len + 1] = 0;
		p1 = p2 + 1;

		popdown_list = g_list_prepend (popdown_list, s);
	}

	if (strcmp (fs->directory, G_DIR_SEPARATOR_S) != 0)
		popdown_list = g_list_prepend (popdown_list,
					      g_strconcat (fs->directory,
							  G_DIR_SEPARATOR_S,
							  NULL));

	gtk_combo_set_popdown_strings (GTK_COMBO (fs->directory_combo),
				      popdown_list);

	/* Now that we are done, free the string list.  */
	{
		GList *lp;

		for (lp = popdown_list; lp != NULL; lp = lp->next) {
			g_free (lp->data);
			lp->data = NULL;
		}

		g_list_free (popdown_list);
	}

	gtk_signal_handler_unblock (GTK_OBJECT (combo_list),
				   fs->priv->combo_selection_changed_id);

	{
		GtkEntry *entry;

		entry = GTK_ENTRY (GTK_COMBO (fs->directory_combo)->entry);

		/* Ugly kludge: restore the original GtkEntry text.  */
		gtk_entry_set_text (entry, entry_text);
		gtk_entry_set_position (entry, -1);
	}

	g_free (entry_text);
}

static void
update_directory_combo_entry (GnomeFileSelection *fs)
{
	GtkEntry *entry;

	entry = GTK_ENTRY (GTK_COMBO (fs->directory_combo)->entry);

	if (strcmp (fs->directory, G_DIR_SEPARATOR_S) == 0) {
		gtk_entry_set_text (entry, fs->directory);
	} else {
		gchar *s;

		s = g_strconcat (fs->directory, G_DIR_SEPARATOR_S, NULL);
		gtk_entry_set_text (entry, s);
		g_free (s);
	}

	gtk_entry_set_position (entry, -1);
}

static void
update_filter_combo (GnomeFileSelection *fs)
{
	GtkCombo *combo;
	GtkEntry *entry;
	GtkList *list;

	combo = GTK_COMBO (fs->filter_combo);
	entry = GTK_ENTRY (combo->entry);
	list = GTK_LIST (combo->list);

	gtk_list_clear_items (list, 0, -1);

#if 0
	GList *p;
	for (p = fs->filter_list; p != NULL; p = p->next) {
		GnomeFileSelectionFilter *filter;
		GtkWidget *item;
		gchar *pattern_list;
		gchar *s;

		filter = p->data;

		pattern_list = pattern_concat (filter->pattern, " ");
		s = g_strconcat (filter->description, ": ", pattern_list, NULL);
		item = gtk_list_item_new_with_label (s);
		g_free (s);
		g_free (pattern_list);

		gtk_object_set_data (GTK_OBJECT (item), filter_key, p);
		gtk_container_add (GTK_CONTAINER (list), item);
		gtk_widget_show (item);
	}
#endif
}

static gboolean
change_dir (GnomeFileSelection *fs,
	    const gchar *path,
	    gboolean do_history)
{
	gchar *old_current_dir, *new_current_dir;

	old_current_dir = g_get_current_dir ();

	g_warning ("Changing dir to `%s'", path);

	if (chdir (path) != 0) {
		g_free (old_current_dir);
		return FALSE;
	}
	new_current_dir = g_get_current_dir ();

	if (strcmp (fs->directory, new_current_dir) != 0) {
		g_free (fs->directory);
		fs->directory = new_current_dir;
		if (do_history)
			gnome_file_selection_history_add (fs->priv->history,
							  fs->directory);
		clean_file_list (fs->priv);
		update_toolbar (fs);
		start_populating (fs);
		update_directory_combo_entry (fs);
	} else {
		g_warning ("(Not updating.)");
		g_free (new_current_dir);
	}

	if (strcmp (path, "..") == 0) {
		gchar *base_name;

		base_name = strrchr (old_current_dir, '/');

		if (base_name != NULL && base_name[1] != 0) {
			GList *p;
			guint i;
			GtkCList *clist;

			clist = GTK_CLIST (fs->directory_clist);

			for (i = 0, p = clist->row_list; p != NULL; p = p->next, i++) {
				gchar *text;

				text = GTK_CELL_TEXT (GTK_CLIST_ROW (p)->cell[1])->text;
				if (strcmp (text, base_name) == 0) {
					gtk_clist_moveto (clist, -1, 0, 0.5, 0.0);
					break;
				}
			}
		}
	}
	g_free (old_current_dir);

	return TRUE;
}


/* Toolbar buttons.  */

static void
toolbar_refresh_callback (GtkWidget *widget, gpointer data)
{
	g_return_if_fail (GNOME_IS_FILE_SELECTION (data));

	start_populating (GNOME_FILE_SELECTION (data));
}

static void
toolbar_back_callback (GtkWidget *widget, gpointer data)
{
	GnomeFileSelection *fs;
	gchar *new_dir;

	g_return_if_fail (GNOME_IS_FILE_SELECTION (data));

	fs = GNOME_FILE_SELECTION (data);
	new_dir = gnome_file_selection_history_back (fs->priv->history);
	change_dir (fs, new_dir, FALSE);
}

static void
toolbar_up_callback (GtkWidget *widget, gpointer data)
{
	g_return_if_fail (GNOME_IS_FILE_SELECTION (data));

	change_dir (GNOME_FILE_SELECTION (data),
					 "..", TRUE);
}

static void
toolbar_forward_callback (GtkWidget *widget, gpointer data)
{
	GnomeFileSelection *fs;
	gchar *new_dir;

	g_return_if_fail (GNOME_IS_FILE_SELECTION (data));

	fs = GNOME_FILE_SELECTION (data);
	new_dir = gnome_file_selection_history_forward (fs->priv->history);
	change_dir (fs, new_dir, FALSE);
}

static void
toolbar_home_callback (GtkWidget *widget, gpointer data)
{
	g_return_if_fail (GNOME_IS_FILE_SELECTION (data));

	change_dir (GNOME_FILE_SELECTION (data),
					g_get_home_dir (), TRUE);
}


static void
select_icon_callback (GtkWidget *widget,
		     int index,
		     GdkEvent *event,
		     GnomeFileSelection *fs)
{
	GtkEntry *entry;
	GList    *l;

	g_return_if_fail (fs != NULL);
	g_return_if_fail (fs->priv != NULL);
	g_return_if_fail (!fs->priv->populating_in_progress);
	g_return_if_fail (index < g_list_length (fs->priv->file_list));

	l = g_list_nth (fs->priv->file_list, index);
	g_return_if_fail (l != NULL);
	entry = GTK_ENTRY (fs->selection_entry);
	gtk_entry_set_text (entry, l->data);
	gtk_entry_set_position (entry, -1);
	gtk_entry_select_region (entry, 0, -1);
}

static void
unselect_icon_callback (GtkWidget *widget,
				   int index,
				   GdkEvent *event,
				   GnomeFileSelection *fs)
{
	gtk_entry_set_text (GTK_ENTRY (fs->selection_entry), "");
}

static void
directory_combo_changed_callback (GtkWidget *widget,
						      gpointer data)
{
	GnomeFileSelection *fs;
	GtkEntry *entry;

	g_return_if_fail (GNOME_IS_FILE_SELECTION (data));

	g_warning (__FUNCTION__);

	fs = GNOME_FILE_SELECTION (data);
	entry = GTK_ENTRY (GTK_COMBO (fs->directory_combo)->entry);

	change_dir (fs, gtk_entry_get_text (entry), TRUE);
}

static void
filter_combo_changed_callback (GtkWidget *widget,
						   GnomeFileSelection *fs)
{
#if 0
	GList *selection;
	GtkListItem *list_item;
	GList *new_filter;

	selection = GTK_LIST (widget)->selection;
	if (selection == NULL)
		return;

	list_item = selection->data;
	new_filter = gtk_object_get_data (GTK_OBJECT (list_item),
					 filter_key);
	g_return_if_fail (new_filter != NULL);

	if (new_filter != fs->current_filter) {
		fs->current_filter = new_filter;
		g_warning ("Populating again");
		start_populating (fs);
	}
#endif
}

/* Notice that we update the combo list only at button press, because
   we are not allowed to change the contents of the list at the
   "selection_changed" signal.  */
static gint
directory_combo_button_press (GtkWidget *widget,
						  GdkEventButton *event,
						  gpointer data)
{
	g_return_val_if_fail (GNOME_IS_FILE_SELECTION (data), FALSE);

	g_warning (__FUNCTION__);

	update_directory_combo_list
		(GNOME_FILE_SELECTION (data));

	return FALSE;
}


/* Dotfiles toggle.  */

static void
toggle_dotfiles_callback (GtkWidget *widget, gpointer data)
{
	GnomeFileSelection *fs;

	g_return_if_fail (GNOME_IS_FILE_SELECTION (data));

	fs = GNOME_FILE_SELECTION (data);

	gnome_file_selection_show_dotfiles
		(fs,
		 gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)));
}


static void
directory_button_callback (GtkWidget *widget,
			   gint row,
			   gint column,
			   GdkEventButton *event,
			   gpointer user_data)
{
	GnomeFileSelection *fs;
	gchar *directory_name;
	GtkCList *clist;

	g_return_if_fail (GTK_IS_CLIST (widget));
	g_return_if_fail (user_data != NULL);

	fs = GNOME_FILE_SELECTION (user_data);
	clist = GTK_CLIST (widget);

	g_return_if_fail (GNOME_IS_FILE_SELECTION (fs));

	gtk_clist_get_text (clist, row, 1, &directory_name);

	/* This is necessary because changing the directory also updates the
	   clist, thus making the pointer invalid.  */
	directory_name = g_strdup (directory_name);

	if (!change_dir (fs, directory_name, TRUE))
		gtk_clist_unselect_all (clist);

	g_free (directory_name);
}


static void
setup_directory_combo_and_toolbar (GnomeFileSelection *fs)
{
	GnomeDialog *dialog;
	GtkWidget *toolbar;
	GtkWidget *hbox;

	dialog = GNOME_DIALOG (fs);

	hbox = gtk_hbox_new (FALSE, 0);

	fs->directory_combo = gtk_combo_new ();

	/* FIXME bugzilla.eazel.com 1123: I cannot get editing to work
	 * nicely, so I just disable it for now.
	 */
	gtk_editable_set_editable (GTK_EDITABLE
				  (GTK_COMBO (fs->directory_combo)->entry),
				  FALSE);

	gtk_widget_show (fs->directory_combo);
	gtk_box_pack_start (GTK_BOX (hbox), fs->directory_combo,
			   TRUE, TRUE, 0);

	fs->priv->combo_selection_changed_id
		= gtk_signal_connect (GTK_OBJECT (GTK_COMBO (fs->directory_combo)->list),
				     "selection_changed",
				     GTK_SIGNAL_FUNC (directory_combo_changed_callback),
				     fs);
	gtk_signal_connect (GTK_OBJECT (GTK_COMBO (fs->directory_combo)->button),
			   "button_press_event",
			   GTK_SIGNAL_FUNC (directory_combo_button_press),
			   fs);

	toolbar = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL,
				  GTK_TOOLBAR_ICONS);
	fs->back_button
		= gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
					  _("Back"),
					  _("Go to the previously visited directory"),
					  NULL,
					  gnome_stock_pixmap_widget
					          (toolbar,
						   GNOME_STOCK_PIXMAP_BACK),
					  toolbar_back_callback, fs);
	fs->up_button
		= gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
					  _("Up"),
					  _("Go to the parent directory"),
					  NULL,
					  gnome_stock_pixmap_widget
					          (toolbar,
						   GNOME_STOCK_PIXMAP_UP),
					  toolbar_up_callback, fs);
	fs->forward_button
		= gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
					  _("Forward"),
					  _("Go to the next visited directory"),
					  NULL,
					  gnome_stock_pixmap_widget
					          (toolbar,
						   GNOME_STOCK_PIXMAP_FORWARD),
					  toolbar_forward_callback, fs);
	gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));
	fs->rescan_button
		= gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
					  _("Rescan"),
					  _("Rescan the current directory"),
					  NULL,
					  gnome_stock_pixmap_widget
					          (toolbar,
						   GNOME_STOCK_PIXMAP_REFRESH),
					  toolbar_refresh_callback, fs);
	gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));
	fs->home_button
		= gtk_toolbar_append_item (GTK_TOOLBAR (toolbar),
					  _("Home"),
					  _("Go to the home directory"),
					  NULL,
					  gnome_stock_pixmap_widget
					          (toolbar,
						   GNOME_STOCK_PIXMAP_HOME),
					  toolbar_home_callback, fs);

	/* Setup the toolbar according to the preferences (sync with
	   gnome-app.c:gnome_app_add_toolbar ()).  Yeah, this sucks.  */
	if (gnome_preferences_get_toolbar_lines ()) {
		gtk_toolbar_set_space_style (GTK_TOOLBAR (toolbar),
					     GTK_TOOLBAR_SPACE_LINE);
		gtk_toolbar_set_space_size (GTK_TOOLBAR (toolbar),
					    GNOME_PAD * 2);
	} else {
		gtk_toolbar_set_space_size (GTK_TOOLBAR (toolbar), GNOME_PAD);
	}

	if (!gnome_preferences_get_toolbar_relief_btn ())
		gtk_toolbar_set_button_relief (GTK_TOOLBAR (toolbar),
					       GTK_RELIEF_NONE);

	gtk_box_pack_start (GTK_BOX (hbox), toolbar, FALSE, FALSE, GNOME_PAD);

	gtk_widget_show (toolbar);

	gtk_widget_show (hbox);

	gtk_box_pack_start (GTK_BOX (dialog->vbox), hbox, FALSE, TRUE, 0);
}

static void
setup_directory_clist (GnomeFileSelection *fs)
{
	GtkWidget *scrolled_window;

	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());

	fs->directory_clist = gtk_clist_new (2);
	gtk_clist_column_titles_hide (GTK_CLIST (fs->directory_clist));
	gtk_clist_set_row_height (GTK_CLIST (fs->directory_clist),
				 DIRECTORY_LIST_ICON_SIZE);
	gtk_clist_set_column_width (GTK_CLIST (fs->directory_clist), 0,
				   DIRECTORY_LIST_ICON_SIZE);
	gtk_clist_set_column_width (GTK_CLIST (fs->directory_clist), 1, 100);
	gtk_signal_connect (GTK_OBJECT (fs->directory_clist), "select_row",
			   (GtkSignalFunc) directory_button_callback, fs);
	scrolled_window = create_scrolled_window ();
	gtk_container_add (GTK_CONTAINER (scrolled_window), fs->directory_clist);
	gtk_paned_add1(GTK_PANED (fs->paned), scrolled_window);

	gtk_widget_show (fs->directory_clist);

	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
}

static void
setup_file_icon_list (GnomeFileSelection *fs)
{
	fs->file_icon_list = gnome_icon_list_new (ICON_LIST_SPACING, NULL, TRUE);
	gnome_icon_list_set_separators (GNOME_ICON_LIST (fs->file_icon_list),
					ICON_LIST_SEPARATORS);
	gnome_icon_list_set_row_spacing (GNOME_ICON_LIST (fs->file_icon_list),
					 ICON_LIST_ROW_SPACING);
	gnome_icon_list_set_col_spacing (GNOME_ICON_LIST (fs->file_icon_list),
					 ICON_LIST_COL_SPACING);
	gnome_icon_list_set_icon_border (GNOME_ICON_LIST (fs->file_icon_list),
					 ICON_LIST_ICON_BORDER);
	gnome_icon_list_set_text_spacing (GNOME_ICON_LIST (fs->file_icon_list),
					  ICON_LIST_TEXT_SPACING);
	gnome_icon_list_set_selection_mode (GNOME_ICON_LIST (fs->file_icon_list),
					    GTK_SELECTION_SINGLE);

	GTK_WIDGET_SET_FLAGS (fs->file_icon_list, GTK_CAN_FOCUS);
	gtk_widget_set_usize (fs->file_icon_list,
			      FILE_LIST_WIDTH, FILE_LIST_HEIGHT);

	gtk_widget_show (fs->file_icon_list);

	gtk_signal_connect (GTK_OBJECT (fs->file_icon_list),
			    "select_icon",
			    GTK_SIGNAL_FUNC (select_icon_callback),
			    fs);
	gtk_signal_connect (GTK_OBJECT (fs->file_icon_list),
			    "unselect_icon",
			    GTK_SIGNAL_FUNC (unselect_icon_callback),
			    fs);
}

static void
setup_file_clist (GnomeFileSelection *fs)
{
	gchar *titles[3];

	titles[0] = _("Name");
	titles[1] = _("Size");
	titles[2] = _("Date");
	fs->file_clist = gtk_clist_new_with_titles (3, titles);
	gtk_clist_set_selection_mode (GTK_CLIST (fs->file_clist),
				     GTK_SELECTION_SINGLE);
	gtk_clist_set_column_width (GTK_CLIST (fs->file_clist), 0, 100);
	gtk_clist_set_column_width (GTK_CLIST (fs->file_clist), 1, 50);
	gtk_clist_set_column_justification (GTK_CLIST (fs->file_clist), 1,
					   GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_width (GTK_CLIST (fs->file_clist), 2, 50);

	gtk_widget_show (fs->file_clist);
}

static void
setup_selection_and_filter (GnomeFileSelection *fs)
{
	GnomeDialog *dialog;
	GtkWidget *table;

	dialog = GNOME_DIALOG (fs);

	table = gtk_table_new (2, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (table), 5);
	gtk_box_pack_end (GTK_BOX (dialog->vbox), table, FALSE, FALSE, 0);
	gtk_widget_show (table);

	/* Selection entry and its text.  */
	{
		fs->selection_text = gtk_label_new (_("Enter name:"));
		gtk_label_set_justify (GTK_LABEL (fs->selection_text), GTK_JUSTIFY_LEFT);
		gtk_table_attach (GTK_TABLE (table), fs->selection_text,
				  0, 1, 0, 1,
				  0,
				  GTK_FILL | GTK_SHRINK,
				  0, 0);
		gtk_widget_show (fs->selection_text);

		fs->selection_entry = gtk_entry_new ();
		gtk_table_attach (GTK_TABLE (table), fs->selection_entry,
				  1, 3, 0, 1,
				  GTK_FILL | GTK_EXPAND | GTK_SHRINK,
				  GTK_FILL | GTK_SHRINK,
				  0, 0);
		gtk_widget_show (fs->selection_entry);
		gnome_dialog_editable_enters (dialog, GTK_EDITABLE (fs->selection_entry));
	}

	/* Filter list.  */
	{
		GtkWidget *label;

		label = gtk_label_new (_("Show:"));
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
		gtk_table_attach (GTK_TABLE (table), label,
				  0, 1, 1, 2,
				  GTK_FILL,
				  GTK_FILL | GTK_SHRINK,
				  0, 0);
		gtk_widget_show (label);

		fs->filter_combo = gtk_combo_new ();
		gtk_table_attach (GTK_TABLE (table), fs->filter_combo,
				  1, 2, 1, 2,
				  GTK_FILL | GTK_EXPAND | GTK_SHRINK,
				  GTK_FILL | GTK_SHRINK,
				  0, 0);
		gtk_editable_set_editable (GTK_EDITABLE
					   (GTK_COMBO (fs->filter_combo)->entry),
					   FALSE);
		gtk_signal_connect (GTK_OBJECT (GTK_COMBO (fs->filter_combo)->list),
				    "selection_changed",
				    GTK_SIGNAL_FUNC
				    (filter_combo_changed_callback),
				    fs);
		gtk_widget_show (fs->filter_combo);
	}

	fs->dotfiles_check_button
		= gtk_check_button_new_with_label (_("Show dotfiles"));
	gtk_table_attach (GTK_TABLE (table), fs->dotfiles_check_button,
			  2, 3, 1, 2,
			  0,
			  GTK_FILL | GTK_SHRINK,
			  0, 0);
	gtk_signal_connect (GTK_OBJECT (fs->dotfiles_check_button), "clicked",
			    GTK_SIGNAL_FUNC (toggle_dotfiles_callback), fs);
	gtk_widget_show (fs->dotfiles_check_button);
}


static GnomeFileSelectionFilter *
filter_new (const gchar *description,
	    const gchar *pattern)
{
	GnomeFileSelectionFilter *filter;

	filter = g_new (GnomeFileSelectionFilter, 1);
	filter->description = g_strdup (description);

#if 0
	filter->pattern = pattern_dup (pattern);
#endif

	return filter;
}

static void
destroy (GtkObject *object)
{
	GnomeFileSelection *fs;

	fs = GNOME_FILE_SELECTION (object);

	g_free (fs->directory);

	while (fs->priv->populating_in_progress) ;
	clean_file_list (fs->priv);

	if (fs->priv->async_handle != NULL)
		gnome_vfs_async_cancel (fs->priv->async_handle);

	g_free (fs->priv);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
map (GtkWidget *widget)
{
	update_filter_combo (GNOME_FILE_SELECTION (widget));

	if (GTK_WIDGET_CLASS (parent_class)->map != NULL)
		GTK_WIDGET_CLASS (parent_class)->map (widget);
}


static void
class_init (GnomeFileSelectionClass *class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = GTK_OBJECT_CLASS (class);
	widget_class = GTK_WIDGET_CLASS (class);

	parent_class = gtk_type_class (gnome_dialog_get_type ());

	object_class->destroy = destroy;

	widget_class->map = map;

	gicon_init ();		/* FIXME bugzilla.eazel.com 1121 */
}

static void
init (GnomeFileSelection *fs)
{
	GnomeDialog *dialog;

	fs->priv = g_new (GnomeFileSelectionPrivate, 1);
	fs->priv->history = gnome_file_selection_history_new (HISTORY_SIZE);

	fs->priv->file_list = NULL;
	fs->filter_list = NULL;
	fs->current_filter = NULL;

	fs->priv->async_handle = NULL;

	/* Dialog.  */
	{
		dialog = GNOME_DIALOG (fs);
		gnome_dialog_append_buttons (dialog,
					    GNOME_STOCK_BUTTON_OK,
					    GNOME_STOCK_BUTTON_CANCEL,
					    NULL);
		gtk_window_set_policy (GTK_WINDOW (fs), TRUE, TRUE, FALSE);

		/* Set the "OK" button as the default one.  */
		gnome_dialog_set_default (dialog, 0);

		/* Directory combo and toolbar.  */
		{
			setup_directory_combo_and_toolbar (fs);
		}

		/* Horizontal paned to hold the directory and file listings.  */
		{
			fs->paned = gtk_hpaned_new ();

			/* FIXME bugzilla.eazel.com 1126: This does not appear to work!  */
			gtk_paned_set_position (GTK_PANED (fs->paned), 100);

			gtk_box_pack_start (GTK_BOX (dialog->vbox), fs->paned, TRUE, TRUE, 0);

			/* Directory listing.  */
			setup_directory_clist (fs);

			/* File list (icons).  */
			setup_file_icon_list (fs);

			/* File list (details).  */
			setup_file_clist (fs);

			/* Scrolled window to hold the file list.  */
			fs->file_scrolled_window = create_scrolled_window ();
			gtk_paned_add2(GTK_PANED (fs->paned), fs->file_scrolled_window);

			gtk_widget_show (fs->paned);
		}

		/* Selection/filter area.  */
		setup_selection_and_filter (fs);
	}

	/* Notice that this must be done *before* setting the list type,
	   because the latter will cause the widget to be populated.
	   FIXME bugzilla.eazel.com 1125: This could be done via `change_dir ()'.  */
	fs->directory = g_get_current_dir ();
	gnome_file_selection_history_add (fs->priv->history, fs->directory);
	update_directory_combo_entry (fs);

	gnome_file_selection_set_list_type (fs,
					    GNOME_FILE_SELECTION_LIST_ICONS);
	gnome_file_selection_show_dotfiles (fs, FALSE);

	update_toolbar (fs);

	start_populating (fs);
}


/* Public functions.  */

GtkType
gnome_file_selection_get_type (void)
{
	static GtkType fs_type = 0;

	if (fs_type == 0) {
		GtkTypeInfo fs_info = {
			"GnomeFileSelection",
			sizeof (GnomeFileSelection),
			sizeof (GnomeFileSelectionClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};

		fs_type = gtk_type_unique (gnome_dialog_get_type (), &fs_info);
	}
	return fs_type;
}

GtkWidget *
gnome_file_selection_new (const gchar *title)
{
	GnomeFileSelection *fs;

	fs = gtk_type_new (gnome_file_selection_get_type ());

	return GTK_WIDGET (fs);
}

void
gnome_file_selection_set_list_type (GnomeFileSelection *fs,
				   GnomeFileSelectionListType type)
{
	fs->list_type = type;

	if (fs->file_clist->parent != NULL)
		gtk_container_remove (GTK_CONTAINER (fs->file_clist->parent),
				     fs->file_clist);
	if (fs->file_icon_list->parent != NULL)
		gtk_container_remove (GTK_CONTAINER (fs->file_icon_list->parent),
				     fs->file_icon_list);

	switch (type) {
	case GNOME_FILE_SELECTION_LIST_ICONS:
	case GNOME_FILE_SELECTION_LIST_ICONS_SMALL:
		gtk_container_add (GTK_CONTAINER (fs->file_scrolled_window),
				  fs->file_icon_list);
		break;
	case GNOME_FILE_SELECTION_LIST_DETAILED:
		gtk_container_add (GTK_CONTAINER (fs->file_scrolled_window),
				  fs->file_clist);
		break;
	default:
		g_error ("Unknown GnomeFileSelectionType %d", type);
		return;
	}

	start_populating (fs);
}

void
gnome_file_selection_show_dotfiles (GnomeFileSelection *fs,
				   gboolean enable)
{
	fs->show_dotfiles = enable;

	start_populating (fs);
}

void
gnome_file_selection_append_filter (GnomeFileSelection *fs,
				   const gchar *description,
				   const gchar *pattern)
{
	fs->filter_list
		= g_list_append (fs->filter_list,
				filter_new (description, pattern));
	if (fs->current_filter == NULL)
		fs->current_filter = fs->filter_list;

	if (GTK_WIDGET_MAPPED (fs))
		update_filter_combo (fs);
}
