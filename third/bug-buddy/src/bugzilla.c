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
#include "md5-utils.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <utime.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <gnome.h>

#include <libgnomevfs/gnome-vfs.h>

#include <libgnome/gnome-desktop-item.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo-activation/bonobo-activation.h>

#include <dirent.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#define CHECK_UPDATES_TIME (3600*24)

/* define to x for some debugging output */
#define d(x)

static void
bugzilla_bts_insert_product (BugzillaBTS *bts, BugzillaProduct *prod)
{
	g_hash_table_insert (bts->products, prod->name, prod);
}

static void
bugzilla_product_insert_component (BugzillaProduct *prod, BugzillaComponent *comp)
{
	g_hash_table_insert (prod->components, comp->name, comp);
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

	/* FIXME: g_hash_table_foreach_remove(); */
	bts->products = g_hash_table_new (g_str_hash, g_str_equal);

	for (node = xmlDocGetRootElement (doc)->children; node; node = node->next) {
		d(g_print ("\t%s\n", node->name));
		if (!strcmp (node->name, "product")) {
			prod = g_new0 (BugzillaProduct, 1);
			prod->bts = bts;

			prod->name        = XML_NODE_GET_PROP (node, "name");
			prod->description = XML_NODE_GET_PROP (node, "description");
			prod->components  = g_hash_table_new (g_str_hash, g_str_equal);

			bugzilla_bts_insert_product (bts, prod);

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

static int
async_update (GnomeVFSAsyncHandle *handle, GnomeVFSXferProgressInfo *info, gpointer data)
{
	static int count = 0;
	
	d(g_print ("%" GNOME_VFS_SIZE_FORMAT_STR "\n", info->bytes_copied));

	if (info->status == GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR) {
		end_bugzilla_download (0);
		g_message ("there was an error........");
		return GNOME_VFS_XFER_ERROR_ACTION_ABORT;
	}

	if (info->bytes_total) {
		char *text_count;
		if (info->file_index>0)
			count++;
		text_count = g_strdup_printf (_("%d of %d"), info->file_index, info->files_total);
		buddy_set_text ("progress-count", text_count);
		g_free (text_count);
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (GET_WIDGET ("progress-progress")), 
					       (gfloat)info->total_bytes_copied / info->bytes_total);
	}

	if (info->phase == GNOME_VFS_XFER_PHASE_COMPLETED) {
		d(g_print ("Transfer finished\n"));
		end_bugzilla_download (count);
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

static gboolean
md5str_equal_digest(char *md5, guchar digest[16])
{
	int i;
	char cmp[3];
	for (i=0;i<=15;i++) {
		g_snprintf(cmp, sizeof(cmp), "%02x", (guint)digest[i]);
		if (cmp[0]!=md5[i*2] || cmp[1]!=md5[i*2+1])
			return FALSE;
	}
	return TRUE;
}

static BugzillaXMLFile *
get_xml_file (BugzillaBTS *bts, const char *key, char md5[33], XMLFunc parse_func)
{
	BugzillaXMLFile *xmlfile;
	char *localdir, *tmppath, *src_uri, *tmp_key;
	guchar digest[16];
	int res;
	
	gboolean need_to_download = FALSE;
	

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

	if (!bts->md5s_downloaded || !md5 || md5=="")
		return xmlfile;

	md5_get_digest_from_file (xmlfile->cache_path, digest);
	if (md5str_equal_digest(md5, digest))
		return xmlfile;
		
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

	d(g_print("Need to download %s\n", xmlfile->cache_path));

	druid_data.dlsources = g_list_prepend (druid_data.dlsources, xmlfile->source_uri);
	druid_data.dldests   = g_list_prepend (druid_data.dldests,   xmlfile->dest_uri);

	xmlfile->download = xmlfile->read_from_cache = TRUE;
	return xmlfile;
}

static void
bugzilla_get_md5sums (BugzillaBTS *bts, char *url)
{
	GnomeVFSHandle *handle;
	GnomeVFSResult res;
	GnomeVFSFileSize read;
	char *buffer;
	char md5str[33];
	char *products_filename;
	char *config_filename;
	char *mostfreq_filename;
	char *ptr;

	buffer = g_malloc0(1024*sizeof(char));

	res = gnome_vfs_open (&handle, url, GNOME_VFS_OPEN_READ);
	if (res != GNOME_VFS_OK) {
		druid_data.last_update_check = 0;
		return;
	}

	res = gnome_vfs_read (handle, buffer, 1024, &read);
	
	/* md5sum file greater than 1024 is really wrong! */
	if (res != GNOME_VFS_OK || read >= 1024) {
		druid_data.last_update_check = 0;
		return;
	}

	druid_data.last_update_check++;

	gnome_vfs_close (handle);

	products_filename = rindex (gnome_config_get_string("products"), '/') + 1;
	config_filename = rindex (gnome_config_get_string("config"), '/') + 1;
	mostfreq_filename = rindex (gnome_config_get_string("mostfreq"), '/') + 1;
	ptr = buffer;

	while (*ptr) {
		sscanf (ptr, "%32s", md5str);
		ptr = index (ptr, ' ');
		ptr += 2;
		if (!strncmp (products_filename, ptr, strlen(products_filename))) {
			strncpy(bts->products_xml_md5, md5str, 32);
			ptr += strlen (products_filename);
			if (ptr)
				ptr++;
		} else if (!strncmp (config_filename, ptr, strlen(config_filename))) {
			strncpy(bts->config_xml_md5, md5str, 32);
			ptr += strlen (config_filename);
			if (ptr)
				ptr++;
		} else if (!strncmp (mostfreq_filename, ptr, strlen(mostfreq_filename))) {
			strncpy(bts->mostfreq_xml_md5, md5str, 32);
			ptr += strlen (mostfreq_filename);
			if (ptr)
				ptr++;
		} 
	}
	bts->md5s_downloaded = TRUE;

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

	g_free (path);

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
	if ((time (NULL) - druid_data.last_update_check) > CHECK_UPDATES_TIME )
		bugzilla_get_md5sums (bts, gnome_config_get_string ("md5sums"));
	
	bts->products_xml = get_xml_file (bts, "products", 
					  bts->products_xml_md5,
					  load_products_xml);
	
	bts->config_xml   = get_xml_file (bts, "config",
					  bts->config_xml_md5,
					  load_config_xml);
	
	bts->mostfreq_xml = get_xml_file (bts, "mostfreq",
					  bts->mostfreq_xml_md5,
					  load_mostfreq_xml);

	gnome_config_pop_prefix ();

		

	if ((bts->products_xml && bts->products_xml->download) ||
	    (bts->config_xml   && bts->config_xml->download) || 
	    (bts->mostfreq_xml   && bts->mostfreq_xml->download))
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

static void
load_bugzilla_xml_cb (gpointer key, BugzillaBTS *bts, GtkListStore *store)
{
	xmlDoc *doc;

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
}

void
load_bugzilla_xml (void)
{
	d(g_print ("loading xml..\n"));

	g_hash_table_foreach (druid_data.bugzillas, (GHFunc)load_bugzilla_xml_cb, NULL);

	if (druid_data.show_products)
		products_list_load ();
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

gboolean
open_mostfreq_bug (const char *path_str)
{

	GtkTreeView *view;
	GtkListStore *store;
	GtkTreeIter iter;
	GtkTreePath *path;
	GValue value = {0};

	view = GTK_TREE_VIEW (GET_WIDGET ("mostfreq-list"));
	g_object_get (G_OBJECT (view), "model", &store, NULL);
		                                                                                           
	path = gtk_tree_path_new_from_string (path_str);
	gtk_tree_model_get_iter (GTK_TREE_MODEL (store), &iter, path);
	g_free(path);
					                                                                                           
	gtk_tree_model_get_value (GTK_TREE_MODEL (store), &iter, MOSTFREQ_URL, &value);
	d(g_print("Asked to go to %s\n", g_value_get_string (&value)));
	
	if (gnome_url_show (g_value_get_string (&value), NULL)) {
		g_value_unset (&value);
		return TRUE;
	} 
	
	g_value_unset (&value);
	return FALSE;
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

	/* sort on Product name */
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), PRODUCT_NAME, GTK_SORT_ASCENDING);

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

	/* sort on Product name */
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model), COMPONENT_NAME, GTK_SORT_ASCENDING);
	
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
end_bugzilla_download (int count)
{
	druid_data.download_in_progress = FALSE;
	if (count>0)
		gtk_dialog_response (GTK_DIALOG (GET_WIDGET ("progress-dialog")),
				     DOWNLOAD_FINISHED_OK_RESPONSE);
	else
		gtk_dialog_response (GTK_DIALOG (GET_WIDGET ("progress-dialog")),
				     DOWNLOAD_FINISHED_ZERO_RESPONSE);

}

gboolean
start_bugzilla_download (void)
{
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

	druid_data.bugzillas = g_hash_table_new (g_str_hash, g_str_equal);

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
			g_hash_table_insert (druid_data.bugzillas, bts->name, bts);
		}
	}

	closedir (dir);

	if (druid_data.need_to_download && !druid_data.dlsources) {
		w = gtk_message_dialog_new (GTK_WINDOW (GET_WIDGET ("druid-window")),
					    0,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("Bug Buddy could not find any information on "
					      "where to submit bugs.\n\n"
					      "Please make sure Bug Buddy was "
					      "installed correctly.\n\n"
					      "Bug Buddy will now quit."));
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

#define ALL_APPLICATIONS_URI "applications-all-users:///"
#define APPLET_REQUIREMENTS \
	"has_all (repo_ids, ['IDL:Bonobo/Control:1.0'," \
	"		     'IDL:GNOME/Vertigo/PanelAppletShell:1.0']) && " \
	"defined (panel:icon) && defined (panel:category)"

#define BUGZILLA_BUGZILLA               "X-GNOME-Bugzilla-Bugzilla"
#define BUGZILLA_PRODUCT                "X-GNOME-Bugzilla-Product"
#define BUGZILLA_COMPONENT              "X-GNOME-Bugzilla-Component"
#define BUGZILLA_EMAIL                  "X-GNOME-Bugzilla-Email"
#define BUGZILLA_OTHER_BINARIES         "X-Gnome-Bugzilla-OtherBinaries"

#define BA_BUGZILLA_BUGZILLA            "bugzilla:bugzilla"
#define BA_BUGZILLA_PRODUCT             "bugzilla:product"
#define BA_BUGZILLA_COMPONENT           "bugzilla:component"
#define BA_BUGZILLA_EMAIL               "bugzilla:email"
#define BA_BUGZILLA_OTHER_BINARIES      "bugzilla:other_binaries"

static void
bugzilla_application_new (const char *name, 
			  const char *comment, 
			  const char *bugzilla, 
			  const char *product,
			  const char *component, 
			  const char *email, 
			  const char *icon,
			  const char *program,
			  const char *other_programs)
{
	BugzillaApplication *app;
	GdkPixbuf *pb = NULL;
	char *icon_copy;
	GError *error = NULL;
	char **programv;
	int i;

	app = g_new0 (BugzillaApplication, 1);
	
	app->name        = g_strdup (name);
	app->comment     = g_strdup (comment);
	app->bugzilla    = g_strdup (bugzilla);
	app->product     = g_strdup (product);
	app->component   = g_strdup (component);
	app->email       = g_strdup (email);

	if (icon && *icon != G_DIR_SEPARATOR)
		icon_copy = gnome_program_locate_file (NULL,
						       GNOME_FILE_DOMAIN_PIXMAP,
						       icon, TRUE, NULL);
	else
		icon_copy = g_strdup (icon);

	if (icon_copy) {
		pb = gdk_pixbuf_new_from_file (icon_copy, &error);
		g_free (icon_copy);
	}

	if (pb) {
		app->pixbuf = gdk_pixbuf_scale_simple (pb, 20, 20, GDK_INTERP_HYPER);
		g_object_unref (pb);
	} else if (error) {
		g_warning (_("Couldn't load icon for %s: %s"), app->name, error->message);
		g_error_free (error);
	}
	
	druid_data.applications = g_slist_prepend (druid_data.applications, app);

	if (program) {
		g_shell_parse_argv (program, &i, &programv, NULL);
		if (programv[0]) {
			char *s;
			s = strrchr (programv[0], G_DIR_SEPARATOR);
			s = s ? s+1 : programv[0];
			if (g_hash_table_lookup (druid_data.program_to_application, s) == NULL) {
				d(g_print ("adding app: %s\n", s));
				g_hash_table_insert (druid_data.program_to_application, g_strdup (s), app);
			}
		}
		if (programv)
			g_strfreev (programv);
	}

	if (other_programs) {
		programv = g_strsplit (other_programs, ";", -1);
		for (i=0; programv[i]; i++) {
			if (g_hash_table_lookup (druid_data.program_to_application, programv[i]) == NULL) {
				d(g_print ("adding app: %s\n", programv[i]));
				g_hash_table_insert (druid_data.program_to_application, g_strdup (programv[i]), app);
			}
		}
		g_strfreev (programv);
	}
}

static const GSList *
get_i18n_slist (void)
{
  GList *langs_glist;
  static GSList *langs_gslist;

  if (langs_gslist)
	  return langs_gslist;

  langs_glist = (GList *)gnome_i18n_get_language_list ("LC_MESSAGES");
  langs_gslist = NULL;
  while (langs_glist != NULL) {
    langs_gslist = g_slist_append (langs_gslist, langs_glist->data);
    langs_glist = langs_glist->next;
  }

  return langs_gslist;
}

static void
load_applets (GnomeIconTheme *git)
{
	Bonobo_ServerInfoList *info_list;
	Bonobo_ServerInfo *info;
	CORBA_Environment ev;
	GSList *langs;
	int i;

	CORBA_exception_init (&ev);
	info_list = bonobo_activation_query (APPLET_REQUIREMENTS, NULL, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("Applet list query failed: %s", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		return;
	}
	CORBA_exception_free (&ev);

	langs = (GSList *)get_i18n_slist ();

	for (i = 0; i < info_list->_length; i++) {
		info = info_list->_buffer + i;

		bugzilla_application_new (
			bonobo_server_info_prop_lookup (info, "name", langs),
			bonobo_server_info_prop_lookup (info, "description", langs),
			bonobo_server_info_prop_lookup (info, BA_BUGZILLA_BUGZILLA,  NULL),
			bonobo_server_info_prop_lookup (info, BA_BUGZILLA_PRODUCT,   NULL),
			bonobo_server_info_prop_lookup (info, BA_BUGZILLA_COMPONENT, NULL),
			bonobo_server_info_prop_lookup (info, BA_BUGZILLA_EMAIL,     NULL),
			bonobo_server_info_prop_lookup (info, "panel:icon", NULL),
			NULL,
			bonobo_server_info_prop_lookup (info, BA_BUGZILLA_OTHER_BINARIES, NULL));
	}

	CORBA_free (info_list);
}

static gboolean
visit_cb (const char *rel_path,
	  GnomeVFSFileInfo *info,
	  gboolean recursing_will_loop,
	  gpointer data,
	  gboolean *recurse)
{
	GnomeDesktopItem *ditem;
	GError *error = NULL;
	char *full_path, *icon;
	GnomeIconTheme *git = data;
	
	if (info->name[0] != '.'
	    || (info->name[1] != '.' && info->name[1] != 0)
	    || info->name[2] != 0) {
		if (recursing_will_loop) {
			g_print ("Loop detected\n");
			return TRUE;
		}
		*recurse = TRUE;
	} else {
		*recurse = FALSE;
	}

	full_path = g_strconcat (ALL_APPLICATIONS_URI, rel_path, NULL);

	ditem = gnome_desktop_item_new_from_uri (full_path, 0, &error);
	if (!ditem) {
		if (error) {
			g_warning ("Couldn't load %s: %s", full_path, error->message);
		}
		goto visit_cb_out;
	}
    
	if (gnome_desktop_item_get_entry_type (ditem) != GNOME_DESKTOP_ITEM_TYPE_APPLICATION)
		goto visit_cb_out;

	icon = gnome_desktop_item_get_icon (ditem, git);
	bugzilla_application_new (
		gnome_desktop_item_get_localestring (ditem, GNOME_DESKTOP_ITEM_NAME),
		gnome_desktop_item_get_localestring (ditem, GNOME_DESKTOP_ITEM_COMMENT),
		gnome_desktop_item_get_string (ditem, BUGZILLA_BUGZILLA),
		gnome_desktop_item_get_string (ditem, BUGZILLA_PRODUCT),
		gnome_desktop_item_get_string (ditem, BUGZILLA_COMPONENT),
		gnome_desktop_item_get_string (ditem, BUGZILLA_EMAIL),
		icon,
		gnome_desktop_item_get_string (ditem, GNOME_DESKTOP_ITEM_EXEC),
		gnome_desktop_item_get_string (ditem, BUGZILLA_OTHER_BINARIES));
	g_free (icon);

 visit_cb_out:
	g_free (full_path);
	if (error)
		g_error_free (error);
	if (ditem)
		gnome_desktop_item_unref (ditem);

	return TRUE;
}

void
load_applications (void)
{
	GnomeIconTheme *git;

	druid_data.program_to_application = g_hash_table_new (g_str_hash, g_str_equal);

	git = gnome_icon_theme_new ();
	gnome_icon_theme_set_allow_svg (git, FALSE);
	
	/* load applications */
	gnome_vfs_directory_visit (ALL_APPLICATIONS_URI,
				   GNOME_VFS_FILE_INFO_FOLLOW_LINKS,
				   GNOME_VFS_DIRECTORY_VISIT_LOOPCHECK,
				   visit_cb, git);


	load_applets (git);

	g_object_unref (git);
}

static void
add_component (gpointer key, BugzillaComponent *comp, GtkListStore *store)
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

static void
add_product (gpointer key, BugzillaProduct *p, GtkListStore *store)
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

static void
add_products (gpointer key, BugzillaBTS *bts, GtkListStore *store)
{
	g_hash_table_foreach (bts->products, (GHFunc)add_product, store);
}

static void
add_application (BugzillaApplication *app, GtkListStore *store)
{
	gtk_list_store_append (store, &app->iter);
	gtk_list_store_set (store, &app->iter,
			    PRODUCT_ICON, app->pixbuf,
			    PRODUCT_NAME, app->name,
			    PRODUCT_DESC, app->comment,
			    PRODUCT_DATA, app,
			    -1);
}

static gboolean
unselect (gpointer data)
{
	gtk_tree_selection_unselect_all (data);
	return FALSE;
}

void
products_list_load (void)
{
	BugzillaApplication *app;
	GtkTreeView *view;
	GtkListStore *store;
	GtkTreeSelection *selection;
	GtkTreeIter *iter = NULL;

	view = GTK_TREE_VIEW (GET_WIDGET ("product-list"));
	selection = gtk_tree_view_get_selection (view);
	g_object_get (G_OBJECT (view), "model", &store, NULL);
	gtk_list_store_clear (store);

	druid_data.product = NULL;

	if (druid_data.show_products) {
		if (druid_data.download_in_progress)
			return;
		g_hash_table_foreach (druid_data.bugzillas, (GHFunc)add_products, store);
	} else {
		g_slist_foreach (druid_data.applications, (GFunc)add_application, store);
	}
	gtk_tree_view_columns_autosize (view);
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

	g_hash_table_foreach (prod->components, (GHFunc)add_component, store);

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
	GString *email;
	char *subject, *body, *s, *debug_info, *version, *severity;

	gboolean is_bugzilla;
       
	is_bugzilla = !(GTK_TOGGLE_BUTTON (GET_WIDGET ("no-product-toggle"))->active || druid_data.product == NULL);

	email = g_string_new (NULL);

	subject    = buddy_get_text ("desc-subject");
	version    = buddy_get_text ("the-version-entry");

	s          = buddy_get_text ("desc-text");
	body = format_for_width (s);
	g_free (s);

	s = buddy_get_text ("gdb-text");
	debug_info = format_for_width (s);
	g_free (s);

	severity = druid_data.severity ? druid_data.severity : "Normal";

	if (include_headers)
		g_string_append_printf (email, "Subject: %s\n\n", subject);

	if (!is_bugzilla) {
		g_string_append (email, "(This bug report was generated by Bug Buddy " VERSION ")\n");
	} else if (druid_data.product->bts->submit_type == BUGZILLA_SUBMIT_DEBIAN) {
		/* This is what gnome.org and ximian.com use */
		g_string_append_printf (email, "Package: %s\n", druid_data.product->name);
		g_string_append_printf (email, "%s: %s\n", druid_data.product->bts->severity_header, severity);					
		g_string_append_printf (email, "Version: %s %s\n",
					druid_data.gnome_platform_version
					? druid_data.gnome_platform_version
					: "",
					version);
		if (druid_data.gnome_vendor)
			g_string_append_printf (email, "os_details: %s\n", druid_data.gnome_vendor);
		g_string_append_printf (email, "Synopsis: %s\n", subject);
		g_string_append_printf (email, "Bugzilla-Product: %s\n", druid_data.product->name);
		g_string_append_printf (email, "Bugzilla-Component: %s\n", druid_data.component->name);
		if (druid_data.gnome_version)
			g_string_append_printf (email, "BugBuddy-GnomeVersion: %s\n", druid_data.gnome_version);
		g_string_append (email, "Description:\n");
	} else if (druid_data.product->bts->submit_type == BUGZILLA_SUBMIT_FREITAG) {
		/* i don't know of anything that uses this */
		g_string_append_printf (email, "@product = %s\n", druid_data.product->name);
		g_string_append_printf (email, "@component = %s\n", druid_data.component->name);
		g_string_append_printf (email, "@version = %s\n", version);
#if 0
		g_string_append_printf (email, "@op_sys = %s\n", opsys);
		g_string_append_printf (email, "@rep_platform = %s\n", platform);
#endif
		g_string_append_printf (email, "@severity = %s\n", severity);
	}

	g_string_append (email, body);

	if (druid_data.crash_type != CRASH_NONE && *debug_info)
		g_string_append_printf (email, "\n\nDebugging Information:\n\n%s\n", debug_info);

	g_free (subject);
	g_free (version);
	g_free (body);
	g_free (debug_info);

	s = email->str;

	g_string_free (email, FALSE);

	return s;
}
