#include <config.h>
#include <string.h>
#include "sound-view.h"
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>

static GtkVBoxClass *sound_view_parent_class;

enum {
	CATEGORY,
	EVENT
} SoundViewRowType;

struct _SoundViewPrivate
{
	GtkTreeModel *model;
	GtkTreeModel *smodel;
	GtkTreeView *view;
	GtkTreeSelection *selection;
	SoundProperties *props;
	GtkWidget *play_button;
	GtkWidget *entry;
	GHashTable *event2path;
	GSList *paths;
	gboolean ignore_changed;
};

enum {
	EVENT_COLUMN,
	FILE_COLUMN,
	SORT_DATA_COLUMN,
	TYPE_COLUMN,
	DATA_COLUMN,
	NUM_COLUMNS
};

static void sound_view_reload (SoundView *view);

static void select_row_cb (GtkTreeSelection *sel, SoundView *view);

static void
sound_view_destroy (GtkObject *object)
{
	SoundView *view = SOUND_VIEW (object);

	if (view->priv != NULL) {
		while (view->priv->paths) {
			gtk_tree_path_free ((GtkTreePath *) view->priv->paths->data);
			view->priv->paths = g_slist_remove (view->priv->paths, view->priv->paths->data);
		}
		if (view->priv->event2path != NULL) {
			g_hash_table_destroy (view->priv->event2path);
			view->priv->event2path = NULL;
		}
		g_free (view->priv);
		view->priv = NULL;
	}

	if (GTK_OBJECT_CLASS (sound_view_parent_class)->destroy)
		GTK_OBJECT_CLASS (sound_view_parent_class)->destroy (object);
}

static void
sound_view_class_init (GtkObjectClass *object_class)
{
	sound_view_parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->destroy = sound_view_destroy;
}

static gint 
compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data)
{
	GValue val_a = {0};
	GValue val_b = {0};
	const gchar *str_a, *str_b;

	gtk_tree_model_get_value (model, a, SORT_DATA_COLUMN, &val_a);
	str_a = g_value_get_string (&val_a);
	gtk_tree_model_get_value (model, b, SORT_DATA_COLUMN, &val_b);
	str_b = g_value_get_string (&val_b);

	if (!str_a)
		return 1;
	else if (!str_b)
		return -1;
	else
		return g_ascii_strcasecmp (str_a, str_b);
}

static void
play_cb (GtkButton *button, SoundView *view)
{
	GtkTreeIter iter;
	GValue value = {0};
	SoundEvent *event;
	gchar *filename;

	if (!gtk_tree_selection_get_selected (view->priv->selection, NULL, &iter))
		return;
	
	gtk_tree_model_get_value (view->priv->smodel, &iter, DATA_COLUMN, &value);
	event = g_value_get_pointer (&value);

	g_assert (event->file);
	if (!event->file)
		return;

	if ((event->file[0] == '/') || g_file_test (event->file, G_FILE_TEST_EXISTS))
		filename = g_strdup (event->file);
	else
		filename = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_SOUND, event->file, TRUE, NULL);
	
	if (filename && g_file_test (filename, G_FILE_TEST_EXISTS))
		gnome_sound_play (filename);
	else
	{
		GtkWidget *md;
		gchar *msg;

		if (filename && filename[0] == '/')
			msg = _("The sound file for this event does not exist.");
		else
			msg = _("The sound file for this event does not exist.\n"
				"You may want to install the gnome-audio package\n"
				"for a set of default sounds.");
		md = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, msg);
		gtk_dialog_run (GTK_DIALOG (md));
		gtk_widget_destroy (md);
	}

	g_free (filename);
}

static void
entry_changed_cb (GtkEditable *entry, SoundView *view)
{
	const gchar *file;
	GtkTreeIter iter;
	GValue tval = {0};
	GValue eval = {0};
	SoundEvent *event;
	
	if (view->priv->ignore_changed)
		return;

	if (!gtk_tree_selection_get_selected (view->priv->selection, NULL, &iter))
		return;

	gtk_tree_model_get_value (view->priv->smodel, &iter, TYPE_COLUMN, &tval);
	if (g_value_get_uint (&tval) != EVENT) return;

	gtk_tree_model_get_value (view->priv->smodel, &iter, DATA_COLUMN, &eval);
	event = g_value_get_pointer (&eval);

	file = gtk_entry_get_text (GTK_ENTRY (entry));
	sound_event_set_file (event, (gchar *) file);
	sound_properties_event_changed (view->priv->props, event);
}

static void
event_changed_cb (SoundProperties *props, SoundEvent *event, SoundView *view)
{
	GtkTreePath *path;

	path = g_hash_table_lookup (view->priv->event2path, event);

	if (path) {
		GtkTreeIter iter;
		if (gtk_tree_model_get_iter (view->priv->model, &iter, path)) {
			GValue val = {0};
			gtk_tree_model_get_value (view->priv->model, &iter, DATA_COLUMN, &val);
			gtk_tree_store_set (GTK_TREE_STORE (view->priv->model), &iter,
					    FILE_COLUMN, event->file,
					    -1);
		}
	}
}

static void
sound_view_init (GtkObject *object)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	SoundView *view = SOUND_VIEW (object);
	GtkWidget *widget, *hbox, *bbox;
	GtkWidget *vbox;
	GtkWidget *frame, *label;
	gchar *path;

	view->priv = g_new0 (SoundViewPrivate, 1);

	view->priv->event2path = g_hash_table_new (g_direct_hash, g_direct_equal);
	
	view->priv->model = (GtkTreeModel *) gtk_tree_store_new (NUM_COLUMNS,
								 G_TYPE_STRING,
								 G_TYPE_STRING,
								 G_TYPE_STRING,
								 G_TYPE_UINT,
								 G_TYPE_POINTER);
	view->priv->smodel = gtk_tree_model_sort_new_with_model (view->priv->model);
	view->priv->view = (GtkTreeView *) gtk_tree_view_new_with_model (view->priv->smodel);
	view->priv->selection = gtk_tree_view_get_selection (view->priv->view);
	gtk_tree_selection_set_mode (view->priv->selection, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (view->priv->selection), "changed",
			  (GCallback) select_row_cb, view);

#if 1
	gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (view->priv->smodel),
					SORT_DATA_COLUMN, (GtkTreeIterCompareFunc) compare_func, NULL, NULL);
#endif
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (view->priv->smodel),
					      SORT_DATA_COLUMN, GTK_SORT_ASCENDING);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Event"),
							   renderer,
							   "text", EVENT_COLUMN,
							   NULL);
	gtk_tree_view_column_set_clickable (column, FALSE);
	gtk_tree_view_append_column (view->priv->view, column);

	column = gtk_tree_view_column_new_with_attributes (_("File to play"),
							   renderer,
							   "text", FILE_COLUMN,
							   NULL);
	gtk_tree_view_column_set_clickable (column, FALSE);
	gtk_tree_view_append_column (view->priv->view, column);

	widget = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget),
					GTK_POLICY_ALWAYS,
					GTK_POLICY_ALWAYS);
	gtk_widget_set_size_request (widget, 350, -1);
	gtk_container_add (GTK_CONTAINER (widget), GTK_WIDGET (view->priv->view));

	frame = gtk_frame_new ("");
	label = gtk_label_new_with_mnemonic (_("_Sounds"));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label),
				       GTK_WIDGET (view->priv->view));
	gtk_frame_set_label_widget (GTK_FRAME (frame), label);
	gtk_container_add (GTK_CONTAINER (frame), widget);

	gtk_box_pack_start (GTK_BOX (view), frame, TRUE, TRUE, 0);
	gtk_widget_show_all (frame);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (view), hbox, FALSE, FALSE, GNOME_PAD);
	view->priv->play_button = gtk_button_new ();
	bbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (bbox), gtk_image_new_from_stock (GNOME_STOCK_VOLUME, GTK_ICON_SIZE_BUTTON), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (bbox), gtk_label_new_with_mnemonic (_("_Play")), FALSE, FALSE, GNOME_PAD);
	gtk_container_add (GTK_CONTAINER (view->priv->play_button), bbox);
	gtk_box_pack_start (GTK_BOX (hbox), view->priv->play_button, TRUE, FALSE, GNOME_PAD_SMALL);
	g_object_set_data (G_OBJECT (view->priv->play_button), "gnome_disable_sound_events", GINT_TO_POINTER(1));
	g_signal_connect (G_OBJECT (view->priv->play_button), "clicked", (GCallback) play_cb, view);

	view->priv->entry = gnome_file_entry_new (NULL, _("Select sound file"));
	gnome_file_entry_set_modal (GNOME_FILE_ENTRY (view->priv->entry), TRUE);
	path = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_DATADIR, "sounds/", TRUE, NULL);
	gnome_file_entry_set_default_path (GNOME_FILE_ENTRY (view->priv->entry), path);
	g_free (path);

	g_signal_connect (G_OBJECT (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (view->priv->entry))), "changed",
			  (GCallback) entry_changed_cb, view);
				       
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), view->priv->entry, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, FALSE, GNOME_PAD_SMALL);


	gtk_widget_set_sensitive (view->priv->play_button, FALSE);
	gtk_widget_set_sensitive (view->priv->entry, FALSE);
	
	gtk_widget_show_all (hbox);
}

GtkType
sound_view_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		GTypeInfo info =
		{
			sizeof (SoundViewClass),
			NULL, NULL,
			(GClassInitFunc) sound_view_class_init,
			NULL, NULL,
			sizeof (SoundView),
			0,
			(GInstanceInitFunc) sound_view_init
		};
		
		type = g_type_register_static (gtk_vbox_get_type (), "SoundView", &info, 0);
	}

	return type;
}

GtkWidget*
sound_view_new (SoundProperties *props)
{
	SoundView *view = g_object_new (sound_view_get_type (), NULL);
	view->priv->props = props;
	sound_view_reload (view);

	gtk_tree_view_expand_all (view->priv->view);

	g_signal_connect (GTK_OBJECT (props), "event_changed",
			  (GCallback) event_changed_cb, view);

	return GTK_WIDGET (view);
}

static void
foreach_cb (gchar *category, gchar *desc, GList *events, SoundView *view)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	GList *l;

	/* No events in this category... */
	if (!events)
		return;

	gtk_tree_store_append (GTK_TREE_STORE (view->priv->model), &iter, NULL);
	/* Force the gnome-2 items to the begining of the list by lying about
	 * its description in the sort key
	 * 
	 * http://bugzilla.gnome.org/show_bug.cgi?id=76437
	 * disable this until we know why this doesn't work.
	 * I do not feel like wasting time figuring it out
	 */
	gtk_tree_store_set (GTK_TREE_STORE (view->priv->model), &iter,
			    EVENT_COLUMN, desc,
			    FILE_COLUMN, NULL,
			    SORT_DATA_COLUMN, "Why the hell doesn't this work", /* (!g_ascii_strcasecmp (category, "gnome-2")) ? " " : desc, */
			    TYPE_COLUMN, (guint) CATEGORY,
			    DATA_COLUMN, category,
			    -1);

	for (l = events; l != NULL; l = l->next)
	{
		GtkTreeIter child_iter;
		SoundEvent *event = l->data;

		gtk_tree_store_append (GTK_TREE_STORE (view->priv->model), &child_iter, &iter);
		gtk_tree_store_set (GTK_TREE_STORE (view->priv->model), &child_iter,
				    EVENT_COLUMN, event->desc,
				    FILE_COLUMN, event->file,
				    SORT_DATA_COLUMN, event->desc,
				    TYPE_COLUMN, (guint) EVENT,
				    DATA_COLUMN, event,
				    -1);
		/* fixme: I do not like this - does anybody know better way? */
		path = gtk_tree_model_get_path (view->priv->model, &child_iter);
		g_hash_table_insert (view->priv->event2path, event, path);
		view->priv->paths = g_slist_prepend (view->priv->paths, path);
	}
}

static void
sound_view_reload (SoundView *view)
{
	g_return_if_fail (SOUND_IS_VIEW (view));
	
	gtk_tree_store_clear (GTK_TREE_STORE (view->priv->model));

	sound_properties_category_foreach (view->priv->props,
					   (SoundPropertiesCategoryForeachFunc) foreach_cb, view);
}

static void
select_row_cb (GtkTreeSelection *sel, SoundView *view)
{
	GtkTreeIter iter;
	GValue value = {0};
	guint type;
	gchar *text;

	if (!gtk_tree_selection_get_selected (sel, NULL, &iter))
		return;

	gtk_tree_model_get_value (view->priv->smodel, &iter, TYPE_COLUMN, &value);
	type = g_value_get_uint (&value);

	if (type == EVENT)
	{
		GValue eval = {0};
		SoundEvent *event;

		gtk_tree_model_get_value (view->priv->smodel, &iter, DATA_COLUMN, &eval);
		event = g_value_get_pointer (&eval);
		text = event->file;

		if (text && text[0] == '/')
		{
			gchar *dirname;
			dirname = g_path_get_dirname (text);
			gnome_file_entry_set_default_path (GNOME_FILE_ENTRY (view->priv->entry), dirname);
			g_free (dirname);
			text = g_path_get_basename (text);
		} else {
			text = g_strdup (text);
		}

		gtk_widget_set_sensitive (view->priv->play_button, TRUE);
		gtk_widget_set_sensitive (view->priv->entry, TRUE);
	} else {
		text = g_strdup ("");
		gtk_widget_set_sensitive (view->priv->play_button, FALSE);
		gtk_widget_set_sensitive (view->priv->entry, FALSE);
	}

	view->priv->ignore_changed = TRUE;
	gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (view->priv->entry))), text);
	view->priv->ignore_changed = FALSE;
	g_free (text);
}
