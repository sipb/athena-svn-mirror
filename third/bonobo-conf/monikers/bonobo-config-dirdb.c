/**
 * bonobo-config-xmldirdb.c: xml configuration database implementation.
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 *
 * Description:
 * 
 * This backend uses a separate file to store the data of each configuration
 * directory. Unlike GConf, all files are stored in a single file system
 * directory. There is a 1:1 mapping between the configuration key and the
 * filename, for example the value "/Applications/Gnumeric/autosave" is stored
 * in the file "%Application:Gnumeric.xmldb". All files start with a "%" sign
 * and ends in ".xmldb", and all slashes are replaced with colons.
 *
 * The stored files have the same format as the xmldb backend, but only
 * contains one section tag.  
 *
 * Performance should be quite good because we keep most things in
 * memory. Memory usage is also limited because we use a cache and unload
 * unused directories. But maybe we should use a fully associative cache with
 * LRU replacement instead of a direct mapped cache.
 * 
 **/
#include <config.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <bonobo/bonobo-arg.h>
#include <bonobo/bonobo-property-bag-xml.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo-conf/bonobo-config-utils.h>
#include <gnome-xml/xmlmemory.h>
#include <gtk/gtkmain.h>

#include "bonobo-config-dirdb.h"

static GtkObjectClass *parent_class = NULL;

#define CLASS(o) BONOBO_CONFIG_DIRDB_CLASS (GTK_OBJECT(o)->klass)

#define PARENT_TYPE BONOBO_CONFIG_DATABASE_TYPE

#define CSIZE 37  /* should be a prime number */

#define FLUSH_INTERVAL 30 /* 30 seconds */

typedef struct _DirData DirData;

struct _DirData {
	char       *name;
	GSList     *entries;
	xmlDocPtr   doc;
	gboolean    dirty;
};

typedef struct {
	char       *name;
	CORBA_any  *value;
	xmlNodePtr  node;
	DirData    *dir;
} DirEntry;

typedef GNode DTree;
 
typedef struct _BonoboConfigDIRDBPriv DirCache;

struct _BonoboConfigDIRDBPriv {
	char              *basedir;
	DirData           *data [CSIZE];
	DTree             *dtree;
	BonoboEventSource *es;
	guint              time_id;
};


/** 
 * returns the first directory name, rest points to the rest of the string. We
 * use colon and slashes as delimiters
 **/
static char *
split_name (const char *dir, const char **rest)
{
	char *name;
	int i = 0, j, l;

	while (dir [i] == '/' || dir [i] == ':')
		i++;

	j = i;

	while (dir [j] && dir [j] != '/' && dir [j] != ':')
		j++;

	l = j - i;

	name = g_strndup (&dir [i], l);
	
	if (dir [j]) 
		*rest = &dir [j];
	else 
		*rest = NULL;

	return name;
}

/**
 * remove unnecessary slashes from a key name and check if key is valid. All
 * leading, trailing and sequences of slashes are removed. 
 **/
static char *
normalize_key (const char *key)
{
	char *nkey;
	int l, i = 0, j = 0;

	if (!key)
		return NULL;

	l = strlen (key);

	nkey = g_new0 (char, l + 1);

	while (key [i] == '/')
		i++;

	while ((nkey [j++] = key [i])) {
		if (key [i] == ':') {
			g_free (nkey);
			return NULL;
		}
		if (key [++i] == '/') {
			nkey [j++] = key [i];
			while (key [++i] == '/');
		}
	}

	if (--j && nkey [j-1] == '/')
		nkey [j-1] = '\0';

	return nkey;
}

/**
 * implements the mapping between directory names and filesystem paths
 **/
static char *
dir_to_path (const char *prefix, const char *dir)
{
	char *fn, *path;
	int i = 0;

	fn = g_strconcat ("%", dir, ".xmldb", NULL); 

	while (fn [i]) {
		if (fn [i] == '/')
			fn [i] = ':';
		i++;
	}

	path = g_strconcat (prefix, "/", fn, NULL);

	g_free (fn);

	return path;
}

/**
 * We use a tree to store all directory names in memory. This function adds a
 * directory to the tree. A DTree is simply a GNode, and node->data contains
 * the directory name.
 **/
static void
dtree_add_dir (DTree *dt, const char *dir)
{
	char *name;
	const char *rest;
	GNode *n;

	name = split_name (dir, &rest);

	for (n = dt->children; n; n = n->next) {
		if (!strcmp (n->data, name)) {
			g_free (name);
			break;
		}
	}
	
	if (!n) {
		n = g_node_new (name);
		g_node_prepend (dt, n);
	}

	if (rest)
		dtree_add_dir (n, rest);
}

/**
 * check if directory dir exists, and returns a ponter to the corresponding
 * GNode.
 **/
static GNode *
dtree_lookup_dir (DTree *dt, const char *dir)
{
	char *name;
	const char *rest;
	GNode *n;

	name = split_name (dir, &rest);

	for (n = dt->children; n; n = n->next)
		if (!strcmp (n->data, name)) 
			break;

	g_free (name);

	if (!n)
		return NULL;

	if (rest)
		return dtree_lookup_dir (n, rest);
	else 
		return n;
}

/**
 * remove the directory from the tree, but only if there are no more subdirs.
 **/
static void
dtree_remove_dir (DTree *dt, const char *dir)
{
	GNode *n;

	if ((n = dtree_lookup_dir (dt, dir)) && !n->children) {
		g_free (n->data);
		g_node_destroy (n);
	}
}

/**
 * read the contents of basedir, and create the corresponding directory tree.
 **/
static DTree *
dtree_create (const char *basedir)
{
	DIR *d;
	struct dirent *e;
	DTree *dt;
	int l;

	dt = g_node_new (NULL);

	d = opendir (basedir);

	while ((e = readdir (d))) {

		l = strlen (e->d_name);

		if (l < 6 || e->d_name [0] != '%' || 
		    strcmp (e->d_name + l - 6, ".xmldb"))
			continue;
		
		e->d_name [l - 6] = '\0';

		dtree_add_dir (dt, &e->d_name [1]);
	}

	return dt;
}

/**
 * free all directory tree memory.
 **/
static void
dtree_destroy (DTree *dt)
{
	GNode *n;

	if (!dt)
		return;

	for (n = dt->children; n; n = n->next) {
		dtree_destroy (n);
	}

	g_free (dt->data);
}

/**
 * try to find entry #name inside #dd. If #create is TRUE the entry is
 * allocated if it does not exist.
 **/
static DirEntry *
dir_lookup_entry (DirData *dd, const char *name, gboolean create)
{
	GSList *l;
	DirEntry *de;
	
	for (l = dd->entries; l; l = l->next) {
		de = (DirEntry *)l->data;

		if (!strcmp (de->name, name))
			return de;
	}

	if (create) {

		de = g_new0 (DirEntry, 1);
		
		de->dir = dd;
		de->name = g_strdup (name);

		dd->entries = g_slist_prepend (dd->entries, de);

		return de;
	}

	return NULL;
}

/**
 * loads the corresponding .xmldb file into a DirData structure. If create is
 * TRUE the structure is allocated if the file does not exist.
 **/
static DirData *
dcache_load_dir (DirCache *cache, const char *dir, gboolean create)
{
	CORBA_Environment ev;
	DirData *dd;
	DirEntry *de;
	xmlDocPtr doc;
	xmlNodePtr n;
	char *path, *ndir, *name;

	if (!(ndir = normalize_key (dir)))
		return NULL;

	path = dir_to_path (cache->basedir, ndir);

	if (!(doc = xmlParseFile (path)) && !create) {
		g_free (ndir);
		g_free (path);
		return NULL;
	}

	g_free (path);
	
	if (!doc || strcmp (doc->root->name, "bonobo-config")) {
		xmlFreeDoc (doc);
		doc = xmlNewDoc("1.0");
		doc->root = xmlNewDocNode (doc, NULL, "bonobo-config", NULL);
		xmlNewChild (doc->root, NULL, "section", NULL);
	}

	n = doc->root->childs;
	if (!n || n->type != XML_ELEMENT_NODE || strcmp (n->name, "section")) {
		xmlFreeDoc (doc);
		g_free (ndir);
		g_free (path);
		return NULL;
	}

	dd = g_new0 (DirData, 1);
	dd->name = ndir;
	dd->doc = doc;

	CORBA_exception_init (&ev);

	for (n = n->childs; n; n = n->next) {

		if (n->type == XML_ELEMENT_NODE && 
		    !strcmp (n->name, "entry") &&
		    (name = xmlGetProp(n, "name"))) {
			
			de = dir_lookup_entry (dd, name, TRUE);
			de->node = n;
			
			/* we only decode it if it is a single value, 
			 * multiple (localized) values are decoded in the
			 * get_value function */
			if (!(n->childs && n->childs->next))
				de->value = bonobo_config_xml_decode_any 
					((BonoboUINode *)n, NULL, &ev);
			
			xmlFree (name);
		}

	}

	CORBA_exception_free (&ev);

	dtree_add_dir (cache->dtree, dd->name);

	return dd;
}

/**
 * writes the contents of #dd back to the file system (atomic save).
 **/
static gboolean
dcache_save (DirCache *cache, DirData *dd)
{
	char *path;
	char *tmp_name;
	gboolean rval = FALSE;

	path = dir_to_path (cache->basedir, dd->name);
	tmp_name = g_strdup_printf ("%s.tmp.%d\n", path, getpid ());

	if (xmlSaveFile(tmp_name, dd->doc) >= 0 && 
	    rename (tmp_name, path) >= 0) {
		rval = TRUE;
		dd->dirty = FALSE;
	}

	g_free (path);
	g_free (tmp_name);
	
	return rval;
}

/**
 * free a DirEntry structure.
 **/
static void
dcache_free_entry (DirEntry *de)
{
	CORBA_free (de->value);

	if (de->node) {
		xmlUnlinkNode (de->node);
		xmlFreeNode (de->node);
	}
	
	g_free (de->name);
	g_free (de);
}

/**
 * free a whole DirData structure.
 **/
static void
dcache_free_dir (DirData *dd)
{
	GSList *l;

	for (l = dd->entries; l; l = l->next)
		dcache_free_entry ((DirEntry *)l->data);

	g_slist_free (dd->entries);

	g_free (dd->name);

	xmlFreeDoc (dd->doc);	

	g_free (dd);
}

/**
 * free the whole cache.
 **/
static void
dcache_free_all (DirCache *cache)
{
	int i;

	for (i = 0; i < CSIZE; i++) {
		if (cache->data [i]) 
			dcache_free_dir (cache->data [i]);
		cache->data [i] = NULL;
	}
}

/**
 * return the DirData for directory #dir. We use a direct mapped cache to speed
 * up things. It creates a new directory if #create is TRUE and the directory
 * dose not already exist.
 **/
static DirData *
dcache_lookup_dir (DirCache *cache, const char *dir, gboolean create)
{
	guint pos;
	DirData *dd, *odd;

	pos = g_str_hash (dir) % CSIZE;

	if ((dd = cache->data [pos]) && !strcmp (dd->name, dir))
		return dd;

	if (!(dd = dcache_load_dir (cache, dir, create)))
		return NULL;
	
	if ((odd = cache->data [pos])) {
		if (odd->dirty)
			dcache_save (cache, odd);

		dcache_free_dir (odd);
	}

	cache->data [pos] = dd;
	
	return dd;
}

/**
 * removes a directory from the cache. All changed data will be lost.
 **/
static void
dcache_remove_dir (DirCache *cache, const char *dir)
{
	guint pos;
	DirData *dd;

	pos = g_str_hash (dir) % CSIZE;

	if (!(dd = cache->data [pos]) || strcmp (dd->name, dir))
		return;

	dcache_free_dir (dd);

	cache->data [pos] = NULL;
}

/**
 * return the DirEntry for #key. If #key does not exist it will be created if
 * #create is TRUE.
 **/
static DirEntry *
dcache_lookup_entry (DirCache   *cache,
		     const char *key, 
		     gboolean    create)
{
	char *dir_name;
	char *leaf_name;
	DirEntry *de;
	DirData  *dd;

	if (!(dir_name = bonobo_config_dir_name (key)))
		dir_name = g_strdup ("");

	dd = dcache_lookup_dir (cache, dir_name, create);

	g_free (dir_name);

	if (!dd)
		return NULL;

	if (!(leaf_name = bonobo_config_leaf_name (key)))
		return NULL;

	de = dir_lookup_entry (dd, leaf_name, create);

	g_free (leaf_name);

	return de;
}

static CORBA_any *
real_get_value (BonoboConfigDatabase *db,
		const CORBA_char     *key, 
		const CORBA_char     *locale, 
		CORBA_Environment    *ev)
{
	BonoboConfigDIRDB *dirdb = BONOBO_CONFIG_DIRDB (db);
	DirEntry          *de;
	CORBA_any         *value = NULL;
	char              *ckey;

	if (!(ckey = normalize_key (key)) ||
	    !(de = dcache_lookup_entry (dirdb->priv, ckey, FALSE))) {
		bonobo_exception_set (ev, ex_Bonobo_ConfigDatabase_NotFound);
		g_free (ckey);
		return NULL;
	}

	g_free (ckey);

	if (de->value)
		return bonobo_arg_copy (de->value);
	
	/* localized value */
	if (de->node && de->node->childs && de->node->childs->next)	
		value = bonobo_config_xml_decode_any ((BonoboUINode *)de->node,
						      locale, ev);

	if (!value)
		bonobo_exception_set (ev, ex_Bonobo_ConfigDatabase_NotFound);
	
	return value;
}

static void
real_sync (BonoboConfigDatabase *db, 
	   CORBA_Environment    *ev)
{
	BonoboConfigDIRDB *dirdb = BONOBO_CONFIG_DIRDB (db);
	DirData *dd;
	int i;

	for (i = 0; i < CSIZE; i++) {
		if ((dd = dirdb->priv->data [i]) && dd->dirty) 
			dcache_save (dirdb->priv, dd);
	}
}

static gint
timeout_cb (gpointer data)
{
	BonoboConfigDIRDB *dirdb = BONOBO_CONFIG_DIRDB (data);
	CORBA_Environment ev;

	CORBA_exception_init(&ev);

	real_sync (BONOBO_CONFIG_DATABASE (data), &ev);
	
	CORBA_exception_free (&ev);

	dirdb->priv->time_id = 0;

	/* remove the timeout */
	return 0;
}

static void
notify_listeners (BonoboConfigDIRDB *dirdb, 
		  const char        *key, 
		  const CORBA_any   *value)
{
	CORBA_Environment ev;
	char *dir_name;
	char *leaf_name;
	char *ename;

	if (!key)
		return;

	CORBA_exception_init(&ev);

	ename = g_strconcat ("Bonobo/Property:change:", key, NULL);

	bonobo_event_source_notify_listeners(dirdb->priv->es, ename, value, 
					     &ev);

	g_free (ename);
	
	if (!(dir_name = bonobo_config_dir_name (key)))
		dir_name = g_strdup ("");

	if (!(leaf_name = bonobo_config_leaf_name (key)))
		leaf_name = g_strdup ("");
	
	ename = g_strconcat ("Bonobo/ConfigDatabase:change", dir_name, ":", 
			     leaf_name, NULL);

	bonobo_event_source_notify_listeners (dirdb->priv->es, ename, value, 
					      &ev);
						   
	CORBA_exception_free (&ev);

	g_free (ename);
	g_free (dir_name);
	g_free (leaf_name);
}

static void
real_set_value (BonoboConfigDatabase *db,
		const CORBA_char     *key, 
		const CORBA_any      *value,
		CORBA_Environment    *ev)
{
	BonoboConfigDIRDB *dirdb = BONOBO_CONFIG_DIRDB (db);
	xmlNodePtr n;
	DirEntry *de;
	char *name, *ckey;

	if (!(ckey = normalize_key (key)) ||
	    !(de = dcache_lookup_entry (dirdb->priv, ckey, TRUE))) {
		bonobo_exception_set (ev, ex_Bonobo_ConfigDatabase_NotFound);
		g_free (ckey);
		return;
	}

	CORBA_free (de->value);

	de->value = bonobo_arg_copy (value);

	if (de->node) {
		xmlUnlinkNode (de->node);
		xmlFreeNode (de->node);
	}
		
	name =  bonobo_config_leaf_name (ckey);

	de->node = (xmlNodePtr) bonobo_config_xml_encode_any (value, name, ev);
	
	g_free (name);
       
	n = de->dir->doc->root->childs;
	bonobo_ui_node_add_child ((BonoboUINode *)n, (BonoboUINode *)de->node);

	de->dir->dirty = TRUE;

	notify_listeners (dirdb, ckey, value);

	g_free (ckey);

	if (!dirdb->priv->time_id)
		dirdb->priv->time_id = gtk_timeout_add (FLUSH_INTERVAL * 1000, 
		        (GtkFunction)timeout_cb, dirdb);
}

static Bonobo_KeyList *
real_list_dirs (BonoboConfigDatabase *db,
		const CORBA_char     *dir,
		CORBA_Environment    *ev)
{
	BonoboConfigDIRDB *dirdb = BONOBO_CONFIG_DIRDB (db);
	Bonobo_KeyList *key_list;
	char *ndir;
	GNode *dn, *n;
	int len = 0;

	if (!(ndir = normalize_key (dir)) || 
	    !(dn = dtree_lookup_dir (dirdb->priv->dtree, ndir))) {
		bonobo_exception_set (ev, ex_Bonobo_ConfigDatabase_NotFound);
		return NULL;
	}
	
	g_free (ndir);

	key_list = Bonobo_KeyList__alloc ();

	if (!dn->children)
		return key_list;

	for (n = dn->children; n; n = n->next)
		len++;

	key_list->_buffer = CORBA_sequence_CORBA_string_allocbuf (len);
	CORBA_sequence_set_release (key_list, TRUE); 

	for (n = dn->children; n; n = n->next) {
		key_list->_buffer [key_list->_length] = 
			CORBA_string_dup (n->data);
		key_list->_length++;
	}
	
	return key_list;
}

static Bonobo_KeyList *
real_list_keys (BonoboConfigDatabase *db,
		const CORBA_char     *dir,
		CORBA_Environment    *ev)
{
	BonoboConfigDIRDB *dirdb = BONOBO_CONFIG_DIRDB (db);
	Bonobo_KeyList *key_list;
	DirData *dd;
	char *ndir;
	DirEntry *de;
	GSList *l;
	int len;

	if (!(ndir = normalize_key (dir)) ||
	    !(dd = dcache_lookup_dir (dirdb->priv, ndir, FALSE))) {
		g_free (ndir);
		bonobo_exception_set (ev, ex_Bonobo_ConfigDatabase_NotFound);
		return NULL;
	}

	g_free (ndir);
	
	key_list = Bonobo_KeyList__alloc ();

	if (!(len = g_slist_length (dd->entries)))
		return key_list;

	key_list->_buffer = CORBA_sequence_CORBA_string_allocbuf (len);
	CORBA_sequence_set_release (key_list, TRUE); 
	
	for (l = dd->entries; l ; l = l->next) {
		de = (DirEntry *)l->data;
		key_list->_buffer [key_list->_length] = 
			CORBA_string_dup (de->name);
		key_list->_length++;
	}
	
	return key_list;
}

static CORBA_boolean
real_dir_exists (BonoboConfigDatabase *db,
		 const CORBA_char     *dir,
		 CORBA_Environment    *ev)
{
	BonoboConfigDIRDB *dirdb = BONOBO_CONFIG_DIRDB (db);
	char *ndir;
	GNode *n;
	CORBA_boolean rval = FALSE;

	if ((ndir = normalize_key (dir)) &&
	    (n = dtree_lookup_dir (dirdb->priv->dtree, ndir)))
		rval = TRUE;

	g_free (ndir);
	return rval;
}

static void
real_remove_value (BonoboConfigDatabase *db,
		   const CORBA_char     *key, 
		   CORBA_Environment    *ev)
{
	BonoboConfigDIRDB *dirdb = BONOBO_CONFIG_DIRDB (db);
	DirEntry *de;
	char *ckey;

	if (!(ckey = normalize_key (key)) || 
	    !(de = dcache_lookup_entry (dirdb->priv, ckey, FALSE))) {
		g_free (ckey);
		return;
	}

	g_free (ckey);
	
	de->dir->entries = g_slist_remove (de->dir->entries, de);

	de->dir->dirty = TRUE;

	dcache_free_entry (de);

	return;
}

static void
real_remove_dir (BonoboConfigDatabase *db,
		 const CORBA_char     *dir, 
		 CORBA_Environment    *ev)
{
	BonoboConfigDIRDB *dirdb = BONOBO_CONFIG_DIRDB (db);
	char *path, *ndir;

	if ((ndir = normalize_key (dir))) {
		dcache_remove_dir (dirdb->priv, ndir);
		dtree_remove_dir (dirdb->priv->dtree, ndir);

		path = dir_to_path (dirdb->priv->basedir, ndir);

		unlink (path);

		g_free (path);
		g_free (ndir);
	}
}

static void
bonobo_config_dirdb_destroy (GtkObject *object)
{
	BonoboConfigDIRDB     *dirdb = BONOBO_CONFIG_DIRDB (object);
	BonoboConfigDIRDBPriv *priv = dirdb->priv;
	CORBA_Environment      ev;

	if (!priv)
		return;

	dirdb->priv = NULL;

	CORBA_exception_init (&ev);

	bonobo_url_unregister ("BONOBO_CONF:XLMDIRDB", priv->basedir, &ev);
      
	CORBA_exception_free (&ev);

	dcache_free_all (priv);

	dtree_destroy (priv->dtree);

	g_free (priv->basedir);

	g_free (priv);

	parent_class->destroy (object);
}


static void
bonobo_config_dirdb_class_init (BonoboConfigDatabaseClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	BonoboConfigDatabaseClass *cd_class;

	parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = bonobo_config_dirdb_destroy;

	cd_class = BONOBO_CONFIG_DATABASE_CLASS (class);
		
	cd_class->get_value    = real_get_value;
	cd_class->set_value    = real_set_value;
	cd_class->list_dirs    = real_list_dirs;
	cd_class->list_keys    = real_list_keys;
	cd_class->dir_exists   = real_dir_exists;
	cd_class->remove_value = real_remove_value;
	cd_class->remove_dir   = real_remove_dir;
	cd_class->sync         = real_sync;
}

static void
bonobo_config_dirdb_init (BonoboConfigDIRDB *dirdb)
{
	dirdb->priv = g_new0 (BonoboConfigDIRDBPriv, 1);
}

BONOBO_X_TYPE_FUNC (BonoboConfigDIRDB, PARENT_TYPE, bonobo_config_dirdb);

Bonobo_ConfigDatabase
bonobo_config_dirdb_new (const char *basedir)
{
	BonoboConfigDIRDB *dirdb;
	Bonobo_ConfigDatabase db;
	struct stat fs;
	CORBA_Environment ev;
	char *real_name;
	int l;

	g_return_val_if_fail (basedir != NULL, NULL);


	if (basedir [0] == '~' && basedir [1] == '/')
		real_name = g_strconcat (g_get_home_dir (), &basedir [1], 
					 NULL);
	else
		real_name = g_strdup (basedir);

	while ((l = strlen (real_name)) && real_name [l] == '/')
	       real_name [l] = '\0';
		
	CORBA_exception_init (&ev);

	db = bonobo_url_lookup ("BONOBO_CONF:XLMDIRDB", real_name, &ev);

	if (BONOBO_EX (&ev)) {
		CORBA_exception_init (&ev);
		db = CORBA_OBJECT_NIL;
	}

	if (db) {
		g_free (real_name);
		CORBA_exception_free (&ev);
		return bonobo_object_dup_ref (db, NULL);
	}

	mkdir (real_name, 0755);

	if (stat (real_name, &fs) || !S_ISDIR (fs.st_mode)) {
		g_free (real_name);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	if (!(dirdb = gtk_type_new (BONOBO_CONFIG_DIRDB_TYPE))) {
		g_free (real_name);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	dirdb->priv->basedir = real_name;

	dirdb->priv->dtree = dtree_create (real_name);
	
	if (fs.st_uid == getuid () && (fs.st_mode & S_IWUSR))
		BONOBO_CONFIG_DATABASE (dirdb)->writeable = TRUE;
	else 
		BONOBO_CONFIG_DATABASE (dirdb)->writeable = FALSE;
		       
	dirdb->priv->es = bonobo_event_source_new ();

	bonobo_object_add_interface (BONOBO_OBJECT (dirdb), 
				     BONOBO_OBJECT (dirdb->priv->es));

	db = CORBA_Object_duplicate (BONOBO_OBJREF (dirdb), NULL);

	bonobo_url_register ("BONOBO_CONF:XLMDIRDB", real_name, NULL, db, &ev);

	CORBA_exception_free (&ev);

	return db;
}
