/* bug-buddy bug submitting program
 *
 * Copyright (C) 2001 Jacob Berkman
 * Copyright 2001 Ximian, Inc.
 *
 * Author:  jacob berkman  <jacob@bug-buddy.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include "bug-buddy.h"
#include "libglade-buddy.h"
#include "cell-renderer-uri.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <gnome.h>

#include <libgnomevfs/gnome-vfs.h>

#include <dirent.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

/* define to x for some debugging output */
#define d(x)

static int
prod_cmp (BugzillaProduct *a, BugzillaProduct *b)
{
	return strcasecmp (a->name, b->name);
}

static int
comp_cmp (BugzillaComponent *a, BugzillaComponent *b)
{
	return strcasecmp (a->name, b->name);
}

static void
bugzilla_bts_insert_product (BugzillaBTS *bts, BugzillaProduct *prod)
{
	bts->products = g_slist_insert_sorted (bts->products, prod, (gpointer)prod_cmp);
}

static void
bugzilla_product_insert_component (BugzillaProduct *prod, BugzillaComponent *comp)
{
	prod->components = g_slist_insert_sorted (prod->components, comp, (gpointer)comp_cmp);
}

/* i think the bugzilla files are ISO8859-1 */
/* they aren't anymore, and my iconv on solaris isn't doing UTF-8 for
 * some reason */
static char *
gify (char *x)
{
	char *g;
	g = g_convert (x, strlen (x), "UTF-8", "ISO8859-1", NULL, NULL, NULL);
	if (!g)
		g = g_strdup (x);
	xmlFree (x);
	return g;
}
	
#define XML_NODE_GET_PROP(node, name) (gify (xmlGetProp ((node), (name))))
#define XML_NODE_GET_CONTENT(node)    (gify (xmlNodeGetContent ((node))))

static void
load_mostfreq_xml (BugzillaBTS *bts, xmlDoc *doc)
{
	xmlNode *prod, *comp, *bug, *node;
	BugzillaBug *bbug;

	d(g_print ("mostfreq:\n"));
	/* FIXME: free these */
	bts->bugs = NULL;

	for (prod = xmlDocGetRootElement (doc)->children; prod; prod = prod->next) {
		d(g_print ("\t%s\n", prod->name));

		if (strcmp (prod->name, "product"))
			continue;

		for (comp = prod->children; comp; comp = comp->next) {
			d(g_print ("\t\t%s\n", comp->name));
			if (strcmp (comp->name, "component"))
				continue;

			for (bug = comp->children; bug; bug = bug->next) {
				d(g_print ("\t\t\t%s\n", bug->name));
				if (strcmp (bug->name, "bug"))
					continue;
				
				bbug = g_new0 (BugzillaBug, 1);
		
				bbug->product   = XML_NODE_GET_PROP (prod, "name");
				bbug->component = XML_NODE_GET_PROP (comp, "name");
				bbug->id        = XML_NODE_GET_PROP (bug, "bugid");

				for (node = bug->children; node; node = node->next) {
					d(g_print ("\t\t\t\t%s\n", node->name));

					if (!strcmp (node->name, "desc"))
						bbug->desc = XML_NODE_GET_CONTENT (node);
					else if (!strcmp (node->name, "url"))
						bbug->url = XML_NODE_GET_CONTENT (node);
				}
				bts->bugs = g_slist_prepend (bts->bugs, bbug);
			}
		}
	}

	xmlFreeDoc (doc);
}

static void
load_config_xml (BugzillaBTS *bts, xmlDoc *doc)
{
	xmlNode *node, *cur;

	d(g_print ("config:\n"));

	/* FIXME: free these */
	bts->opsys = NULL;
	bts->severities = NULL;

	for (node = xmlDocGetRootElement (doc)->children; node; node = node->next) {
		d(g_print ("\t%s\n", node->name));
		if (!strcmp (node->name, bts->severity_node)) {
			for (cur = node->children; cur; cur = cur->next) {
				d(g_print ("\t\t%s\n", cur->name));
				if (strcmp (cur->name, bts->severity_item))
					continue;
				bts->severities = g_slist_append (
					bts->severities,
					XML_NODE_GET_CONTENT (cur));
			}
		} else if (!strcmp (node->name, "opsys_list")) {
			for (cur = node->children; cur; cur = cur->next) {
				d(g_print ("\t\t%s\n", cur->name));
				if (strcmp (cur->name, "opsys"))
					continue;
				bts->opsys = g_slist_append (
					bts->opsys,
					XML_NODE_GET_CONTENT (cur));
			}
		}
	}

	xmlFreeDoc (doc);
}

static void
load_products_xml (BugzillaBTS *bts, xmlDoc *doc)
{
	BugzillaProduct *prod;
	BugzillaComponent *comp;
	xmlNode *node, *cur;

	d(g_print ("products:\n"));

	/* FIXME: free list */
	bts->products = NULL;

	for (node = xmlDocGetRootElement (doc)->children; node; node = node->next) {
		d(g_print ("\t%s\n", node->name));
		if (!strcmp (node->name, "product")) {
			prod = g_new0 (BugzillaProduct, 1);
			prod->bts = bts;

			prod->name        = XML_NODE_GET_PROP (node, "name");
			prod->description = XML_NODE_GET_PROP (node, "description");

			bugzilla_bts_insert_product (bts, prod);
			bugzilla_bts_insert_product (druid_data.all_products, prod);

			for (cur = node->children; cur; cur = cur->next) {
				d(g_print ("\t\t%s\n", node->name));
				if (strcmp (cur->name, "component"))
					continue;
				
				comp = g_new0 (BugzillaComponent, 1);
				comp->product = prod;
				
				comp->name        = XML_NODE_GET_PROP (cur, "value");
				comp->description = XML_NODE_GET_PROP (cur, "description");

				bugzilla_product_insert_component (prod, comp);
			}
		}
	}

	xmlFreeDoc (doc);
}

#if 0
static void
print_comp (gpointer data, gpointer user_data)
{
	BugzillaComponent *comp = data;

	g_print ("\t\t%s (%s)\n", comp->name, comp->description);
}

static void
print_product (gpointer data, gpointer user_data)
{
	BugzillaProduct *prod = data;

	g_print ("\t%s (%s)\n", prod->name, prod->description);

	g_slist_foreach (prod->components, print_comp, NULL);
}

static void
print_item (gpointer data, gpointer user_data)
{
	g_print ("\t%s\n", (char *)data);
}
#endif

#if 0
static void
goto_product_page (void)
{
	druid_set_sensitive (TRUE, TRUE, TRUE);
	druid_set_state (STATE_PRODUCT);
}
#endif

static int
async_update (GnomeVFSAsyncHandle *handle, GnomeVFSXferProgressInfo *info, gpointer data)
{
	d(g_print ("%" GNOME_VFS_SIZE_FORMAT_STR "\n", info->bytes_copied));

	if (info->status == GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR) {
		end_bugzilla_download (END_BUGZILLA_NOOP);
		g_message ("there was an error........");
		return GNOME_VFS_XFER_ERROR_ACTION_ABORT;
	}

	if (info->source_name) {
		d(g_print ("source: %s\n", info->source_name));
		buddy_set_text ("progress-source", info->source_name);
	}

	if (info->target_name) {
		d(g_print ("target: %s\n", info->target_name));
		buddy_set_text ("progress-dest", info->target_name);
	}

	if (info->bytes_total)
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (GET_WIDGET ("progress-progress")), 
					       (gfloat)info->total_bytes_copied / info->bytes_total);

	if (info->phase == GNOME_VFS_XFER_PHASE_COMPLETED) {
		end_bugzilla_download (END_BUGZILLA_HIDE_BOX);
		load_bugzilla_xml ();
	}

	return druid_data.download_in_progress;
}

/**
 * e_mkdir_hier:
 * @path: a directory path
 * @mode: a mode, as for mkdir(2)
 *
 * This creates the named directory with the given @mode, creating
 * any necessary intermediate directories (with the same @mode).
 *
 * Return value: 0 on success, -1 on error, in which case errno will
 * be set as for mkdir(2).
 **/
static int
e_mkdir_hier(const char *path, mode_t mode)
{
        char *copy, *p;

        p = copy = g_strdup (path);
        do {
                p = strchr (p + 1, '/');
                if (p)
                        *p = '\0';
                if (access (copy, F_OK) == -1) {
                        if (mkdir (copy, mode) == -1) {
                                g_free (copy);
                                return -1;
                        }
                }
                if (p)
                        *p = '/';
        } while (p);

        g_free (copy);
        return 0;
}

static BugzillaXMLFile *
get_xml_file (BugzillaBTS *bts, const char *key, XMLFunc parse_func)
{
	BugzillaXMLFile *xmlfile;
	char *localdir, *tmppath, *src_uri, *tmp_key;

	src_uri = gnome_config_get_string (key);
	if (!src_uri) {
		d(g_warning ("could not read: %s\n", key));
		return NULL;
	}

	xmlfile = g_new0 (BugzillaXMLFile, 1);
	xmlfile->xml_func = parse_func;

	tmppath = gnome_util_home_file ("bug-buddy.d");
	tmp_key = g_strconcat (key, ".tmp", NULL);

	xmlfile->system_path = g_build_filename (BUDDY_DATADIR, "bugzilla", bts->subdir, key, NULL);
	xmlfile->cache_path  = g_build_filename (tmppath,       "bugzilla", bts->subdir, key, NULL);
	xmlfile->tmp_path    = g_build_filename (tmppath,       "bugzilla", bts->subdir, tmp_key, NULL);
	localdir             = g_build_filename (tmppath,       "bugzilla", bts->subdir, NULL);

	g_free (tmppath);
	
	if (e_mkdir_hier (localdir, S_IRWXU)) {
		d(g_warning ("could not create local dir: `%s': %s\n", localdir, g_strerror (errno)));
	}
	
	g_free (localdir);

	d(g_print ("wanting to save: %s\nto %s\n", src_uri, xmlfile->cache_path));

	xmlfile->source_uri = gnome_vfs_uri_new (src_uri);
	g_free (src_uri);

	if (!xmlfile->source_uri) {
		d(g_error ("could not parse source"));
		return xmlfile;
	}
		
	xmlfile->dest_uri = gnome_vfs_uri_new (xmlfile->tmp_path);
	if (!xmlfile->dest_uri) {
		gnome_vfs_uri_unref (xmlfile->source_uri);
		xmlfile->source_uri = NULL;
		d(g_error ("could not parse dest"));
		return xmlfile;
	}

	druid_data.dlsources = g_list_prepend (druid_data.dlsources, xmlfile->source_uri);
	druid_data.dldests   = g_list_prepend (druid_data.dldests,   xmlfile->dest_uri);

	xmlfile->download = xmlfile->read_from_cache = TRUE;
	return xmlfile;
}

static BugzillaBTS *
load_bugzilla (const char *filename)
{
	BugzillaBTS *bts;
	char *path, *pixmap;
	gboolean def;
	char *s;
	GdkPixbuf *pb;
	
	d(g_print ("loading `%s'...\n", filename));

	bts = g_new0 (BugzillaBTS, 1);

	path = g_strdup_printf ("=%s=/Bugzilla/", filename);

	gnome_config_push_prefix (path);

	bts->name = gnome_config_get_string ("name");

	if (!bts->name) {
		g_free (bts);
		return NULL;
	}

	bts->subdir = gnome_config_get_string ("subdir");
	if (!bts->subdir)
		bts->subdir = g_strdup (bts->name);

	pixmap = gnome_config_get_string_with_default ("icon="BUDDY_ICONDIR"/bug-buddy.png", &def);

	if (pixmap[0] == '/')
		bts->icon = g_strdup (pixmap);
	else
		bts->icon = g_build_filename (BUDDY_DATADIR, pixmap, NULL);
	g_free (pixmap);

	s = gnome_config_get_string ("submit_type=freitag");
	if (!strcasecmp (s, "freitag"))
		bts->submit_type = BUGZILLA_SUBMIT_FREITAG;
	else if (!strcasecmp (s, "debian"))
		bts->submit_type = BUGZILLA_SUBMIT_DEBIAN;
	g_free (s);
	
	d(g_print ("icon: %s\n", bts->icon));

	bts->email = gnome_config_get_string ("email");

	pb = gdk_pixbuf_new_from_file (bts->icon, NULL);
	if (pb) {
		bts->pixbuf = gdk_pixbuf_scale_simple (pb, 20, 20, GDK_INTERP_HYPER);
		g_object_unref (pb);
	}

	bts->severity_node   = gnome_config_get_string ("severity_node=severities");
	bts->severity_item   = gnome_config_get_string ("severity_item=severity");
	bts->severity_header = gnome_config_get_string ("severity_header=Severity");

	bts->products_xml = get_xml_file (bts, "products", load_products_xml);
	bts->config_xml   = get_xml_file (bts, "config",   load_config_xml);
	bts->mostfreq_xml = get_xml_file (bts, "mostfreq", load_mostfreq_xml);

	gnome_config_pop_prefix ();

	if ((bts->products_xml && bts->products_xml->download) ||
	    (bts->config_xml   && bts->config_xml->download))
		druid_data.need_to_download = TRUE;

	return bts;
}

#if 0
void
on_progress_cancel_clicked (GtkWidget *w, gpointer data)
{
	d(g_print ("clicked!!\n"));
	gnome_vfs_async_cancel (druid_data.vfshandle);
	d(g_print ("shaggy?\n"));
	goto_product_page ();
	d(g_print ("scooby dooby doo!\n"));
}
#endif

static void
show_products (GtkWidget *w, gpointer data)
{
	bugzilla_bts_add_products_to_clist ((BugzillaBTS *)data);
}

static xmlDoc *
load_bugzilla_xml_file (BugzillaXMLFile *xml_file)
{
	xmlDoc *doc = NULL;
	struct stat statbuf;

	if (!stat (xml_file->tmp_path, &statbuf) && statbuf.st_size) {
		if (!rename (xml_file->tmp_path, xml_file->cache_path)) {
			doc = xmlParseFile (xml_file->cache_path);
		}
	}


	if (!doc)
		doc = xmlParseFile (xml_file->system_path);

	return doc;
}

void
load_bugzilla_xml (void)
{
	static gboolean loaded;
	xmlDoc *doc;
	BugzillaBTS *bts;
	GSList *item;
	GtkWidget *m;
	GtkWidget *w;

	if (loaded) return;
	loaded = TRUE;

	d(g_print ("loading xml..\n"));

	m = gtk_menu_new ();

	for (item = druid_data.bugzillas; item; item = item->next) {
		bts = (BugzillaBTS *)item->data;
		if (bts->products_xml) { // && !bts->products_xml->done) {
			doc = load_bugzilla_xml_file (bts->products_xml);
			if (doc)
				load_products_xml (bts, doc);
			bts->products_xml->done = TRUE;
		}

		if (bts->config_xml) { // && !bts->config_xml->done) {
			doc = load_bugzilla_xml_file (bts->config_xml);
			if (doc) 
				load_config_xml (bts, doc);
			bts->config_xml->done = TRUE;
		}

		if (bts->mostfreq_xml) { // && !bts->mostfreq_xml->done) {
			doc = load_bugzilla_xml_file (bts->mostfreq_xml);
			if (doc)
				load_mostfreq_xml (bts, doc);
			bts->mostfreq_xml->done = TRUE;
		}

		w = gtk_menu_item_new_with_label (bts->name);
		g_signal_connect (G_OBJECT (w), "activate",
				  G_CALLBACK (show_products),
				  bts);
		gtk_widget_show (w);
		gtk_menu_shell_append (GTK_MENU_SHELL (m), w);
	}
	w = GET_WIDGET ("bts-menu");
	gtk_option_menu_set_menu (GTK_OPTION_MENU (w), m);
	gtk_option_menu_set_history (GTK_OPTION_MENU (w), 0);
	bugzilla_bts_add_products_to_clist (druid_data.all_products);	
}

static void
p_string (GnomeVFSURI *uri, gpointer data)
{
	char *s;
	
	s = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	g_print ("\t%s\n", s);
	g_free (s);

	return;

	/* reference ourselves to make gcc quiet */
	p_string (NULL, NULL);
}

static void
uri_visited (CellRendererUri *uri, const char *path, GtkListStore *model)
{
	GtkTreeIter iter;

	if (!gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path))
		return;

	gtk_list_store_set (model, &iter, 
			    MOSTFREQ_SHOWN, TRUE,
			    -1);
}

static gboolean
mostfreq_motion (GtkWidget *w, GdkEventMotion *event, gpointer data)
{
	int tx, ty;
	GtkTreeViewColumn *col;
	GdkCursor *cursor;
	gboolean showing_hand, inside;

	gtk_tree_view_widget_to_tree_coords (GTK_TREE_VIEW (w), 
					     event->x, event->y,
					     &tx, &ty);
	
	inside = gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (w),
						tx, ty,
						NULL, &col,
						NULL, NULL);

	showing_hand = inside &&
		(col == gtk_tree_view_get_column (GTK_TREE_VIEW (w), 0));

	if (showing_hand == druid_data.showing_hand)
		return FALSE;

	druid_data.showing_hand = showing_hand;
	cursor = gdk_cursor_new (showing_hand
				 ? GDK_HAND2
				 : GDK_LEFT_PTR);
	gdk_window_set_cursor (w->window, cursor);
	gdk_cursor_unref (cursor);

	return FALSE;
}

static void
create_mostfreq_list (void)
{
	GtkListStore *model;
	GtkTreeView *view;
	GtkCellRenderer *ren;

	view = GTK_TREE_VIEW (GET_WIDGET ("mostfreq-list"));

	model = gtk_list_store_new (MOSTFREQ_COLS, 
				    G_TYPE_STRING, G_TYPE_STRING, 
				    G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_STRING, G_TYPE_BOOLEAN);

	g_object_set (G_OBJECT (view), "model", model, NULL);
	g_object_unref (G_OBJECT (model));

	ren = g_object_new (TYPE_CELL_RENDERER_URI, NULL);
	gtk_tree_view_insert_column_with_attributes (view, -1,
						     _("ID"), ren,
						     "text", MOSTFREQ_ID,
						     "uri", MOSTFREQ_URL,
						     "visited", MOSTFREQ_SHOWN,
						     NULL);
	
	g_signal_connect (ren, "uri_visited", G_CALLBACK (uri_visited), model);
	g_signal_connect (view, "motion-notify-event",
			  G_CALLBACK (mostfreq_motion),
			  ren);


	ren = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (view, -1,
						     _("Product"), ren,
						     "text", MOSTFREQ_PRODUCT,
						     NULL);

	/*ren = gtk_cell_renderer_text_new ();*/
	gtk_tree_view_insert_column_with_attributes (view, -1,
						     _("Component"), ren,
						     "text", MOSTFREQ_COMPONENT,
						     NULL);

	/*ren = gtk_cell_renderer_text_new ();*/
	gtk_tree_view_insert_column_with_attributes (view, -1,
						     _("Description"), ren,
						     "text", MOSTFREQ_DESC,
						     NULL);
}

static void
create_products_list (void)
{
	GtkListStore *model;
	GtkTreeView *view;
	GtkCellRenderer *ren;
	GtkTreeViewColumn *col;

	view = GTK_TREE_VIEW (GET_WIDGET ("product-list"));

	model = gtk_list_store_new (PRODUCT_COLS, GDK_TYPE_PIXBUF, 
				    G_TYPE_STRING, G_TYPE_STRING,
				    G_TYPE_POINTER);

	g_object_set (G_OBJECT (view), "model", model, NULL);
	g_object_unref (G_OBJECT (model));
	
	col = g_object_new (GTK_TYPE_TREE_VIEW_COLUMN,
			    "title", _("Product"),
			    NULL);
	gtk_tree_view_append_column (view, col);
	ren = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (col, ren, FALSE);


	gtk_tree_view_column_set_attributes (col, ren,
					     "pixbuf", PRODUCT_ICON,
					     NULL);

	ren = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (col, ren, TRUE);
	gtk_tree_view_column_set_attributes (col, ren,
					     "text", PRODUCT_NAME,
					     NULL);

	gtk_tree_view_insert_column_with_attributes (view, -1,
						     _("Description"), ren,
						     "text", PRODUCT_DESC,
						     NULL);
}

static void
create_components_list (void)
{
	GtkListStore *model;
	GtkTreeView *view;
	GtkCellRenderer *ren;

	view = GTK_TREE_VIEW (GET_WIDGET ("component-list"));

	model = gtk_list_store_new (COMPONENT_COLS, G_TYPE_STRING, 
				    G_TYPE_STRING, G_TYPE_POINTER);
	
	g_object_set (G_OBJECT (view), "model", model, NULL);
	g_object_unref (G_OBJECT (model));

	ren = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (view, -1,
						     _("Component"), ren,
						     "text", COMPONENT_NAME,
						     NULL);

	ren = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (view, -1,
						     _("Description"), ren,
						     "text", COMPONENT_DESC,
						     NULL);
}

void
end_bugzilla_download (EndBugzillaFlags flags)
{
	gboolean cancel   = flags & END_BUGZILLA_CANCEL;
	gboolean hide_box = flags & END_BUGZILLA_HIDE_BOX;

	if (druid_data.download_in_progress && cancel)
		gnome_vfs_async_cancel (druid_data.vfshandle);

	if (hide_box) {
		gtk_widget_hide (GET_WIDGET ("progress-box"));
		gtk_widget_hide (GET_WIDGET ("progress-sep"));
	} else {
		gtk_widget_set_sensitive (GET_WIDGET ("progress-start"), TRUE);
		gtk_widget_set_sensitive (GET_WIDGET ("progress-stop"),  FALSE);
	}

	druid_data.download_in_progress = FALSE;
}

gboolean
start_bugzilla_download (void)
{
	end_bugzilla_download (END_BUGZILLA_CANCEL);

	gtk_widget_show (GET_WIDGET ("progress-box"));
	gtk_widget_show (GET_WIDGET ("progress-sep"));
	gtk_widget_set_sensitive (GET_WIDGET ("progress-start"), FALSE);
	gtk_widget_set_sensitive (GET_WIDGET ("progress-stop"), TRUE);
	druid_data.download_in_progress = TRUE;
	gnome_vfs_async_xfer (	    
		&druid_data.vfshandle,
		druid_data.dlsources,
		druid_data.dldests,
		GNOME_VFS_XFER_DEFAULT,
		GNOME_VFS_XFER_ERROR_MODE_QUERY,
		GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE,
		GNOME_VFS_PRIORITY_DEFAULT,
		async_update, NULL, NULL, NULL);
	druid_data.dl_timeout = 0;
	return FALSE;
}

void
load_bugzillas (void)
{
	BugzillaBTS *bts;
	GtkWidget *w;
	DIR *dir;
	struct dirent *dent;
	char *p;

	dir = opendir (BUDDY_DATADIR"/bugzilla/");
	if (!dir) {
		w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("Bug Buddy could not open '%s'.\n"
					      "Please make sure Bug Buddy was "
					      "installed correctly.\n\n"
					      "Bug Buddy will now quit."),
					    BUDDY_DATADIR "/bugzilla/");
		gtk_dialog_set_default_response (GTK_DIALOG (w),
						 GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		exit (0);
	}

	create_products_list ();

	create_components_list ();

	create_mostfreq_list ();

	druid_data.all_products = g_new0 (BugzillaBTS, 1);
	druid_data.all_products->name = _("All");

	druid_data.bugzillas = g_slist_append (druid_data.bugzillas, druid_data.all_products);

	while ((dent = readdir (dir))) {
		if (dent->d_name[0] == '.')
			continue;

		p = strrchr (dent->d_name, '.');
		if (!p || strcmp (p, ".bugzilla")) 
			continue;	
		
		p = g_build_filename (BUDDY_DATADIR, "bugzilla", dent->d_name, NULL);
		d(g_print ("trying to load `%s'\n", p));
		bts = load_bugzilla (p);
		g_free (p);
		if (bts) {
			d(g_print ("bugzilla loaded: %s\n", bts->name));
			druid_data.bugzillas = g_slist_append (druid_data.bugzillas, bts);
		}
	}

	closedir (dir);

	if (!druid_data.dlsources) {
		w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("Bug Buddy could not find any information on "
					      "where to submit bugs.\n\n"
					      "Please make sure Bug Buddy was "
					      "installed correctly.\n\n"
					      "Bug Buddy will now quit."),
					    BUDDY_DATADIR "/bugzilla/");
		gtk_dialog_set_default_response (GTK_DIALOG (w),
						 GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		exit (0);
	}

	d(g_print ("downloading:\n"));
	d(g_list_foreach (druid_data.dlsources, (GFunc)p_string, NULL));
	d(g_print ("to:\n"));
	d(g_list_foreach (druid_data.dldests, (GFunc)p_string, NULL));
}

static void
add_product (BugzillaProduct *p, GtkListStore *store)
{
	GtkTreeIter iter;

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    PRODUCT_ICON, p->bts->pixbuf,
			    PRODUCT_NAME, p->name,
			    PRODUCT_DESC, p->description,
			    PRODUCT_DATA, p,
			    -1);
}

void
bugzilla_bts_add_products_to_clist (BugzillaBTS *bts)
{
	GtkTreeView *w;
	GtkListStore *store;

	w = GTK_TREE_VIEW (GET_WIDGET ("product-list"));

	g_object_get (G_OBJECT (w), "model", &store, NULL);

	gtk_list_store_clear (store);
	druid_data.product = NULL;

	g_slist_foreach (bts->products, (GFunc)add_product, store);
	gtk_tree_view_columns_autosize (w);
}

static void
add_component (BugzillaComponent *comp, GtkListStore *store)
{
	GtkTreeIter iter;

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    COMPONENT_NAME, comp->name,
			    COMPONENT_DESC, comp->description,
			    COMPONENT_DATA, comp,
			    -1);
}

static void
update_severity (GtkWidget *w, gpointer data)
{
	druid_data.severity = (char *)data;
}

static void
add_severity (char *s, GtkMenu *m)
{
	GtkWidget *w;

	w = gtk_menu_item_new_with_label (s);
	g_signal_connect (G_OBJECT (w), "activate",
			  G_CALLBACK (update_severity),
			  s);
	gtk_widget_show (w);
	gtk_menu_shell_append (GTK_MENU_SHELL (m), w);
	if (!strcasecmp (s, "normal") || !strcasecmp (s, "unknown")) {
		gtk_menu_item_activate (GTK_MENU_ITEM (w));
		gtk_option_menu_set_history (GTK_OPTION_MENU (GET_WIDGET ("severity-list")), 
					     g_slist_index (druid_data.product->bts->severities, s));
	}
}

void
bugzilla_product_add_components_to_clist (BugzillaProduct *prod)
{
	GtkWidget *m, *c;
	GtkTreeView *w;
	GtkListStore *store;

	w = GTK_TREE_VIEW (GET_WIDGET ("component-list"));

	g_object_get (G_OBJECT (w), "model", &store, NULL);

	gtk_list_store_clear (store);
	druid_data.component = NULL;

	g_slist_foreach (prod->components, (GFunc)add_component, store);

	gtk_tree_view_columns_autosize (w);

	m = gtk_menu_new ();
	c = GET_WIDGET ("severity-list");
	gtk_option_menu_set_menu (GTK_OPTION_MENU (c), m);
	g_slist_foreach (prod->bts->severities, (GFunc)add_severity, m);
}

static void
add_mostfreq (BugzillaBug *bug, GtkListStore *store)
{
	GtkTreeIter iter;

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    MOSTFREQ_PRODUCT,   bug->product,
			    MOSTFREQ_COMPONENT, bug->component,
			    MOSTFREQ_URL,       bug->url,
			    MOSTFREQ_ID,        bug->id,
			    MOSTFREQ_DESC,      bug->desc,
			    -1);
}

gboolean
bugzilla_add_mostfreq (BugzillaBTS *bts)
{
	/* GtkWidget *m, *c; */
	GtkTreeView *w;
	GtkListStore *store;
	GtkTreeSelection *selection;
	
	w = GTK_TREE_VIEW (GET_WIDGET ("mostfreq-list"));
	g_object_get (G_OBJECT (w), "model", &store, NULL);;

	gtk_list_store_clear (store);

	g_slist_foreach (bts->bugs, (GFunc)add_mostfreq, store);

	gtk_tree_view_columns_autosize (w);

	selection = gtk_tree_view_get_selection (w);
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);

	return bts->bugs == NULL;
}

#define LINE_WIDTH 72
static void
append_line_width (GString *str, char *s)
{
	gchar *sp;
	if (!s) return;

	if (strlen (s) < LINE_WIDTH) {
		g_string_append (str, s);
		g_string_append_c (str, '\n');
		return;
	}

	for (sp = s+LINE_WIDTH; sp > s && !isspace (*sp); sp--)
		;

	if (s == sp) sp = strpbrk (s+LINE_WIDTH, "\t\n ");
       
	if (sp)	*sp = '\0';

	g_string_append (str, s);
	g_string_append_c (str, '\n');

	if (sp) append_line_width (str, sp+1);
}

static char *
format_for_width (const char *s)
{
	GString *str;
	int i;
	char **sv, *r;

	str = g_string_new (NULL);

	sv = g_strsplit (s, "\n", -1);
	for (i = 0; sv[i]; i++)
		append_line_width (str, sv[i]);

	r = str->str;
	g_string_free (str, FALSE);
	return r;
}

/*
 * from bugmail_help.html:
 * 
 * Example Mail
 * See the example of the mail body (Dont forget to specify the short description in the mail subject):
 *
 * @product      = Bugzilla
 * @component    = general
 * @version      = All
 * @groupset     = ReadWorld ReadPartners
 * @op_sys       = Linux
 * @priority     = P3
 * @rep_platform = i386
 *
 *
 * This is the description of the bug I found. It is not neccessary to start
 * it with a keyword. 
 *
 * Note: The short_description is neccessary and may be given with the keyword
 * @short_description or will be retrieved from the mail subject.
 */ 

gchar *
generate_email_text (gboolean include_headers)
{	
	char *subject, *product,  *component, *version;
	char *opsys,   *platform, *severity,  *body, *tmp_body;
	char *debug_info;
	char *email, *email1= NULL, *email2;
	char *gnome_version;
#if 0
	char *text_file, *sysinfo;
#endif

	gboolean is_bugzilla = !GTK_TOGGLE_BUTTON (GET_WIDGET ("no-product-toggle"))->active;

	subject    = buddy_get_text ("desc-subject");
	if (is_bugzilla) {
		product    = druid_data.product->name;
		component  = druid_data.component->name;
		version    = buddy_get_text ("the-version-entry");
	} else {
		product = component = version = NULL;
	}
	opsys      = "Linux";
	platform   = "Debian";
	severity   = druid_data.severity ? druid_data.severity : "Normal";
	/* sysinfo    = generate_sysinfo (); */
	gnome_version = druid_data.gnome_version
		? g_strdup_printf ("BugBuddy-GnomeVersion: %s\n", druid_data.gnome_version)
		: "";

	tmp_body   = buddy_get_text ("desc-text");		
	body = format_for_width (tmp_body);
	g_free (tmp_body);

	tmp_body = buddy_get_text ("gdb-text");
	debug_info = format_for_width (tmp_body);
	g_free (tmp_body);

	if (!is_bugzilla) {
		if (include_headers)
			email1 = g_strdup_printf ("Subject: %s\n\n", subject);
		email2 = g_strdup_printf (
			"(This bug report was generated by Bug Buddy " VERSION ")\n"
			"%s\n"
			"\n",
			body);
	} else if (druid_data.product->bts->submit_type == BUGZILLA_SUBMIT_FREITAG) {
		if (include_headers)
			email1 = g_strdup_printf ("Subject: %s\n\n", subject);
				
		email2 = g_strdup_printf (
			"@product = %s\n"
			"@component = %s\n"
			"@version = %s\n"
#if 0
			"@op_sys = %s\n"
			"@rep_platform = %s\n"
#endif
			"@severity = %s\n"
			"\n"
			"%s\n"
			"\n",
			product,
			component,
			version,
#if 0
			opsys,
			platform,
#endif
			severity,
			body);
	} else if (druid_data.product->bts->submit_type == BUGZILLA_SUBMIT_DEBIAN) {
		if (include_headers)
			email1 = g_strdup_printf ("Subject: %s\n\n", subject);

		email2 = g_strdup_printf (
			"Package: %s\n"
			"%s: %s\n"
			"Version: %s\n"
			"Synopsis: %s\n"
			"Bugzilla-Product: %s\n"
			"Bugzilla-Component: %s\n"
			"%s"
			"\n"
			"Description:\n"
			"%s\n"
			"\n",
			product,
			druid_data.product->bts->severity_header,
			severity,
			version,
			subject,
			product,
			component,
			gnome_version,
			body);
	} else 
		email2 = g_strdup ("Unkown format.");
	
	email = g_strconcat (email1 ? email1 : "", 
			     email2, 
			     druid_data.crash_type != CRASH_NONE && debug_info[0]
			     ? "\nDebugging Information:\n\n"
			     : NULL,
			     debug_info, NULL);

	g_free (email1);
	g_free (email2);
	g_free (subject);
	g_free (version);
	g_free (body);
	g_free (debug_info);
	if (druid_data.gnome_version)
		g_free (gnome_version);

	return email;
}
