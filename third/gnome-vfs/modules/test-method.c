/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* Test-method.c: Gnome-VFS testing method

   Copyright (C) 2000 Eazel

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

   Authors: Seth Nickell      (seth@eazel.com) */

/* To use: create a /gnome/etc/vfs/Test-conf.xml, and point gnome-vfs
 * clients to Test:restofuri which will translate into the "real" method
 *
 * here's a sample config file (pointing to the file method):
 *
 *    <?xml version="1.0"?>
 *        <TestModule method="file">
 *	      <Function name="do_open_directory" result="GNOME_VFS_OK" execute_operation="TRUE" delay="2000"/>
 *        </TestModule>
 *
 */

#include <config.h>

#include <stdio.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-private.h>
#include <string.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>
#include "gnome-vfs-uri.h"

typedef struct {
	char *operation_name;
	GnomeVFSResult new_result;
	gboolean execute_operation;
        int delay;
} Interposition;

#define NUM_RESULT_STRINGS 41

static char TestMethodName[10];

static GList *TestOperationConfig;

static char
TestGnomeVFSResultString[NUM_RESULT_STRINGS][40] = {
			 "GNOME_VFS_OK",
			 "GNOME_VFS_ERROR_NOT_FOUND",
			 "GNOME_VFS_ERROR_GENERIC",
			 "GNOME_VFS_ERROR_INTERNAL",
			 "GNOME_VFS_ERROR_BAD_PARAMETERS",
			 "GNOME_VFS_ERROR_NOT_SUPPORTED",
			 "GNOME_VFS_ERROR_IO",
			 "GNOME_VFS_ERROR_CORRUPTED_DATA",
			 "GNOME_VFS_ERROR_WRONG_FORMAT",
			 "GNOME_VFS_ERROR_BAD_FILE",
			 "GNOME_VFS_ERROR_TOO_BIG",
			 "GNOME_VFS_ERROR_NO_SPACE",
			 "GNOME_VFS_ERROR_READ_ONLY",
			 "GNOME_VFS_ERROR_INVALID_URI",
			 "GNOME_VFS_ERROR_NOT_OPEN",
			 "GNOME_VFS_ERROR_INVALID_OPEN_MODE",
			 "GNOME_VFS_ERROR_ACCESS_DENIED",
			 "GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES",
			 "GNOME_VFS_ERROR_EOF",
			 "GNOME_VFS_ERROR_NOT_A_DIRECTORY",
			 "GNOME_VFS_ERROR_IN_PROGRESS",
			 "GNOME_VFS_ERROR_INTERRUPTED",
			 "GNOME_VFS_ERROR_FILE_EXISTS",
			 "GNOME_VFS_ERROR_LOOP",
			 "GNOME_VFS_ERROR_NOT_PERMITTED",
			 "GNOME_VFS_ERROR_IS_DIRECTORY",
			 "GNOME_VFS_ERROR_NO_MEMORY",
			 "GNOME_VFS_ERROR_HOST_NOT_FOUND",
			 "GNOME_VFS_ERROR_INVALID_HOST_NAME",
			 "GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS",
			 "GNOME_VFS_ERROR_LOGIN_FAILED",
			 "GNOME_VFS_ERROR_CANCELLED",
			 "GNOME_VFS_ERROR_DIRECTORY_BUSY",
			 "GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY",
			 "GNOME_VFS_ERROR_TOO_MANY_LINKS",
			 "GNOME_VFS_ERROR_READ_ONLY_FILE_SYSTEM",
			 "GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM",
			 "GNOME_VFS_ERROR_NAME_TOO_LONG",
			 "GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE",
			 "GNOME_VFS_NUM_ERRORS"
};

/* Module entry points. */
GnomeVFSMethod *vfs_module_init     (const char     *method_name,
				     const char     *args);
void            vfs_module_shutdown (GnomeVFSMethod *method);

static GnomeVFSURI *
translate_uri (GnomeVFSURI *uri)
{
	GnomeVFSURI *translated_uri;
	char *uri_text;
	char *translated_uri_text;
	char *no_method;

	uri_text = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	no_method = strchr (uri_text, ':');
	translated_uri_text = g_strconcat (TestMethodName, no_method, NULL);

	translated_uri = gnome_vfs_uri_new (translated_uri_text);

	g_free (uri_text);
	g_free (translated_uri_text);

	return translated_uri;
}


/* reads the configuration file and returns TRUE if there are special options for
   this execution of the operation.

   if TRUE is returned then result will contain the result the operation should return
   and perform_operation will be TRUE if the operation should execute the underlying
   operation anyway
*/
static gboolean
get_operation_configuration (char *function_identifier, GnomeVFSResult *result, 
			     gboolean *perform_operation, int *delay,
			     GList **other_options)
{
        GList *node;
	Interposition *operation_config;
	Interposition *found_config = NULL;

	printf ("get_operation_configuration for %s\n", function_identifier);
	for (node = TestOperationConfig; node != NULL; node = node->next) {
	        operation_config = node->data;
		if (strcmp (operation_config->operation_name, function_identifier)) {
		        found_config = operation_config;
		}
	}
	if (!found_config) {
	        return FALSE;
	} else {
	        *result = found_config->new_result;
		*perform_operation = found_config->execute_operation;
		*delay = found_config->delay;
		return TRUE;
	}	  
}

#define SWITCH_DEBUG(URI, OPERATION, FUNCTION_NAME)                      \
do {                                                                     \
	gboolean  perform_operation = FALSE;                             \
	GList    *other_options;                                         \
	GnomeVFSResult result_new;                                       \
	GnomeVFSURI *translated_uri, *hold_uri;                          \
	gboolean found_configuration;                                    \
        int delay;                                                       \
        char *delay_command;                                             \
                                                                         \
	found_configuration =                                            \
		get_operation_configuration ( (FUNCTION_NAME),           \
					      &result_new,               \
					      &perform_operation,        \
                                              &delay,                    \
					      &other_options);           \
	                                                                 \
        if (found_configuration) {                                       \
                delay_command = g_new (char, 90);                        \
                g_snprintf (delay_command, 90, "usleep %d",              \
			    delay * 1000);                               \
                system (delay_command);                                  \
                g_free (delay_command);                                  \
        }                                                                \
	if (!found_configuration || perform_operation) {                 \
	        translated_uri = URI;                                    \
	        URI = translate_uri( (URI) );                            \
                                                                         \
		if (found_configuration) {                               \
			OPERATION;                                       \
		} else {                                                 \
			result_new = OPERATION;                          \
		}                                                        \
                                                                         \
                hold_uri = URI;                                          \
	        URI = translated_uri;                                    \
                gnome_vfs_uri_unref (hold_uri);                          \
	}                                                                \
                                                                         \
	return result_new;                                               \
} while (0)       


#define SWITCH_DEBUG_NO_URI(OPERATION, FUNCTION_NAME)                    \
do {                                                                     \
	gboolean  perform_operation = FALSE;                             \
	GList    *other_options;                                         \
	GnomeVFSResult result_new;                                       \
	gboolean found_configuration;                                    \
        int delay;                                                       \
        char *delay_command;                                             \
                                                                         \
	found_configuration =                                            \
		get_operation_configuration ( (FUNCTION_NAME),           \
					      &result_new,               \
					      &perform_operation,        \
                                              &delay,                    \
					      &other_options);           \
	                                                                 \
        if (found_configuration) {                                       \
                delay_command = g_new (char, 90);                        \
                g_snprintf (delay_command, 90, "usleep %d",              \
			    delay * 1000);                               \
                system (delay_command);                                  \
                g_free (delay_command);                                  \
        }                                                                \
	if (!found_configuration || perform_operation) {                 \
                                                                         \
		if (found_configuration) {                               \
			OPERATION;                                       \
		} else {                                                 \
			result_new = OPERATION;                          \
		}                                                        \
	}                                                                \
                                                                         \
	return result_new;                                               \
} while (0)       

static GnomeVFSResult
parse_results_text (char *result) {
	int i;
	gboolean found = FALSE;

	for (i = 0; i < NUM_RESULT_STRINGS && !found; i++) {
		found = g_strcasecmp (result, TestGnomeVFSResultString[i]);
	}
	
	if (found) {
		return i;
	} else { 
			return GNOME_VFS_ERROR_NOT_FOUND;
	}
}

static GList *
load_config_file (char *filename) 
{
	Interposition *operation;
	GList *operation_list = NULL;
	xmlDocPtr doc;
	xmlNodePtr node;
	char *new_result_text;
	char *execute_text;
	char *method_name;
	char *delay_string;

	doc = xmlParseFile(filename); 

	/* FIXME bugzilla.eazel.com 3836: the module shouldn't crash when the config file doesn't exist */
	
	if(!doc || !doc->root || !doc->root->name || g_strcasecmp(doc->root->name,"TestModule")!=0) {
		xmlFreeDoc(doc);
		return FALSE;
	}

	method_name = xmlGetProp(doc->root, "method");
	g_snprintf (TestMethodName, 10, "%s", method_name);
	
	printf ("target method: %s\n", TestMethodName);

	for(node = doc->root->childs; node != NULL; node = node->next) {
		if (xmlGetProp (node, "name") != NULL) {
			operation = g_new (Interposition, 1);
			operation->operation_name = xmlGetProp(node, "name");
			new_result_text = xmlGetProp(node, "result");
			execute_text = xmlGetProp(node, "execute_operation");
			
			operation->new_result = parse_results_text (new_result_text);
			operation->execute_operation = execute_text != NULL && g_strcasecmp (execute_text, "TRUE");

			delay_string = xmlGetProp(node, "delay");
			if (delay_string != NULL) {
			      sscanf (delay_string, "%d", &(operation->execute_operation));
			} else {
			  operation->execute_operation = 0;
			}
						   			
			printf ("Added debug mode for %s, return value %s (%s), executed operation %s, delays %d seconds.\n", operation->operation_name, new_result_text, gnome_vfs_result_to_string(operation->new_result), execute_text, operation->execute_operation);

			g_list_append (operation_list, operation);
		}
	}

	return operation_list;
}

static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context)
{	
	SWITCH_DEBUG (uri, gnome_vfs_open_uri_cancellable ((GnomeVFSHandle **) method_handle, uri, mode, context), "do_open");
	/*
	GnomeVFSResult result;

	result = gnome_vfs_open_uri_cancellable
		((GnomeVFSHandle **) method_handle, uri, mode, context);
	return result;
	*/
}

static GnomeVFSResult
do_create (GnomeVFSMethod *method,
	   GnomeVFSMethodHandle **method_handle,
	   GnomeVFSURI *uri,
	   GnomeVFSOpenMode mode,
	   gboolean exclusive,
	   guint perm,
	   GnomeVFSContext *context)
{
	/* fixme: what do we do here? */
	return GNOME_VFS_ERROR_INTERNAL;
}

static GnomeVFSResult
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)
{
	SWITCH_DEBUG_NO_URI (gnome_vfs_close_cancellable ((GnomeVFSHandle *)method_handle, context), "do_close");
	/* GnomeVFSResult result;

	result = gnome_vfs_close_cancellable
		((GnomeVFSHandle *)method_handle, context);

	return result;
	*/
}

static GnomeVFSResult
do_read (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 gpointer buffer,
	 GnomeVFSFileSize num_bytes,
	 GnomeVFSFileSize *bytes_read,
	 GnomeVFSContext *context)
{
	SWITCH_DEBUG_NO_URI (gnome_vfs_read_cancellable((GnomeVFSHandle *)method_handle, buffer, num_bytes, bytes_read, context), "do_read");
	/*
	return gnome_vfs_read_cancellable
		((GnomeVFSHandle *)method_handle, buffer, num_bytes, bytes_read, context);
	*/
}

static GnomeVFSResult
do_write (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  gconstpointer buffer,
	  GnomeVFSFileSize num_bytes,
	  GnomeVFSFileSize *bytes_written,
	  GnomeVFSContext *context)
{
	SWITCH_DEBUG_NO_URI (gnome_vfs_write_cancellable((GnomeVFSHandle *)method_handle, buffer, num_bytes, bytes_written, context), "do_write");
	/*
	return gnome_vfs_write_cancellable
		((GnomeVFSHandle *)method_handle, buffer, num_bytes, bytes_written, context);
	*/
}

static GnomeVFSResult
do_seek (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSSeekPosition whence,
	 GnomeVFSFileOffset offset,
	 GnomeVFSContext *context)
{
	SWITCH_DEBUG_NO_URI (gnome_vfs_seek_cancellable((GnomeVFSHandle *)method_handle, whence, offset, context), "do_seek");
	/*
	return gnome_vfs_seek_cancellable
		((GnomeVFSHandle *)method_handle, whence, offset, context);
	*/
}

static GnomeVFSResult
do_tell (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSFileOffset *offset_return)
{
	SWITCH_DEBUG_NO_URI (gnome_vfs_tell((GnomeVFSHandle *)method_handle, offset_return), "do_tell");
	/*
	return gnome_vfs_tell
		((GnomeVFSHandle *)method_handle, offset_return);
	*/
}


static GnomeVFSResult
do_open_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle **method_handle,
		   GnomeVFSURI *uri,
		   GnomeVFSFileInfoOptions options,
		   const GnomeVFSDirectoryFilter *filter,
		   GnomeVFSContext *context)
{
	SWITCH_DEBUG (uri, gnome_vfs_directory_open_from_uri((GnomeVFSDirectoryHandle **)method_handle, uri, options, filter), "do_open_directory");
	/*
	return gnome_vfs_directory_open_from_uri
		((GnomeVFSDirectoryHandle **)method_handle, uri, options, filter);
	*/
}

static GnomeVFSResult
do_close_directory (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSContext *context)
{	
	return GNOME_VFS_OK;
	/* SWITCH_DEBUG_NO_URI (gnome_vfs_close ((GnomeVFSHandle *)method_handle), "do_close_directory"); */
	/*
	return gnome_vfs_close ((GnomeVFSHandle *)method_handle);
	*/
}

static GnomeVFSResult
do_read_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle *method_handle,
		   GnomeVFSFileInfo *file_info,
		   GnomeVFSContext *context)
{
	SWITCH_DEBUG_NO_URI (gnome_vfs_directory_read_next ((GnomeVFSDirectoryHandle *) method_handle, file_info), "do_read_directory");
	/*
	GnomeVFSResult result;

	result = gnome_vfs_directory_read_next ((GnomeVFSDirectoryHandle *) method_handle,
						file_info);
	return result;
	*/
}

static GnomeVFSResult
do_get_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  GnomeVFSFileInfo *file_info,
		  GnomeVFSFileInfoOptions options,
		  GnomeVFSContext *context)
{
	SWITCH_DEBUG (uri, gnome_vfs_get_file_info_uri_cancellable (uri, file_info, options, context), "do_get_file_info");
	/*
	GnomeVFSResult result;
	result = gnome_vfs_get_file_info_uri_cancellable (uri, file_info, 
							  options, context);

	return result;
	*/
}

static GnomeVFSResult
do_get_file_info_from_handle (GnomeVFSMethod *method,
			      GnomeVFSMethodHandle *method_handle,
			      GnomeVFSFileInfo *file_info,
			      GnomeVFSFileInfoOptions options,
			      GnomeVFSContext *context)
{
	SWITCH_DEBUG_NO_URI (gnome_vfs_get_file_info_from_handle_cancellable((GnomeVFSHandle *)method_handle, file_info, options, context), "do_get_file_info_from_handle");
	/*
	GnomeVFSResult result;

	result = gnome_vfs_get_file_info_from_handle_cancellable 
		((GnomeVFSHandle *)method_handle, file_info, options,
		 context);
	return result;
	*/
}

static gboolean
do_is_local (GnomeVFSMethod *method,
	     const GnomeVFSURI *uri)
{
	gboolean result;
	result = TRUE;
	return result;
}

static GnomeVFSResult
do_make_directory (GnomeVFSMethod *method,
		   GnomeVFSURI *uri,
		   guint perm,
		   GnomeVFSContext *context)
{
	SWITCH_DEBUG (uri,  gnome_vfs_make_directory_for_uri_cancellable (uri, perm, context), "do_make_directory");
	/*
	GnomeVFSResult result;
	result = gnome_vfs_make_directory_for_uri_cancellable (uri, perm, context);
	return result;
	*/
}

static GnomeVFSResult
do_remove_directory (GnomeVFSMethod *method,
		     GnomeVFSURI *uri,
		     GnomeVFSContext *context)
{
	SWITCH_DEBUG (uri, gnome_vfs_remove_directory_from_uri_cancellable (uri, context), "do_remove_directory");
	/*
	GnomeVFSResult result;
	result = gnome_vfs_remove_directory_from_uri_cancellable (uri, context);
	return result;
	*/
}

static GnomeVFSResult
do_move (GnomeVFSMethod *method,
	 GnomeVFSURI *old_uri,
	 GnomeVFSURI *new_uri,
	 gboolean force_replace,
	 GnomeVFSContext *context)
{
	/* FIXME bugzilla.eazel.com 3837: special case */
	GnomeVFSResult result;
	result = gnome_vfs_move_uri_cancellable (old_uri, new_uri, force_replace, context);
	return result;
}

static GnomeVFSResult
do_unlink (GnomeVFSMethod *method,
	   GnomeVFSURI *uri,
	   GnomeVFSContext *context)
{
	SWITCH_DEBUG (uri,  gnome_vfs_unlink_from_uri_cancellable (uri, context), "do_unlink");
	/*
	GnomeVFSResult result;
	result = gnome_vfs_unlink_from_uri_cancellable (uri, context);
	return result;
	*/
}

static GnomeVFSResult
do_check_same_fs (GnomeVFSMethod *method,
		  GnomeVFSURI *a,
		  GnomeVFSURI *b,
		  gboolean *same_fs_return,
		  GnomeVFSContext *context)
{
	/* FIXME bugzilla.eazel.com 3837: special case */
	GnomeVFSResult result;
	result = gnome_vfs_check_same_fs_uris_cancellable (a, b, same_fs_return, context);
	return result;
}

static GnomeVFSResult
do_set_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  const GnomeVFSFileInfo *info,
		  GnomeVFSSetFileInfoMask mask,
		  GnomeVFSContext *context)
{	
	SWITCH_DEBUG (uri, gnome_vfs_set_file_info_cancellable (uri, info, mask, context), "do_set_file_info");
	/*
	GnomeVFSResult result;
	result = gnome_vfs_set_file_info_cancellable (uri, info, mask, context);
	return result;
	*/
}

static GnomeVFSResult
do_truncate (GnomeVFSMethod *method,
	     GnomeVFSURI *uri,
	     GnomeVFSFileSize where,
	     GnomeVFSContext *context)
{
	SWITCH_DEBUG (uri,  gnome_vfs_truncate_uri_cancellable (uri, where, context), "do_truncate");
	/*
	GnomeVFSResult result;
	result = gnome_vfs_truncate_uri_cancellable (uri, where, context);
	return result;
	*/
}

static GnomeVFSResult
do_truncate_handle (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSFileSize where,
		    GnomeVFSContext *context)
{
	SWITCH_DEBUG_NO_URI (gnome_vfs_truncate_handle_cancellable((GnomeVFSHandle *)method_handle, where, context), "do_truncate_handle");
	/*
	return gnome_vfs_truncate_handle_cancellable
		((GnomeVFSHandle *)method_handle, where, context);
	*/
}

static GnomeVFSResult
do_find_directory (GnomeVFSMethod *method,
		   GnomeVFSURI *near_uri,
		   GnomeVFSFindDirectoryKind kind,
		   GnomeVFSURI **result_uri,
		   gboolean create_if_needed,
		   gboolean find_if_needed,
		   guint permissions,
		   GnomeVFSContext *context)
{
	SWITCH_DEBUG (near_uri, gnome_vfs_find_directory_cancellable (near_uri, kind, result_uri, create_if_needed, find_if_needed, permissions, context), "do_find_directory");
	/*
	GnomeVFSResult result;
	result = gnome_vfs_find_directory_cancellable (near_uri, kind, result_uri, 
			create_if_needed, find_if_needed, permissions, context);
	return result;
	*/
}

static GnomeVFSResult
do_create_symbolic_link (GnomeVFSMethod *method,
			 GnomeVFSURI *uri,
			 const char *target_reference,
			 GnomeVFSContext *context)
{
	SWITCH_DEBUG (uri, gnome_vfs_create_symbolic_link_cancellable (uri, target_reference, context), "do_create_symbolic_link");
	/*
	GnomeVFSResult result;
	result = gnome_vfs_create_symbolic_link_cancellable (uri, target_reference, context);
	return result;
	*/
}

static GnomeVFSMethod method = {
	do_open,
	do_create,
	do_close,
	do_read,
	do_write,
	do_seek,
	do_tell,
	do_truncate_handle,
	do_open_directory,
	do_close_directory,
	do_read_directory,
	do_get_file_info,
	do_get_file_info_from_handle,
	do_is_local,
	do_make_directory,
	do_remove_directory,
	do_move,
	do_unlink,
	do_check_same_fs,
        do_set_file_info,
	do_truncate,
	do_find_directory,
	do_create_symbolic_link
};


GnomeVFSMethod *
vfs_module_init (const char *method_name, const char *args)
{
	/* FIXME bugzilla.eazel.com 3838: the path to the config file should not be hardcoded */
	TestOperationConfig = load_config_file ("/gnome/etc/vfs/Test-conf.xml");
	printf ("Module initialized.\n");
	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
}
