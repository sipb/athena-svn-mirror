/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gconf-method.c - VFS Access to the GConf configuration database.

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Dave Camp <campd@oit.edu> */

/* FIXME bugzilla.eazel.com 1180: More error checking */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <string.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "gnome-vfs-module.h"

#include "file-method.h"

static GnomeVFSResult do_open           (GnomeVFSMethod *method,
				         GnomeVFSMethodHandle **method_handle,
				         GnomeVFSURI *uri,
				         GnomeVFSOpenMode mode,
				         GnomeVFSContext *context);
#if 0
static GnomeVFSResult do_create         (GnomeVFSMethod *method,
				         GnomeVFSMethodHandle **method_handle,
				         GnomeVFSURI *uri,
				         GnomeVFSOpenMode mode,
				         gboolean exclusive,
				         guint perm,
				         GnomeVFSContext *context);
#endif
static GnomeVFSResult do_close          (GnomeVFSMethod *method,
				         GnomeVFSMethodHandle *method_handle,
				         GnomeVFSContext *context);
#if 0
static GnomeVFSResult do_read           (GnomeVFSMethodHandle *method_handle,
				         gpointer buffer,
				         GnomeVFSFileSize num_bytes,
				         GnomeVFSFileSize *bytes_read);
static GnomeVFSResult do_write          (GnomeVFSMethodHandle *method_handle,
				         gconstpointer buffer,
				         GnomeVFSFileSize num_bytes,
				         GnomeVFSFileSize *bytes_written);
#endif
static GnomeVFSResult do_open_directory (GnomeVFSMethod *method,
					 GnomeVFSMethodHandle **method_handle,
					 GnomeVFSURI *uri,
					 GnomeVFSFileInfoOptions options,
					 const GnomeVFSDirectoryFilter *filter,
					 GnomeVFSContext *context);
static GnomeVFSResult do_close_directory(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 GnomeVFSContext *context);
static GnomeVFSResult do_read_directory (GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 GnomeVFSFileInfo *file_info,
					 GnomeVFSContext *context);
static GnomeVFSResult do_get_file_info  (GnomeVFSMethod *method,
					 GnomeVFSURI *uri,
					 GnomeVFSFileInfo *file_info,
					 GnomeVFSFileInfoOptions options,
					 GnomeVFSContext *context);
#if 0
static GnomeVFSResult do_get_file_info_from_handle
                                        (GnomeVFSMethodHandle *method_handle,
					 GnomeVFSFileInfo *file_info,
					 GnomeVFSFileInfoOptions options);
#endif
static gboolean       do_is_local       (GnomeVFSMethod *method,
					 const GnomeVFSURI *uri);

static GnomeVFSMethod method = {
        do_open,
        NULL, /* create */
        do_close,
        NULL, /* read */
        NULL, /* write */
        NULL, /* seek */
        NULL, /* tell */
        NULL, /* truncate */
        do_open_directory,
	do_close_directory,
        do_read_directory,
        do_get_file_info,
	NULL, /* get_file_info_from_handle */
        do_is_local,
	NULL, /* make directory */
        NULL, /* remove directory */
	NULL, /* unlink */
	NULL, /* check_same_fs */
	NULL, /* set_file_info */
	NULL, /* truncate */
	NULL, /* find_directory */
	NULL /* create_symbolic_link */
};

static GConfClient *client = NULL;

#ifdef G_THREADS_ENABLED 
static GMutex *client_mutex;
#endif


/* This is to make sure the path starts with `/', so that at least we
 * get a predictable behavior when the leading `/' is not present.  
 * Also make sure there is no trailing '/', as gconf doesn't like trailing 
 * slashes.
 */
#define MAKE_ABSOLUTE(dest, src)                        \
G_STMT_START{                                           \
        if ((src)[0] != '/') {                          \
                (dest) = alloca (strlen (src) + 2);     \
                (dest)[0] = '/';                        \
                strcpy ((dest), (src));                 \
	} else {                                        \
                (dest) = (src);                         \
        }                                               \
       if (strlen(dest) > 1 && dest[strlen(dest) - 1] == '/') \
                dest[strlen(dest) - 1] = '\0'; \
}G_STMT_END

#ifdef G_THREADS_ENABLED
#define MUTEX_LOCK(a)   if ((a) != NULL) g_mutex_lock (a)
#define MUTEX_UNLOCK(a) if ((a) != NULL) g_mutex_unlock (a)
#else
#define MUTEX_LOCK(a)
#define MUTEX_UNLOCK(a)
#endif

typedef struct {
        GnomeVFSURI *uri;
        GnomeVFSFileInfoOptions options;
        const GnomeVFSDirectoryFilter *filter;

        GSList *subdirs;
        GSList *pairs;
	
	GMutex *mutex;
} DirectoryHandle;

static DirectoryHandle *
directory_handle_new (GnomeVFSURI *uri,
                      GnomeVFSFileInfoOptions options,
                      const GnomeVFSDirectoryFilter *filter,
                      GSList *subdirs,
                      GSList *pairs)
{
        DirectoryHandle *retval;
        
        retval = g_new (DirectoryHandle, 1);
        
        retval->uri = gnome_vfs_uri_ref (uri);
        retval->options = options;
        retval->filter = filter;
        retval->pairs = pairs;
        retval->subdirs = subdirs;
#ifdef G_THREADS_ENABLED
        if (g_thread_supported ())
		retval->mutex = g_mutex_new ();
#endif

        return retval;
}

static void
directory_handle_destroy (DirectoryHandle *handle) 
{
        /* FIXME bugzilla.eazel.com 1178: Free unused pairs */
        gnome_vfs_uri_unref (handle->uri);

#ifdef G_THREADS_ENABLED
        if (g_thread_supported ()) {
		g_mutex_free (handle->mutex);
	}
#endif
        g_free (handle);
}

static GnomeVFSResult 
set_mime_type_value (GnomeVFSFileInfo *info,
                     const GConfValue *value,
                     GnomeVFSFileInfoOptions options)
{
        const gchar *mime_type;
        
        switch (value->type) {
        case GCONF_VALUE_INVALID :
                mime_type = "application/x-gconf-invalid";
                break;
        case GCONF_VALUE_STRING :
                mime_type = "application/x-gconf-string";
                break;
        case GCONF_VALUE_INT :
                mime_type = "application/x-gconf-int";
                break;
        case GCONF_VALUE_FLOAT :
                mime_type = "application/x-gconf-float";
                break;
        case GCONF_VALUE_BOOL :
                mime_type = "application/x-gconf-bool";
                break;
        case GCONF_VALUE_SCHEMA :
                mime_type = "application/x-gconf-schema";
                break;
        case GCONF_VALUE_LIST :
                mime_type = "application/x-gconf-list";
                break;
        case GCONF_VALUE_PAIR :
                mime_type = "application/x-gconf-pair";
                break;
        default :
                mime_type = "application/octet-stream";
                break;
                
        }

        info->mime_type = g_strdup (mime_type);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
set_mime_type_dir (GnomeVFSFileInfo *info,
                   const gchar *dirname,
                   GnomeVFSFileInfoOptions options)
{
        info->mime_type = g_strdup ("x-directory/normal");

	return GNOME_VFS_OK;
}

static GnomeVFSResult
get_value_size (const GConfValue *value, GnomeVFSFileSize *size)
{
	
	GnomeVFSFileSize subvalue_size = 0;
	GnomeVFSResult result = GNOME_VFS_OK;
	GSList *values;
	GConfSchema *schema;
	
	*size = 0;
	
	switch (value->type) {
        case GCONF_VALUE_INVALID :
                *size = 0;
                break;
        case GCONF_VALUE_STRING :
                if (value->d.string_data != NULL) 
			*size = strlen (value->d.string_data);
		else 
			*size = 0;
                break;
        case GCONF_VALUE_INT :
                *size = sizeof (gint);
                break;
        case GCONF_VALUE_FLOAT :
                *size = sizeof (gdouble);
                break;
        case GCONF_VALUE_BOOL :
                *size = sizeof (gboolean);
                break;
        case GCONF_VALUE_SCHEMA :
                schema = value->d.schema_data;
		*size = 0;
		if (schema->short_desc != NULL)
			*size += strlen (schema->short_desc);
		if (schema->long_desc != NULL)
			*size += strlen (schema->long_desc);
		if (schema->owner != NULL)
			*size += strlen (schema->owner);
		if (schema->default_value != NULL) {
			result = get_value_size (schema->default_value, 
						 &subvalue_size);
			if (result != GNOME_VFS_OK)
				return result;
			
			*size += subvalue_size;
		}
		
		break;
	case GCONF_VALUE_LIST :
                *size = 0;
		/* FIXME bugzilla.eazel.com 1181: This could be
                 * optimized, and may be a problem with huge lists.
		 */
		values = value->d.list_data.list;
		while (values != NULL) {
			result = get_value_size ((GConfValue*)values->data,
						 &subvalue_size);
			if (result != GNOME_VFS_OK) 
				return result;
			
			*size += subvalue_size;
			values = g_slist_next (values);
		}	
                break;
        case GCONF_VALUE_PAIR :
                result = get_value_size (value->d.pair_data.car, 
					 &subvalue_size);
		if (result != GNOME_VFS_OK) 
			return result;
		*size = subvalue_size;
                
		result = get_value_size (value->d.pair_data.car, 
					 &subvalue_size);
		if (result != GNOME_VFS_OK) 
			return result;
		
		*size += subvalue_size;		
                break;
	default :
		return GNOME_VFS_ERROR_INTERNAL;
		break;     
        }
	
	return GNOME_VFS_OK;
}
	
static GnomeVFSResult
set_stat_info_value (GnomeVFSFileInfo *info,
                     const GConfValue *value,
                     GnomeVFSFileInfoOptions options)
{
	GnomeVFSResult result;
	info->valid_fields = 0;
	info->type = GNOME_VFS_FILE_TYPE_REGULAR;
	info->permissions = 0444;
	info->atime = 0;
	info->mtime = 0;

	result = get_value_size (value, &info->size);
	if (result != GNOME_VFS_OK) 
		return result;

        GNOME_VFS_FILE_INFO_SET_LOCAL (info, TRUE);
        GNOME_VFS_FILE_INFO_SET_SUID (info, FALSE);
        GNOME_VFS_FILE_INFO_SET_SGID (info, FALSE);
        GNOME_VFS_FILE_INFO_SET_STICKY (info, FALSE);
        info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE | 
                GNOME_VFS_FILE_INFO_FIELDS_FLAGS |
                GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS |
                GNOME_VFS_FILE_INFO_FIELDS_SIZE |
                GNOME_VFS_FILE_INFO_FIELDS_ATIME |
                GNOME_VFS_FILE_INFO_FIELDS_MTIME |
                GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

	return GNOME_VFS_OK;
}

static GnomeVFSResult
set_stat_info_dir (GnomeVFSFileInfo *info,
                   GnomeVFSFileInfoOptions options)
{
	info->valid_fields = 0;
	info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
	info->permissions = 0444;
	info->atime = 0;
	info->mtime = 0;
	info->size = 0;

        GNOME_VFS_FILE_INFO_SET_LOCAL (info, TRUE);
        GNOME_VFS_FILE_INFO_SET_SUID (info, FALSE);
        GNOME_VFS_FILE_INFO_SET_SGID (info, FALSE);
        GNOME_VFS_FILE_INFO_SET_STICKY (info, FALSE);
        info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE | 
                GNOME_VFS_FILE_INFO_FIELDS_FLAGS |
                GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS |
                GNOME_VFS_FILE_INFO_FIELDS_SIZE |
                GNOME_VFS_FILE_INFO_FIELDS_ATIME |
                GNOME_VFS_FILE_INFO_FIELDS_MTIME |
                GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context)
{
	return GNOME_VFS_ERROR_WRONG_FORMAT;
}

#if 0
static GnomeVFSResult   
do_create (GnomeVFSMethod *method,
	   GnomeVFSMethodHandle **method_handle,
	   GnomeVFSURI *uri,
	   GnomeVFSOpenMode mode,
	   gboolean exclusive,
	   guint perm,
	   GnomeVFSContext *context)
{
	return GNOME_VFS_ERROR_WRONG_FORMAT;
}
#endif

static GnomeVFSResult   
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)
{
	return GNOME_VFS_ERROR_WRONG_FORMAT;
}

#if 0
static GnomeVFSResult
do_read (GnomeVFSMethodHandle *method_handle,
	 gpointer buffer,
	 GnomeVFSFileSize num_bytes,
	 GnomeVFSFileSize *bytes_read)
{
	return GNOME_VFS_ERROR_WRONG_FORMAT;
}

static GnomeVFSResult   
do_write (GnomeVFSMethodHandle *method_handle,
	  gconstpointer buffer,
	  GnomeVFSFileSize num_bytes,
	  GnomeVFSFileSize *bytes_written)
{
	return GNOME_VFS_ERROR_WRONG_FORMAT;
}
#endif

static GnomeVFSResult 
do_open_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle **method_handle,
                   GnomeVFSURI *uri,
                   GnomeVFSFileInfoOptions options,
                   const GnomeVFSDirectoryFilter *filter,
		   GnomeVFSContext *context)
{
        GSList *pairs;
        GSList *subdirs;
        gchar *dirname;

        MAKE_ABSOLUTE (dirname, uri->text);

	MUTEX_LOCK (client_mutex);
        subdirs = gconf_client_all_dirs (client, dirname, NULL);
        pairs = gconf_client_all_entries (client, dirname, NULL);
	MUTEX_UNLOCK (client_mutex);
        
        *method_handle = 
		(GnomeVFSMethodHandle*)directory_handle_new (uri,
							     options,
							     filter,
							     subdirs,
							     pairs);
        return GNOME_VFS_OK;
}

static GnomeVFSResult 
do_close_directory (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSContext *context)
{
        directory_handle_destroy ((DirectoryHandle *)method_handle);
        return GNOME_VFS_OK;
}

/* FIXME bugzilla.eazel.com 2794: 
   I dunno. There must be something to d there with metedata
   -- Mathieu */
static GnomeVFSResult
file_info_value (GnomeVFSFileInfo *info,
                 GnomeVFSFileInfoOptions options,
                 GConfValue *value,
                 const char *key)
{
        GnomeVFSResult result;

	info->name = g_strdup (key);
        result = set_stat_info_value (info, value, options);
	
	if (result != GNOME_VFS_OK) return result;
        
        if (options & GNOME_VFS_FILE_INFO_GET_MIME_TYPE)
                result = set_mime_type_value (info, value, options);

        return result;
}

static GnomeVFSResult
file_info_dir (GnomeVFSFileInfo *info,
               GnomeVFSFileInfoOptions options,
               gchar *dirname)
{
	GnomeVFSResult result;
	
        info->name = g_strdup (dirname);
        
	result = set_stat_info_dir (info, options);

	if (result != GNOME_VFS_OK) return result;

        if (options & GNOME_VFS_FILE_INFO_GET_MIME_TYPE)
                result = set_mime_type_dir (info, dirname, options);
        
        return result;
}

        
static GnomeVFSResult 
read_directory (DirectoryHandle *handle,
		GnomeVFSFileInfo *file_info,
		gboolean *skip)
{
        GnomeVFSResult result;
        GSList *tmp;
	const GnomeVFSDirectoryFilter *filter;
	GnomeVFSDirectoryFilterNeeds filter_needs;
	gboolean filter_called;

	filter_called = FALSE;
        filter = handle->filter;

	if (filter != NULL) {
		filter_needs = gnome_vfs_directory_filter_get_needs (filter);
	} else {
		filter_needs = GNOME_VFS_DIRECTORY_FILTER_NEEDS_NOTHING;
	}
	

	MUTEX_LOCK (handle->mutex);
	/* Get the next key info */
        if (handle->subdirs != NULL) {
                gchar *dirname = handle->subdirs->data;
                result = file_info_dir (file_info, handle->options, dirname);
                g_free (dirname);
                tmp = g_slist_next (handle->subdirs);
		g_slist_free_1 (handle->subdirs);
                handle->subdirs = tmp;
        } else if (handle->pairs != NULL) {
		GConfEntry *pair = handle->pairs->data;
                result = file_info_value (file_info, handle->options,
                                          pair->value, pair->key);
                gconf_entry_free (handle->pairs->data);
                tmp = g_slist_next (handle->subdirs);
                
		g_slist_free_1 (handle->pairs);
                handle->pairs = tmp;
        } else {
		result = GNOME_VFS_ERROR_EOF;
	}
	MUTEX_UNLOCK (handle->mutex);
	
	if (result != GNOME_VFS_OK) {
		return result;
	}
	
	/* Filter the file */
	*skip = FALSE;;
	if (filter != NULL
	    && !filter_called
	    && !(filter_needs 
		 & (GNOME_VFS_DIRECTORY_FILTER_NEEDS_TYPE
		    | GNOME_VFS_DIRECTORY_FILTER_NEEDS_STAT
		    | GNOME_VFS_DIRECTORY_FILTER_NEEDS_MIMETYPE))) {
		if (!gnome_vfs_directory_filter_apply (filter, file_info)) {
			*skip = TRUE;
			return GNOME_VFS_OK;
		}
		filter_called = TRUE;
	}

	return result;
}

static GnomeVFSResult 
do_read_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle *method_handle,
                   GnomeVFSFileInfo *file_info,
		   GnomeVFSContext *context)
{
	GnomeVFSResult result;
	gboolean skip;
	
	skip = FALSE;
	
	do {
		result = read_directory ((DirectoryHandle*)method_handle,
					 file_info, 
					 &skip);
		if (result != GNOME_VFS_OK) 
			break;
		if (skip)
			gnome_vfs_file_info_clear (file_info);

		
	} while (skip);

	return result;	    
}

GnomeVFSResult
do_get_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
                  GnomeVFSFileInfo *file_info,
                  GnomeVFSFileInfoOptions options,
		  GnomeVFSContext *context)
{
        GConfValue *value;
        gchar *key;
        
        MAKE_ABSOLUTE (key, uri->text);
        
	MUTEX_LOCK (client_mutex);
	if (gconf_client_dir_exists (client, key, NULL)) {
		MUTEX_UNLOCK (client_mutex);
		return file_info_dir (file_info, options, key);
	}

        value = gconf_client_get (client, key, NULL);
	
	MUTEX_UNLOCK (client_mutex);
	return file_info_value (file_info, options, value, key);
}

#if 0
static GnomeVFSResult  
do_get_file_info_from_handle (GnomeVFSMethodHandle *method_handle,
			      GnomeVFSFileInfo *file_info,
			      GnomeVFSFileInfoOptions options)
{
	return GNOME_VFS_ERROR_WRONG_FORMAT;	
}
#endif

gboolean 
do_is_local (GnomeVFSMethod *method, const GnomeVFSURI *uri)
{
        return TRUE;
}

GnomeVFSMethod *
vfs_module_init (const char *method_name, const char *args)
{
        char *argv[] = {"dummy"};
        int argc = 1;
	
	if (!gconf_is_initialized ()) {
		/* auto-initializes OAF if necessary */
		gconf_init (argc, argv, NULL);
	}

	/* These just return and do nothing if GTK
	   is already initialized. */
	gtk_type_init();
	gtk_signal_init();

	client = gconf_client_get_default ();

	gtk_object_ref(GTK_OBJECT(client));
	gtk_object_sink(GTK_OBJECT(client));
	
#ifdef G_THREADS_ENABLED
        if (g_thread_supported ())
                client_mutex = g_mutex_new ();
        else
                client_mutex = NULL;
#endif

        return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
	gtk_object_destroy(GTK_OBJECT(client));
	gtk_object_unref(GTK_OBJECT(client));

#ifdef G_THREADS_ENABLED
	if (g_thread_supported ()) 
		g_mutex_free (client_mutex);
#endif
	client = NULL;
}
