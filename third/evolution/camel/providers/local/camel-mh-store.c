/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2000 Ximian, Inc.
 *
 * Authors: Michael Zucchi <notzed@ximian.com>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include "camel-mh-store.h"
#include "camel-mh-folder.h"
#include "camel-exception.h"
#include "camel-url.h"

static CamelLocalStoreClass *parent_class = NULL;

/* Returns the class for a CamelMhStore */
#define CMHS_CLASS(so) CAMEL_MH_STORE_CLASS (CAMEL_OBJECT_GET_CLASS(so))
#define CF_CLASS(so) CAMEL_FOLDER_CLASS (CAMEL_OBJECT_GET_CLASS(so))
#define CMHF_CLASS(so) CAMEL_MH_FOLDER_CLASS (CAMEL_OBJECT_GET_CLASS(so))

static CamelFolder *get_folder(CamelStore * store, const char *folder_name, guint32 flags, CamelException * ex);
static CamelFolder *get_inbox (CamelStore *store, CamelException *ex);
static void delete_folder(CamelStore * store, const char *folder_name, CamelException * ex);
static CamelFolderInfo * get_folder_info (CamelStore *store, const char *top, guint32 flags, CamelException *ex);

static void camel_mh_store_class_init(CamelObjectClass * camel_mh_store_class)
{
	CamelStoreClass *camel_store_class = CAMEL_STORE_CLASS(camel_mh_store_class);
	/*CamelServiceClass *camel_service_class = CAMEL_SERVICE_CLASS(camel_mh_store_class);*/

	parent_class = (CamelLocalStoreClass *)camel_type_get_global_classfuncs(camel_local_store_get_type());

	/* virtual method overload, use defaults for most */
	camel_store_class->get_folder = get_folder;
	camel_store_class->get_inbox = get_inbox;
	camel_store_class->delete_folder = delete_folder;
	camel_store_class->get_folder_info = get_folder_info;
}

CamelType camel_mh_store_get_type(void)
{
	static CamelType camel_mh_store_type = CAMEL_INVALID_TYPE;

	if (camel_mh_store_type == CAMEL_INVALID_TYPE) {
		camel_mh_store_type = camel_type_register(CAMEL_LOCAL_STORE_TYPE, "CamelMhStore",
							  sizeof(CamelMhStore),
							  sizeof(CamelMhStoreClass),
							  (CamelObjectClassInitFunc) camel_mh_store_class_init,
							  NULL,
							  NULL,
							  NULL);
	}

	return camel_mh_store_type;
}

static CamelFolder *get_folder(CamelStore * store, const char *folder_name, guint32 flags, CamelException * ex)
{
	char *name;
	struct stat st;

	(void) ((CamelStoreClass *)parent_class)->get_folder(store, folder_name, flags, ex);
	if (camel_exception_is_set(ex))
		return NULL;

	name = g_strdup_printf("%s%s", CAMEL_LOCAL_STORE(store)->toplevel_dir, folder_name);

	if (stat(name, &st) == -1) {
		if (errno != ENOENT) {
			camel_exception_setv(ex, CAMEL_EXCEPTION_SYSTEM,
					     _("Could not open folder `%s':\n%s"),
					     folder_name, g_strerror(errno));
			g_free (name);
			return NULL;
		}
		if ((flags & CAMEL_STORE_FOLDER_CREATE) == 0) {
			camel_exception_setv(ex, CAMEL_EXCEPTION_STORE_NO_FOLDER,
					     _("Folder `%s' does not exist."), folder_name);
			g_free (name);
			return NULL;
		}
		if (mkdir(name, 0700) != 0) {
			camel_exception_setv(ex, CAMEL_EXCEPTION_SYSTEM,
					     _("Could not create folder `%s':\n%s"),
					     folder_name, g_strerror(errno));
			g_free (name);
			return NULL;
		}
	} else if (!S_ISDIR(st.st_mode)) {
		camel_exception_setv(ex, CAMEL_EXCEPTION_STORE_NO_FOLDER,
				     _("`%s' is not a directory."), name);
		g_free (name);
		return NULL;
	}
	g_free(name);

	return camel_mh_folder_new(store, folder_name, flags, ex);
}

static CamelFolder *
get_inbox (CamelStore *store, CamelException *ex)
{
	return get_folder (store, "inbox", 0, ex);
}

static void delete_folder(CamelStore * store, const char *folder_name, CamelException * ex)
{
	char *name;

	/* remove folder directory - will fail if not empty */
	name = g_strdup_printf("%s%s", CAMEL_LOCAL_STORE(store)->toplevel_dir, folder_name);
	if (rmdir(name) == -1) {
		camel_exception_setv(ex, CAMEL_EXCEPTION_SYSTEM,
				     _("Could not delete folder `%s': %s"),
				     folder_name, strerror(errno));
		g_free(name);
		return;
	}
	g_free(name);

	/* and remove metadata */
	((CamelStoreClass *)parent_class)->delete_folder(store, folder_name, ex);
}

/* Add a folder if we haven't already seen it and if it exists as a
   directory. */
static void check_and_add(GPtrArray *folders, GHashTable *seen,
			  const char *root, const char *path)
{
	char *fullpath, *pathcopy;
	struct stat st;
	CamelFolderInfo *fi;
	const char *base;

	if (g_hash_table_lookup(seen, path) != NULL)
		return;

	fullpath = g_strdup_printf("%s%s", root, path);
	if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) {
		/* Make sure we don't add this folder twice. */
		pathcopy = g_strdup(path);
		g_hash_table_insert(seen, pathcopy, pathcopy);

		/* Get the last path component for the folder name. */
		base = strrchr(path, '/');
		base = (base == NULL) ? path : base + 1;

		/* Build the folder info structure. */
		fi = g_malloc(sizeof(*fi));
		fi->parent = NULL;
		fi->sibling = NULL;
		fi->child = NULL;
		fi->url = g_strdup_printf("mh:%s#%s", root, path);
		fi->full_name = g_strdup(path);
		fi->name = g_strdup(base);
		fi->unread_message_count = 0;
		camel_folder_info_build_path(fi, '/');

		g_ptr_array_add(folders, fi);
	}
	g_free(fullpath);
}

static void free_str(gpointer key, gpointer value, gpointer user_data)
{
	g_free(key);
}

static CamelFolderInfo *
get_folder_info (CamelStore *store, const char *top, guint32 flags, CamelException *ex)
{
	GPtrArray *folders;
	GHashTable *seen;
	CamelFolderInfo *tree;
	char *root, *name, buf[1024];
	FILE *fp;
	DIR *dp;
	struct dirent *d;
	int len;

	root = CAMEL_LOCAL_STORE(store)->toplevel_dir;
	folders = g_ptr_array_new();
	seen = g_hash_table_new(g_str_hash, g_str_equal);

	/* Read the MH folders file if present. */
	name = g_strdup_printf("%s.folders", root);
	fp = fopen(name, "r");
	if (fp != NULL) {
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			len = strlen(buf);
			/* Stop if we see a ridiculously long line. */
			if (buf[len - 1] != '\n')
				break;
			buf[len - 1] = 0;
			check_and_add(folders, seen, root, buf);
		}
		fclose(fp);
	}
	free(name);

	/* Also scan the top level directory. */
	dp = opendir(root);
	if (dp) {
		while ((d = readdir(dp)) != NULL) {
			if (strcmp(d->d_name, ".") == 0
			    || strcmp(d->d_name, "..") == 0)
				continue;
			check_and_add(folders, seen, root, d->d_name);
		}
	}
	closedir(dp);

	g_hash_table_foreach(seen, free_str, NULL);
	g_hash_table_destroy(seen);

	tree = camel_folder_info_build(folders, NULL, '/', TRUE);
	g_ptr_array_free(folders, TRUE);
	return tree;
}
