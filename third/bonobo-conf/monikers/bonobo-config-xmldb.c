/**
 * bonobo-config-xmldb.c: xml configuration database implementation.
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#include <config.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <bonobo/bonobo-arg.h>
#include <bonobo/bonobo-property-bag-xml.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo-conf/bonobo-config-utils.h>
#include <gnome-xml/xmlmemory.h>
#include <gtk/gtkmain.h>

#include "bonobo-config-xmldb.h"

static GtkObjectClass *parent_class = NULL;

#define CLASS(o) BONOBO_CONFIG_XMLDB_CLASS (GTK_OBJECT(o)->klass)

#define PARENT_TYPE BONOBO_CONFIG_DATABASE_TYPE
#define FLUSH_INTERVAL 30 /* 30 seconds */

static DirEntry *
dir_lookup_entry (DirData  *dir,
		  char     *name,
		  gboolean  create)
{
	GSList *l;
	DirEntry *de;
	
	l = dir->entries;

	while (l) {
		de = (DirEntry *)l->data;

		if (!strcmp (de->name, name))
			return de;
		
		l = l->next;
	}

	if (create) {

		de = g_new0 (DirEntry, 1);
		
		de->dir = dir;

		de->name = g_strdup (name);

		dir->entries = g_slist_prepend (dir->entries, de);

		return de;
	}

	return NULL;
}

static DirData *
dir_lookup_subdir (DirData  *dir,
		   char     *name,
		   gboolean  create)
{
	GSList *l;
	DirData *dd;
	
	l = dir->subdirs;

	while (l) {
		dd = (DirData *)l->data;

		if (!strcmp (dd->name, name))
			return dd;
		
		l = l->next;
	}

	if (create) {

		dd = g_new0 (DirData, 1);

		dd->dir = dir;

		dd->name = g_strdup (name);

		dir->subdirs = g_slist_prepend (dir->subdirs, dd);

		return dd;
	}

	return NULL;
}

static DirData *
lookup_dir (DirData    *dir,
	    const char *path,
	    gboolean    create)
{
	const char *s, *e;
	char *name;
	DirData  *dd = dir;
	
	s = path;
	while (*s == '/') s++;
	
	if (*s == '\0')
		return dir;

	if ((e = strchr (s, '/')))
		name = g_strndup (s, e - s);
	else
		name = g_strdup (s);
	
	if ((dd = dir_lookup_subdir (dd, name, create))) {
		if (e)
			dd = lookup_dir (dd, e, create);

		g_free (name);
		return dd;
		
	}

	return NULL;

}

static DirEntry *
lookup_dir_entry (BonoboConfigXMLDB *xmldb,
		  const char        *key, 
		  gboolean           create)
{
	char *dir_name;
	char *leaf_name;
	DirEntry *de;
	DirData  *dd;

	if ((dir_name = bonobo_config_dir_name (key))) {
		dd = lookup_dir (xmldb->dir, dir_name, create);
		
		if (dd && !dd->node) {
			dd->node = xmlNewChild (xmldb->doc->root, NULL, 
						"section", NULL);
		
			xmlSetProp (dd->node, "path", dir_name);
		}	

		g_free (dir_name);

	} else {
		dd = xmldb->dir;

		if (!dd->node) 
			dd->node = xmlNewChild (xmldb->doc->root, NULL, 
						"section", NULL);
	}

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
	BonoboConfigXMLDB *xmldb = BONOBO_CONFIG_XMLDB (db);
	DirEntry          *de;
	CORBA_any         *value = NULL;

	de = lookup_dir_entry (xmldb, key, FALSE);
	if (!de) {
		bonobo_exception_set (ev, ex_Bonobo_ConfigDatabase_NotFound);
		return NULL;
	}

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


static void xmlNodeDump (xmlBufferPtr buf, xmlDocPtr doc, xmlNodePtr cur, int level, int format);


static void
xmlAttrDump (xmlBufferPtr buf, xmlDocPtr doc, xmlAttrPtr cur)
{
	xmlChar *value;
	
	if (cur == NULL) {
#ifdef DEBUG_TREE
		fprintf(stderr, "xmlAttrDump : property == NULL\n");
#endif
		return;
	}
	
	xmlBufferWriteChar (buf, " ");
	if ((cur->ns != NULL) && (cur->ns->prefix != NULL)) {
		xmlBufferWriteCHAR (buf, cur->ns->prefix);
		xmlBufferWriteChar (buf, ":");
	}
	
	xmlBufferWriteCHAR (buf, cur->name);
	value = xmlNodeListGetString (doc, cur->val, 0);
	if (value) {
		xmlBufferWriteChar (buf, "=");
		xmlBufferWriteQuotedString (buf, value);
		xmlFree (value);
	} else  {
		xmlBufferWriteChar (buf, "=\"\"");
	}
}

static void
xmlAttrListDump (xmlBufferPtr buf, xmlDocPtr doc, xmlAttrPtr cur)
{
	if (cur == NULL) {
#ifdef DEBUG_TREE
		fprintf(stderr, "xmlAttrListDump : property == NULL\n");
#endif
		return;
	}
	
	while (cur != NULL) {
		xmlAttrDump (buf, doc, cur);
		cur = cur->next;
	}
}

static void
xmlNodeListDump (xmlBufferPtr buf, xmlDocPtr doc, xmlNodePtr cur, int level, int format)
{
	int i;
	
	if (cur == NULL) {
#ifdef DEBUG_TREE
		fprintf(stderr, "xmlNodeListDump : node == NULL\n");
#endif
		return;
	}
	
	while (cur != NULL) {
		if ((format) && (xmlIndentTreeOutput) &&
		    (cur->type == XML_ELEMENT_NODE))
			for (i = 0; i < level; i++)
				xmlBufferWriteChar (buf, "  ");
		xmlNodeDump (buf, doc, cur, level, format);
		if (format) {
			xmlBufferWriteChar (buf, "\n");
		}
		cur = cur->next;
	}
}

static void
xmlNodeDump (xmlBufferPtr buf, xmlDocPtr doc, xmlNodePtr cur, int level, int format)
{
	int i;
	xmlNodePtr tmp;
	
	if (cur == NULL) {
#ifdef DEBUG_TREE
		fprintf(stderr, "xmlNodeDump : node == NULL\n");
#endif
		return;
	}
	
	if (cur->type == XML_TEXT_NODE) {
		if (cur->content != NULL) {
			xmlChar *buffer;
			
#ifndef XML_USE_BUFFER_CONTENT
			buffer = xmlEncodeEntitiesReentrant (doc, cur->content);
#else
			buffer = xmlEncodeEntitiesReentrant (doc, xmlBufferContent (cur->content));
#endif
			if (buffer != NULL) {
				xmlBufferWriteCHAR (buf, buffer);
				xmlFree (buffer);
			}
		}
		return;
	}
	
	if (cur->type == XML_PI_NODE) {
		if (cur->content != NULL) {
			xmlBufferWriteChar (buf, "<?");
			xmlBufferWriteCHAR (buf, cur->name);
			if (cur->content != NULL) {
				xmlBufferWriteChar (buf, " ");
#ifndef XML_USE_BUFFER_CONTENT
				xmlBufferWriteCHAR (buf, cur->content);
#else
				xmlBufferWriteCHAR (buf, xmlBufferContent (cur->content));
#endif
			}
			xmlBufferWriteChar (buf, "?>");
		}
		return;
	}
	
	if (cur->type == XML_COMMENT_NODE) {
		if (cur->content != NULL) {
			xmlBufferWriteChar (buf, "<!--");
#ifndef XML_USE_BUFFER_CONTENT
			xmlBufferWriteCHAR (buf, cur->content);
#else
			xmlBufferWriteCHAR (buf, xmlBufferContent (cur->content));
#endif
			xmlBufferWriteChar (buf, "-->");
		}
		return;
	}
	
	if (cur->type == XML_ENTITY_REF_NODE) {
		xmlBufferWriteChar (buf, "&");
		xmlBufferWriteCHAR (buf, cur->name);
		xmlBufferWriteChar (buf, ";");
		return;
	}
	
	if (cur->type == XML_CDATA_SECTION_NODE) {
		xmlBufferWriteChar (buf, "<![CDATA[");
		if (cur->content != NULL)
#ifndef XML_USE_BUFFER_CONTENT
			xmlBufferWriteCHAR (buf, cur->content);
#else
		xmlBufferWriteCHAR (buf, xmlBufferContent(cur->content));
#endif
		xmlBufferWriteChar (buf, "]]>");
		return;
	}
	
	if (format == 1) {
		tmp = cur->childs;
		while (tmp != NULL) {
			if ((tmp->type == XML_TEXT_NODE) || 
			    (tmp->type == XML_ENTITY_REF_NODE)) {
				format = 0;
				break;
			}
			tmp = tmp->next;
		}
	}
	
	xmlBufferWriteChar (buf, "<");
	if ((cur->ns != NULL) && (cur->ns->prefix != NULL)) {
		xmlBufferWriteCHAR (buf, cur->ns->prefix);
		xmlBufferWriteChar (buf, ":");
	}
	
	xmlBufferWriteCHAR (buf, cur->name);
	
	if (cur->properties != NULL)
		xmlAttrListDump (buf, doc, cur->properties);
	
	if ((cur->content == NULL) && (cur->childs == NULL) &&
	    (!xmlSaveNoEmptyTags)) {
		xmlBufferWriteChar (buf, "/>");
		return;
	}
	
	xmlBufferWriteChar (buf, ">");
	if (cur->content != NULL) {
		xmlChar *buffer;
		
#ifndef XML_USE_BUFFER_CONTENT
		buffer = xmlEncodeEntitiesReentrant (doc, cur->content);
#else
		buffer = xmlEncodeEntitiesReentrant (doc, xmlBufferContent (cur->content));
#endif
		if (buffer != NULL) {
			xmlBufferWriteCHAR (buf, buffer);
			xmlFree (buffer);
		}
	}
	
	if (cur->childs != NULL) {
		if (format)
			xmlBufferWriteChar (buf, "\n");
		
		xmlNodeListDump (buf, doc, cur->childs, (level >= 0 ? level + 1 : -1), format);
		if ((xmlIndentTreeOutput) && (format))
			for (i = 0; i < level; i++)
				xmlBufferWriteChar (buf, "  ");
	}
	
	xmlBufferWriteChar (buf, "</");
	if ((cur->ns != NULL) && (cur->ns->prefix != NULL)) {
		xmlBufferWriteCHAR (buf, cur->ns->prefix);
		xmlBufferWriteChar (buf, ":");
	}
	
	xmlBufferWriteCHAR (buf, cur->name);
	xmlBufferWriteChar (buf, ">");
}

static void
xmlDocContentDump (xmlBufferPtr buf, xmlDocPtr cur)
{
	xmlBufferWriteChar (buf, "<?xml version=");
	
	if (cur->version != NULL)
		xmlBufferWriteQuotedString (buf, cur->version);
	else
		xmlBufferWriteChar (buf, "\"1.0\"");
	
	if ((cur->encoding != NULL) &&
	    (strcasecmp (cur->encoding, "UTF-8") != 0)) {
		xmlBufferWriteChar (buf, " encoding=");
		xmlBufferWriteQuotedString (buf, cur->encoding);
	}
	
	switch (cur->standalone) {
        case 1:
		xmlBufferWriteChar (buf, " standalone=\"yes\"");
		break;
	}
	
	xmlBufferWriteChar (buf, "?>\n");
	if (cur->root != NULL) {
		xmlNodePtr child = cur->root;
		
		while (child != NULL) {
			xmlNodeDump (buf, cur, child, 0, 1);
			xmlBufferWriteChar (buf, "\n");
			child = child->next;
		}
	}
}

static void
real_sync (BonoboConfigDatabase *db, 
	   CORBA_Environment *ev)
{
	BonoboConfigXMLDB *xmldb = BONOBO_CONFIG_XMLDB (db);
	size_t n, written = 0;
	xmlBufferPtr buf;
	char *tmp_name;
	ssize_t w;
	int fd;
	
	if (!db->writeable)
		return;
	
 retry_open:
	tmp_name = g_strdup_printf ("%s.tmp.%d.XXXXXX", xmldb->filename, getpid ());
	if (!mktemp (tmp_name)) {
		g_free (tmp_name);
		return;
	}
	
	fd = open (tmp_name, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (fd == -1) {
		g_free (tmp_name);
		if (errno == EEXIST)
			goto retry_open;
		
		return;
	}
	
	if (!(buf = xmlBufferCreate ())) {
		close (fd);
		unlink (tmp_name);
		g_free (tmp_name);
		return;
	}
	
	xmlDocContentDump (buf, xmldb->doc);
	
	n = buf->use;
	do {
		do {
			w = write (fd, buf->content + written, n - written);
		} while (w == -1 && errno == EINTR);
		
		if (w > 0)
			written += w;
	} while (w != -1 && written < n);
	
	xmlBufferFree (buf);
	
	if (written < n || fsync (fd) == -1) {
		close (fd);
		unlink (tmp_name);
		g_free (tmp_name);
		return;
	}
	
	close (fd);
	
	if (rename (tmp_name, xmldb->filename) < 0) {
		/* if we don't have permissions then we can assume the db is read-only */
		if (errno == EACCES || errno == EPERM)
			db->writeable = FALSE;
		
		unlink (tmp_name);
	}
	
	g_free (tmp_name);
}

static gint
timeout_cb (gpointer data)
{
	BonoboConfigXMLDB *xmldb = BONOBO_CONFIG_XMLDB (data);
	CORBA_Environment ev;

	CORBA_exception_init(&ev);

	real_sync (BONOBO_CONFIG_DATABASE (data), &ev);
	
	CORBA_exception_free (&ev);

	xmldb->time_id = 0;

	/* remove the timeout */
	return 0;
}

static void
notify_listeners (BonoboConfigXMLDB *xmldb, 
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

	bonobo_event_source_notify_listeners(xmldb->es, ename, value, &ev);

	g_free (ename);
	
	if (!(dir_name = bonobo_config_dir_name (key)))
		dir_name = g_strdup ("");

	if (!(leaf_name = bonobo_config_leaf_name (key)))
		leaf_name = g_strdup ("");
	
	ename = g_strconcat ("Bonobo/ConfigDatabase:change", dir_name, ":", 
			     leaf_name, NULL);

	bonobo_event_source_notify_listeners (xmldb->es, ename, value, &ev);
						   
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
	BonoboConfigXMLDB *xmldb = BONOBO_CONFIG_XMLDB (db);
	DirEntry *de;
	char *name;

	de = lookup_dir_entry (xmldb, key, TRUE);
	if (!de) {
		bonobo_exception_set (ev, ex_Bonobo_ConfigDatabase_NotFound);
		return;
	}

	if (de->value)
		CORBA_free (de->value);

	de->value = bonobo_arg_copy (value);

	if (de->node) {
		xmlUnlinkNode (de->node);
		xmlFreeNode (de->node);
	}
		
	name =  bonobo_config_leaf_name (key);

	de->node = (xmlNodePtr) bonobo_config_xml_encode_any (value, name, ev);
	
	g_free (name);
	
	bonobo_ui_node_add_child ((BonoboUINode *)de->dir->node, 
				  (BonoboUINode *)de->node);

	if (!xmldb->time_id)
		xmldb->time_id = gtk_timeout_add (FLUSH_INTERVAL * 1000, 
						  (GtkFunction)timeout_cb, 
						  xmldb);

	notify_listeners (xmldb, key, value);
}

static Bonobo_KeyList *
real_list_dirs (BonoboConfigDatabase *db,
		const CORBA_char     *dir,
		CORBA_Environment    *ev)
{
	BonoboConfigXMLDB *xmldb = BONOBO_CONFIG_XMLDB (db);
	Bonobo_KeyList *key_list;
	DirData *dd, *sub;
	GSList *l;
	int len;
	
	key_list = Bonobo_KeyList__alloc ();
	key_list->_length = 0;

	if (!(dd = lookup_dir (xmldb->dir, dir, FALSE)))
		return key_list;

	if (!(len = g_slist_length (dd->subdirs)))
		return key_list;

	key_list->_buffer = CORBA_sequence_CORBA_string_allocbuf (len);
	CORBA_sequence_set_release (key_list, TRUE); 
	
	for (l = dd->subdirs; l != NULL; l = l->next) {
		sub = (DirData *)l->data;
	       
		key_list->_buffer [key_list->_length] = 
			CORBA_string_dup (sub->name);
		key_list->_length++;
	}
	
	return key_list;
}

static Bonobo_KeyList *
real_list_keys (BonoboConfigDatabase *db,
		const CORBA_char     *dir,
		CORBA_Environment    *ev)
{
	BonoboConfigXMLDB *xmldb = BONOBO_CONFIG_XMLDB (db);
	Bonobo_KeyList *key_list;
	DirData *dd;
	DirEntry *de;
	GSList *l;
	int len;
	
	key_list = Bonobo_KeyList__alloc ();
	key_list->_length = 0;

	if (!(dd = lookup_dir (xmldb->dir, dir, FALSE)))
		return key_list;

	if (!(len = g_slist_length (dd->entries)))
		return key_list;

	key_list->_buffer = CORBA_sequence_CORBA_string_allocbuf (len);
	CORBA_sequence_set_release (key_list, TRUE); 
	
	for (l = dd->entries; l != NULL; l = l->next) {
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
	BonoboConfigXMLDB *xmldb = BONOBO_CONFIG_XMLDB (db);

	if (lookup_dir (xmldb->dir, dir, FALSE))
		return TRUE;

	return FALSE;
}

static void
delete_dir_entry (DirEntry *de)
{
	CORBA_free (de->value);

	if (de->node) {
		xmlUnlinkNode (de->node);
		xmlFreeNode (de->node);
	}
	
	g_free (de->name);
	g_free (de);
}

static void
real_remove_value (BonoboConfigDatabase *db,
		   const CORBA_char     *key, 
		   CORBA_Environment    *ev)
{
	BonoboConfigXMLDB *xmldb = BONOBO_CONFIG_XMLDB (db);
	DirEntry *de;

	if (!(de = lookup_dir_entry (xmldb, key, FALSE)))
		return;

	de->dir->entries = g_slist_remove (de->dir->entries, de);

	delete_dir_entry (de);
}

static void
delete_dir_data (DirData *dd, gboolean is_root)
{
	GSList *l;

	for (l = dd->subdirs; l; l = l->next)
		delete_dir_data ((DirData *)l->data, FALSE);

	g_slist_free (dd->subdirs);

	dd->subdirs = NULL;

	for (l = dd->entries; l; l = l->next) 
		delete_dir_entry ((DirEntry *)l->data);

	g_slist_free (dd->entries);

	dd->entries = NULL;

	if (!is_root) {
	
		g_free (dd->name);

		if (dd->node) {
			xmlUnlinkNode (dd->node);
			xmlFreeNode (dd->node);
		}
	
		g_free (dd);
	}
}

static void
real_remove_dir (BonoboConfigDatabase *db,
		 const CORBA_char     *dir, 
		 CORBA_Environment    *ev)
{
	BonoboConfigXMLDB *xmldb = BONOBO_CONFIG_XMLDB (db);
	DirData *dd;

	if (!(dd = lookup_dir (xmldb->dir, dir, FALSE)))
		return;

	if (dd != xmldb->dir && dd->dir)
		dd->dir->subdirs = g_slist_remove (dd->dir->subdirs, dd);

	delete_dir_data (dd, dd == xmldb->dir);
}

static void
bonobo_config_xmldb_destroy (GtkObject *object)
{
	BonoboConfigXMLDB *xmldb = BONOBO_CONFIG_XMLDB (object);
	CORBA_Environment      ev;

	CORBA_exception_init (&ev);

	bonobo_url_unregister ("BONOBO_CONF:XLMDB", xmldb->filename, &ev);
      
	CORBA_exception_free (&ev);

	if (xmldb->doc)
		xmlFreeDoc (xmldb->doc);
	xmldb->doc = NULL;

	g_free (xmldb->filename);
	xmldb->filename = NULL;

	parent_class->destroy (object);
}


static void
bonobo_config_xmldb_class_init (BonoboConfigDatabaseClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	BonoboConfigDatabaseClass *cd_class;

	parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = bonobo_config_xmldb_destroy;

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
bonobo_config_xmldb_init (BonoboConfigXMLDB *xmldb)
{
	xmldb->dir = g_new0 (DirData, 1);
}

BONOBO_X_TYPE_FUNC (BonoboConfigXMLDB, PARENT_TYPE, bonobo_config_xmldb);

static void
read_section (DirData *dd)
{
	CORBA_Environment  ev;
	DirEntry          *de;
	xmlNodePtr         s;
	gchar             *name;

	CORBA_exception_init (&ev);

	s = dd->node->childs;

	while (s) {
		if (s->type == XML_ELEMENT_NODE && 
		    !strcmp (s->name, "entry") &&
		    (name = xmlGetProp(s, "name"))) {
			
			de = dir_lookup_entry (dd, name, TRUE);
			
			de->node = s;
			
			/* we only decode it if it is a single value, 
			 * multiple (localized) values are decoded in the
			 * get_value function */
			if (!(s->childs && s->childs->next))
				de->value = bonobo_config_xml_decode_any 
					((BonoboUINode *)s, NULL, &ev);
			
			xmlFree (name);
		}
		s = s->next;
	}

	CORBA_exception_free (&ev);
}

static void
fill_cache (BonoboConfigXMLDB *xmldb)
{
	xmlNodePtr  n;
	gchar      *path;
	DirData    *dd;

	n = xmldb->doc->root->childs;

	while (n) {
		if (n->type == XML_ELEMENT_NODE && 
		    !strcmp (n->name, "section")) {

			path = xmlGetProp(n, "path");

			if (!path || !strcmp (path, "")) {
				dd = xmldb->dir;
			} else 
				dd = lookup_dir (xmldb->dir, path, TRUE);
				
			if (!dd->node)
				dd->node = n;

			read_section (dd);
			
			xmlFree (path);
		}
		n = n->next;
	}
}

Bonobo_ConfigDatabase
bonobo_config_xmldb_new (const char *filename)
{
	BonoboConfigXMLDB     *xmldb;
	Bonobo_ConfigDatabase  db;
	CORBA_Environment      ev;
	char                  *real_name;

	g_return_val_if_fail (filename != NULL, NULL);

	CORBA_exception_init (&ev);

	if (filename [0] == '~' && filename [1] == '/')
		real_name = g_strconcat (g_get_home_dir (), &filename [1], 
					 NULL);
	else
		real_name = g_strdup (filename);

	db = bonobo_url_lookup ("BONOBO_CONF:XLMDB", real_name, &ev);

	if (BONOBO_EX (&ev)) {
		CORBA_exception_init (&ev);
		db = CORBA_OBJECT_NIL;
	}

	if (db) {
		g_free (real_name);
		CORBA_exception_free (&ev);
		return bonobo_object_dup_ref (db, NULL);
	}

	if (!(xmldb = gtk_type_new (BONOBO_CONFIG_XMLDB_TYPE))) {
		g_free (real_name);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	xmldb->filename = real_name;
	
	BONOBO_CONFIG_DATABASE (xmldb)->writeable = TRUE;
		       
	xmldb->doc = xmlParseFile (xmldb->filename);

	if (!xmldb->doc)
		xmldb->doc = xmlNewDoc("1.0");
	
	if (xmldb->doc->root == NULL)
		xmldb->doc->root = xmlNewDocNode (xmldb->doc, NULL, 
						  "bonobo-config", NULL);

	if (strcmp (xmldb->doc->root->name, "bonobo-config")) {
		xmlFreeDoc (xmldb->doc);
		xmldb->doc = xmlNewDoc("1.0");
		xmldb->doc->root = xmlNewDocNode (xmldb->doc, NULL, 
						  "bonobo-config", NULL);
	}

	fill_cache (xmldb);

	xmldb->es = bonobo_event_source_new ();

	bonobo_object_add_interface (BONOBO_OBJECT (xmldb), 
				     BONOBO_OBJECT (xmldb->es));

	db = CORBA_Object_duplicate (BONOBO_OBJREF (xmldb), NULL);

	bonobo_url_register ("BONOBO_CONF:XLMDB", real_name, NULL, db, &ev);

	CORBA_exception_free (&ev);

	return db;
}
