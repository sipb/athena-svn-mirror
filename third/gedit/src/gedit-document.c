/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gedit-document.c
 * This file is part of gedit
 *
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
/*
 * Modified by the gedit Team, 1998-2001. See the AUTHORS file for a 
 * list of people on the gedit Team.  
 * See the ChangeLog files for a list of changes. 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>  
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>

#include <libgnome/libgnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <eel/eel-vfs-extensions.h>

#include "gedit-prefs-manager.h"
#include "gedit-document.h"
#include "gedit-debug.h"
#include "gedit-utils.h"
#include "gedit-undo-manager.h"

#include "gedit-marshal.h"

#ifdef MAXPATHLEN
#define GEDIT_MAX_PATH_LEN  MAXPATHLEN
#elif defined (PATH_MAX)
#define GEDIT_MAX_PATH_LEN  PATH_MAX
#else
#define GEDIT_MAX_PATH_LEN  2048
#endif

struct _GeditDocumentPrivate
{
	gchar		*uri;
	gint 		 untitled_number;	

	gchar		*encoding;

	gchar		*last_searched_text;
	gchar		*last_replace_text;
	gboolean  	 last_search_was_case_sensitive;
	gboolean  	 last_search_was_entire_word;

	guint		 auto_save_timeout;
	gboolean	 last_save_was_manually; 
	
	gboolean 	 readonly;

	GeditUndoManager *undo_manager;
};

enum {
	NAME_CHANGED,
	SAVED,
	LOADED,
	READONLY_CHANGED,
	CAN_UNDO,
	CAN_REDO,
	LAST_SIGNAL
};

static void gedit_document_class_init 		(GeditDocumentClass 	*klass);
static void gedit_document_init 		(GeditDocument 		*document);
static void gedit_document_finalize 		(GObject 		*object);

static void gedit_document_real_name_changed		(GeditDocument *document);
static void gedit_document_real_loaded			(GeditDocument *document);
static void gedit_document_real_saved			(GeditDocument *document);
static void gedit_document_real_readonly_changed	(GeditDocument *document,
							 gboolean readonly);

static gboolean	gedit_document_save_as_real (GeditDocument* doc, const gchar *uri, 
					     gboolean create_backup_copy, GError **error);
static void gedit_document_set_uri (GeditDocument* doc, const gchar* uri);

static void gedit_document_can_undo_handler (GeditUndoManager* um, gboolean can_undo, 
					     GeditDocument* doc);

static void gedit_document_can_redo_handler (GeditUndoManager* um, gboolean can_redo, 
					     GeditDocument* doc);
static gboolean gedit_document_auto_save (GeditDocument *doc, GError **error);
static gboolean gedit_document_auto_save_timeout (GeditDocument *doc);

static GtkTextBufferClass *parent_class 	= NULL;
static guint document_signals[LAST_SIGNAL] 	= { 0 };

static GHashTable* allocated_untitled_numbers = NULL;

static gint gedit_document_get_untitled_number (void);
static void gedit_document_release_untitled_number (gint n);

static gint
gedit_document_get_untitled_number (void)
{
	gint i = 1;
	
	gedit_debug (DEBUG_DOCUMENT, "");

	if (allocated_untitled_numbers == NULL)
		allocated_untitled_numbers = g_hash_table_new (NULL, NULL);

	g_return_val_if_fail (allocated_untitled_numbers != NULL, -1);
	
	while (TRUE)
	{
		if (g_hash_table_lookup (allocated_untitled_numbers, GINT_TO_POINTER (i)) == NULL)
		{
			g_hash_table_insert (allocated_untitled_numbers, 
					     GINT_TO_POINTER (i),
					     GINT_TO_POINTER (i));
	
			return i;
		}
		
		++i;
	}					
}

static void
gedit_document_release_untitled_number (gint n)
{
	gboolean ret;
	
	g_return_if_fail (allocated_untitled_numbers != NULL);

	gedit_debug (DEBUG_DOCUMENT, "");

	ret = g_hash_table_remove (allocated_untitled_numbers, GINT_TO_POINTER (n));
	g_return_if_fail (ret);	
}



GType
gedit_document_get_type (void)
{
	static GType document_type = 0;

  	if (document_type == 0)
    	{
      		static const GTypeInfo our_info =
      		{
        		sizeof (GeditDocumentClass),
        		NULL, 		/* base_init,*/
        		NULL, 		/* base_finalize, */
        		(GClassInitFunc) gedit_document_class_init,
        		NULL,           /* class_finalize */
        		NULL,           /* class_data */
        		sizeof (GeditDocument),
        		0,              /* n_preallocs */
        		(GInstanceInitFunc) gedit_document_init
      		};

      		document_type = g_type_register_static (GTK_TYPE_TEXT_BUFFER,
                					"GeditDocument",
                                           	 	&our_info,
                                           		0);
    	}

	return document_type;
}

	
static void
gedit_document_class_init (GeditDocumentClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

  	parent_class = g_type_class_peek_parent (klass);

  	object_class->finalize = gedit_document_finalize;

        klass->name_changed 	= gedit_document_real_name_changed;
	klass->loaded 	    	= gedit_document_real_loaded;
	klass->saved        	= gedit_document_real_saved;
	klass->readonly_changed = gedit_document_real_readonly_changed;
	klass->can_undo		= NULL;
	klass->can_redo		= NULL;

  	document_signals[NAME_CHANGED] =
   		g_signal_new ("name_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditDocumentClass, name_changed),
			      NULL, NULL,
			      gedit_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	
	document_signals[LOADED] =
   		g_signal_new ("loaded",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditDocumentClass, loaded),
			      NULL, NULL,
			      gedit_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);
	
	document_signals[SAVED] =
   		g_signal_new ("saved",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditDocumentClass, saved),
			      NULL, NULL,
			      gedit_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	document_signals[READONLY_CHANGED] =
   		g_signal_new ("readonly_changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditDocumentClass, readonly_changed),
			      NULL, NULL,
			      gedit_marshal_VOID__BOOLEAN,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_BOOLEAN);

	document_signals[CAN_UNDO] =
   		g_signal_new ("can_undo",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditDocumentClass, can_undo),
			      NULL, NULL,
			      gedit_marshal_VOID__BOOLEAN,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_BOOLEAN);

	document_signals[CAN_REDO] =
   		g_signal_new ("can_redo",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditDocumentClass, can_redo),
			      NULL, NULL,
			      gedit_marshal_VOID__BOOLEAN,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_BOOLEAN);

}

static void
gedit_document_init (GeditDocument *document)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	document->priv = g_new0 (GeditDocumentPrivate, 1);
	
	document->priv->uri = NULL;
	document->priv->untitled_number = 0;

	document->priv->readonly = FALSE;

	document->priv->last_save_was_manually = TRUE;

	document->priv->undo_manager = gedit_undo_manager_new (document);

	if (gedit_prefs_manager_get_save_encoding () == 
			GEDIT_SAVE_ORIGINAL_FILE_ENCODING_IF_POSSIBLE_NCL)
	{
		const gchar *encoding = NULL;
		g_get_charset(&encoding);
				
		document->priv->encoding = g_strdup (encoding);
	}
	else
		document->priv->encoding = NULL;
			
	g_signal_connect (G_OBJECT (document->priv->undo_manager), "can_undo",
			  G_CALLBACK (gedit_document_can_undo_handler), 
			  document);

	g_signal_connect (G_OBJECT (document->priv->undo_manager), "can_redo",
			  G_CALLBACK (gedit_document_can_redo_handler), 
			  document);
}

static void
gedit_document_finalize (GObject *object)
{
	GeditDocument *document;

	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (object != NULL);
	g_return_if_fail (GEDIT_IS_DOCUMENT (object));

   	document = GEDIT_DOCUMENT (object);

	g_return_if_fail (document->priv != NULL);

	if (document->priv->auto_save_timeout > 0)
		g_source_remove (document->priv->auto_save_timeout);

	if (document->priv->untitled_number > 0)
	{
		g_return_if_fail (document->priv->uri == NULL);
		gedit_document_release_untitled_number (
				document->priv->untitled_number);
	}

	g_free (document->priv->uri);
	g_free (document->priv->last_searched_text);
	g_free (document->priv->last_replace_text);
	g_free (document->priv->encoding);

	g_object_unref (G_OBJECT (document->priv->undo_manager));
	
	g_free (document->priv);
	
	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * gedit_document_new:
 *
 * Creates a new untitled document.
 *
 * Return value: a new untitled document
 **/
GeditDocument*
gedit_document_new (void)
{
 	GeditDocument *document;

	gedit_debug (DEBUG_DOCUMENT, "");

	document = GEDIT_DOCUMENT (g_object_new (GEDIT_TYPE_DOCUMENT, NULL));

	g_return_val_if_fail (document->priv != NULL, NULL);
  	
	document->priv->untitled_number = gedit_document_get_untitled_number ();
	g_return_val_if_fail (document->priv->untitled_number > 0, NULL);

	return document;
}

/**
 * gedit_document_new_with_uri:
 * @uri: the URI of the file that has to be loaded
 * @error: return location for error or NULL
 * 
 * Creates a new document.
 *
 * Return value: a new document
 **/
GeditDocument*
gedit_document_new_with_uri (const gchar *uri, GError **error)
{
 	GeditDocument *document;

	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (uri != NULL, NULL);

	document = GEDIT_DOCUMENT (g_object_new (GEDIT_TYPE_DOCUMENT, NULL));

	g_return_val_if_fail (document->priv != NULL, NULL);
	document->priv->uri = g_strdup (uri);

	if (!gedit_document_load (document, uri, error))
	{
		gedit_debug (DEBUG_DOCUMENT, "ERROR");

		g_object_unref (document);
		return NULL;
	}
	
	return document;
}

/**
 * gedit_document_set_readonly:
 * @document: a #GeditDocument
 * @readonly: if TRUE (FALSE) the @document will be set as (not) readonly
 * 
 * Set the value of the readonly flag.
 **/
void 		
gedit_document_set_readonly (GeditDocument *document, gboolean readonly)
{
	gboolean auto_save;
	
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (document != NULL);
	g_return_if_fail (document->priv != NULL);

	auto_save = gedit_prefs_manager_get_auto_save ();
	
	if (readonly) 
	{
		if (auto_save && (document->priv->auto_save_timeout > 0))
		{
			gedit_debug (DEBUG_DOCUMENT, "Remove autosave timeout");

			g_source_remove (document->priv->auto_save_timeout);
			document->priv->auto_save_timeout = 0;
		}
	}
	else
	{
		if (auto_save && (document->priv->auto_save_timeout <= 0))
		{
			gint auto_save_interval;
			
			gedit_debug (DEBUG_DOCUMENT, "Install autosave timeout");

			auto_save_interval = 
				gedit_prefs_manager_get_auto_save_interval ();
				
			document->priv->auto_save_timeout = g_timeout_add 
				(auto_save_interval * 1000 * 60,
		 		 (GSourceFunc)gedit_document_auto_save_timeout,
		  		 document);		
		}
	}

	if (document->priv->readonly == readonly) 
		return;

	document->priv->readonly = readonly;

	g_signal_emit (G_OBJECT (document),
                       document_signals[READONLY_CHANGED],
                       0,
		       readonly);
}

/**
 * gedit_document_is_readonly:
 * @document: a #GeditDocument
 *
 * Returns TRUE is @document is readonly. 
 * 
 * Return value: TRUE if @document is readonly. FALSE otherwise.
 **/
gboolean
gedit_document_is_readonly (GeditDocument *document)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (document != NULL, TRUE);
	g_return_val_if_fail (document->priv != NULL, TRUE);

	return document->priv->readonly;	
}

static void 
gedit_document_real_name_changed (GeditDocument *document)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (document != NULL);
}

static void 
gedit_document_real_loaded (GeditDocument *document)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (document != NULL);
}

static void 
gedit_document_real_saved (GeditDocument *document)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (document != NULL);
}	

static void 
gedit_document_real_readonly_changed (GeditDocument *document, gboolean readonly)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (document != NULL);
}


gchar*
gedit_document_get_raw_uri (const GeditDocument* doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	if (doc->priv->uri == NULL)
		return NULL;
	else
		return g_strdup (doc->priv->uri);
}

/* 
 * Returns a well formatted (ready to display) URI in UTF-8 format 
 * See: gedit_document_get_raw_uri to have a raw uri (non UTF-8)
 */
gchar*
gedit_document_get_uri (const GeditDocument* doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	if (doc->priv->uri == NULL)
		return g_strdup_printf (_("%s %d"), _("Untitled"), doc->priv->untitled_number);
	else
	{
		gchar *res;

		res = eel_format_uri_for_display (doc->priv->uri);
		g_return_val_if_fail (res != NULL, g_strdup (_("Invalid file name")));
		
		return res;
	}
}

gchar*
gedit_document_get_short_name (const GeditDocument* doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	if (doc->priv->uri == NULL)
		return g_strdup_printf (_("%s %d"), _("Untitled"), doc->priv->untitled_number);
	else
	{
		gchar *basename;
		gchar *utf8_basename;
		
		basename = eel_uri_get_basename (doc->priv->uri);

		if (basename != NULL) 
		{
			gboolean filenames_are_locale_encoded;
		       	filenames_are_locale_encoded = g_getenv ("G_BROKEN_FILENAMES") != NULL;

			if (filenames_are_locale_encoded) 
			{
				utf8_basename = g_locale_to_utf8 (basename, -1, NULL, NULL, NULL);
				
				if (utf8_basename != NULL) 
				{
					g_free (basename);
					return utf8_basename;
				} 
			}
			else 
			{
				if (g_utf8_validate (basename, -1, NULL)) 
					return basename;
			}
			
			/* there are problems */
			utf8_basename = eel_make_valid_utf8 (basename);
			g_free (basename);
			return utf8_basename;
		}
		else
			return 	g_strdup (_("Invalid file name"));
	}
}

GQuark 
gedit_document_io_error_quark (void)
{
	static GQuark quark;
	
	if (!quark)
		quark = g_quark_from_static_string ("gedit_io_load_error");

	return quark;
}

static gboolean
gedit_document_auto_save_timeout (GeditDocument *doc)
{
	GError *error = NULL;

	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (!gedit_document_is_readonly (doc), FALSE);

	/* Remove timeout if now auto_save is FALSE */
	if (!gedit_prefs_manager_get_auto_save ()) 
		return FALSE;

	if (!gedit_document_get_modified (doc))
		return TRUE;

	gedit_document_auto_save (doc, &error);

	if (error) 
	{
		/* FIXME - Should we actually tell 
		 * the user there was an error? - James */
		g_error_free (error);
		return TRUE;
	}

	return TRUE;
}

static gboolean
gedit_document_auto_save (GeditDocument* doc, GError **error)
{
	gedit_debug (DEBUG_DOCUMENT, "");
	
	g_return_val_if_fail (doc != NULL, FALSE);

	if (gedit_document_save_as_real (doc, doc->priv->uri, doc->priv->last_save_was_manually, NULL))
		doc->priv->last_save_was_manually = FALSE;

	return TRUE;
}

/**
 * gedit_document_load:
 * @doc: a GeditDocument
 * @uri: the URI of the file that has to be loaded
 * @error: return location for error or NULL
 * 
 * Read a document from a file
 *
 * Return value: TRUE if the file is correctly loaded
 **/
gboolean
gedit_document_load (GeditDocument* doc, const gchar *uri, GError **error)
{
	char* file_contents;
	GnomeVFSResult res;
   	gsize file_size;
	GtkTextIter iter, end;
	
	gedit_debug (DEBUG_DOCUMENT, "File to load: %s", uri);

	g_return_val_if_fail (doc != NULL, FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	res = eel_read_entire_file (uri, &file_size, &file_contents);

	gedit_debug (DEBUG_DOCUMENT, "End reading %s (result: %s)", uri, gnome_vfs_result_to_string (res));
	
	if (res != GNOME_VFS_OK)
	{
		g_set_error (error, GEDIT_DOCUMENT_IO_ERROR, res,
			gnome_vfs_result_to_string (res));
		return FALSE;
	}

	if (file_size > 0)
	{
		gchar *converted_text;
		gint len = -1;

		if (g_utf8_validate (file_contents, file_size, NULL))
		{
			converted_text = file_contents;
			len = file_size;
			file_contents = NULL;
		}
		else
			converted_text = gedit_utils_convert_to_utf8 (file_contents,
							file_size,
							&doc->priv->encoding);

		if (converted_text == NULL)
		{
			/* bail out */
			g_set_error (error, GEDIT_DOCUMENT_IO_ERROR,
				     GEDIT_ERROR_INVALID_UTF8_DATA,
				     _("Invalid UTF-8 data"));

			g_free (file_contents);

			return FALSE;
		}

		gedit_debug (DEBUG_DOCUMENT, "Document encoding: %s", 
			     doc->priv->encoding == NULL ? "UTF-8 (Null)" : doc->priv->encoding);

		gedit_undo_manager_begin_not_undoable_action (doc->priv->undo_manager);
		/* Insert text in the buffer */
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &iter, 0);
		gtk_text_buffer_insert (GTK_TEXT_BUFFER (doc), &iter, converted_text, len);
		g_free (converted_text);

		/* We had a newline in the buffer to begin with. (The buffer always contains
   		 * a newline, so we delete to the end of the buffer to clean up. */
		gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (doc), &end);
 		gtk_text_buffer_delete (GTK_TEXT_BUFFER (doc), &iter, &end);

		/* Place the cursor at the start of the document */
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &iter, 0);
		gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (doc), &iter);

		gedit_undo_manager_end_not_undoable_action (doc->priv->undo_manager);
	}

	if (file_contents != NULL)
		g_free (file_contents);

	if (gedit_utils_is_uri_read_only (uri))
	{
		gedit_debug (DEBUG_DOCUMENT, "READ-ONLY");

		gedit_document_set_readonly (doc, TRUE);
	}
	else
	{
		gedit_debug (DEBUG_DOCUMENT, "NOT READ-ONLY");

		gedit_document_set_readonly (doc, FALSE);
	}

	gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (doc), FALSE);

	gedit_document_set_uri (doc, uri);
		
	g_signal_emit (G_OBJECT (doc), document_signals[LOADED], 0);

	return TRUE;
}

gboolean
gedit_document_load_from_stdin (GeditDocument* doc, GError **error)
{
	gchar *stdin_data;
	GtkTextIter iter, end;
	
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (doc != NULL, FALSE);
	
	stdin_data = gedit_utils_get_stdin ();

	if (stdin_data != NULL && strlen (stdin_data) > 0)
	{
		gchar *converted_text;

		if (g_utf8_validate (stdin_data, -1, NULL))
		{
			converted_text = stdin_data;
			stdin_data = NULL;
		}
		else
			converted_text = gedit_utils_convert_to_utf8 (stdin_data,
							-1,
							&doc->priv->encoding);

		if (converted_text == NULL)
		{
			/* bail out */
			g_set_error (error, GEDIT_DOCUMENT_IO_ERROR,
				     GEDIT_ERROR_INVALID_UTF8_DATA,
				     _("Invalid UTF-8 data"));

			g_free (stdin_data);

			return FALSE;
		}

		gedit_undo_manager_begin_not_undoable_action (doc->priv->undo_manager);
		/* Insert text in the buffer */
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &iter, 0);
		gtk_text_buffer_insert (GTK_TEXT_BUFFER (doc), &iter, converted_text, -1);

		/* We had a newline in the buffer to begin with. (The buffer always contains
   		 * a newline, so we delete to the end of the buffer to clean up. */
		gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (doc), &end);
 		gtk_text_buffer_delete (GTK_TEXT_BUFFER (doc), &iter, &end);

		/* Place the cursor at the start of the document */
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &iter, 0);
		gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (doc), &iter);

		gedit_undo_manager_end_not_undoable_action (doc->priv->undo_manager);
		
		g_free (converted_text);
	}

	if (stdin_data != NULL)
		g_free (stdin_data);
			

	gedit_document_set_readonly (doc, FALSE);
	gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (doc), TRUE);

	g_signal_emit (G_OBJECT (doc), document_signals [LOADED], 0);

	return TRUE;
}


gboolean 
gedit_document_is_untouched (const GeditDocument *doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	return (doc->priv->uri == NULL) && 
		(!gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (doc)));
}

static void
gedit_document_set_uri (GeditDocument* doc, const gchar* uri)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (doc != NULL);
	g_return_if_fail (doc->priv != NULL);

	if (doc->priv->uri == uri)
		return;

	if (doc->priv->uri != NULL)
		g_free (doc->priv->uri);
			
	doc->priv->uri = g_strdup (uri);

	if (doc->priv->untitled_number > 0)
	{
		gedit_document_release_untitled_number (doc->priv->untitled_number);
		doc->priv->untitled_number = 0;
	}
	
	g_signal_emit (G_OBJECT (doc), document_signals[NAME_CHANGED], 0);
}
	
gboolean 
gedit_document_save (GeditDocument* doc, GError **error)
{	
	gboolean auto_save;
	gboolean create_backup_copy;

	gboolean ret;

	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (doc != NULL, FALSE);
	g_return_val_if_fail (doc->priv != NULL, FALSE);
	g_return_val_if_fail (!doc->priv->readonly, FALSE);
	g_return_val_if_fail (doc->priv->uri != NULL, FALSE);
	
	auto_save = gedit_prefs_manager_get_auto_save ();
	create_backup_copy = gedit_prefs_manager_get_create_backup_copy ();
		
	if (auto_save) 
	{
		if (doc->priv->auto_save_timeout > 0)
		{
			g_source_remove (doc->priv->auto_save_timeout);
			doc->priv->auto_save_timeout = 0;
		}
	}
	
	ret = gedit_document_save_as_real (doc, 
					   doc->priv->uri, 
					   create_backup_copy, 
					   error);

	if (ret)
		doc->priv->last_save_was_manually = TRUE;

	if (auto_save && (doc->priv->auto_save_timeout <= 0)) 
	{
		gint auto_save_interval = 
			gedit_prefs_manager_get_auto_save_interval ();

		doc->priv->auto_save_timeout =
			g_timeout_add (auto_save_interval * 1000 * 60,
				       (GSourceFunc) gedit_document_auto_save, 
				       doc);
	}

	return ret;
}

gboolean	
gedit_document_save_as (GeditDocument* doc, const gchar *uri, GError **error)
{	
	gboolean auto_save;
	
	gboolean ret = FALSE;
	
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (doc != NULL, FALSE);
	g_return_val_if_fail (doc->priv != NULL, FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	auto_save = gedit_prefs_manager_get_auto_save ();

	if (auto_save) 
	{

		if (doc->priv->auto_save_timeout > 0)
		{
			g_source_remove (doc->priv->auto_save_timeout);
			doc->priv->auto_save_timeout = 0;
		}
	}

	if (gedit_document_save_as_real (doc, uri, TRUE, error))
	{
		gedit_document_set_uri (doc, uri);
		gedit_document_set_readonly (doc, FALSE);
		doc->priv->last_save_was_manually = TRUE;

		ret = TRUE;
	}		

	if (auto_save && (doc->priv->auto_save_timeout <= 0)) 
	{
		gint auto_save_interval =
			gedit_prefs_manager_get_auto_save_interval ();
		
		doc->priv->auto_save_timeout =
			g_timeout_add (auto_save_interval * 1000 * 60,
				       (GSourceFunc)gedit_document_auto_save, doc);
	}

	return ret;
}

gboolean	
gedit_document_save_a_copy_as (GeditDocument* doc, const gchar *uri, GError **error)
{
	gboolean m;
	gboolean ret;

	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (doc != NULL, FALSE);
	g_return_val_if_fail (doc->priv != NULL, FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);
	
	m = gedit_document_get_modified (doc);

	ret = gedit_document_save_as_real (doc, uri, FALSE, error);
	
	gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (doc), m);	 

	return ret;
}

#define MAX_LINK_LEVEL 256

/* Does readlink() recursively until we find a real filename. */
static char *
follow_symlinks (const gchar *filename, GError **error)
{
	gchar *followed_filename;
	gint link_count;

	g_return_val_if_fail (filename != NULL, NULL);
	
	gedit_debug (DEBUG_DOCUMENT, "Filename: %s", filename); 
	g_return_val_if_fail (strlen (filename) + 1 <= GEDIT_MAX_PATH_LEN, NULL);

	followed_filename = g_strdup (filename);
	link_count = 0;

	while (link_count < MAX_LINK_LEVEL)
	{
		struct stat st;

		if (lstat (followed_filename, &st) != 0)
			/* We could not access the file, so perhaps it does not
			 * exist.  Return this as a real name so that we can
			 * attempt to create the file.
			 */
			return followed_filename;

		if (S_ISLNK (st.st_mode))
		{
			gint len;
			gchar linkname[GEDIT_MAX_PATH_LEN];

			link_count++;

			len = readlink (followed_filename, linkname, GEDIT_MAX_PATH_LEN - 1);

			if (len == -1)
			{
				g_set_error (error, GEDIT_DOCUMENT_IO_ERROR, errno,
					     _("Could not read symbolic link information "
					       "for %s"), followed_filename);
				g_free (followed_filename);
				return NULL;
			}

			linkname[len] = '\0';

			/* If the linkname is not an absolute path name, append
			 * it to the directory name of the followed filename.  E.g.
			 * we may have /foo/bar/baz.lnk -> eek.txt, which really
			 * is /foo/bar/eek.txt.
			 */

			if (linkname[0] != G_DIR_SEPARATOR)
			{
				gchar *slashpos;
				gchar *tmp;

				slashpos = strrchr (followed_filename, G_DIR_SEPARATOR);

				if (slashpos)
					*slashpos = '\0';
				else
				{
					tmp = g_strconcat ("./", followed_filename, NULL);
					g_free (followed_filename);
					followed_filename = tmp;
				}

				tmp = g_build_filename (followed_filename, linkname, NULL);
				g_free (followed_filename);
				followed_filename = tmp;
			}
			else
			{
				g_free (followed_filename);
				followed_filename = g_strdup (linkname);
			}
		} else
			return followed_filename;
	}

	/* Too many symlinks */

	g_set_error (error, GEDIT_DOCUMENT_IO_ERROR, ELOOP,
		     _("The file has too many symbolic links."));

	return NULL;
}

/* FIXME: define new ERROR_CODE and remove the error 
 * strings from here -- Paolo
 */

static gboolean	
gedit_document_save_as_real (GeditDocument* doc, const gchar *uri,
			     gboolean create_backup_copy, GError **error)
{
	gchar *filename; /* Filename without URI scheme */
	gchar *real_filename; /* Final filename with no symlinks */
	gchar *backup_filename; /* Backup filename, like real_filename.bak */
	gchar *temp_filename; /* Filename for saving */
	gchar *slashpos;
	gchar *dirname;
	mode_t saved_umask;
	struct stat st;
	gchar *chars = NULL;
	gint chars_len;
	gint fd;
	gint retval;
	GeditSaveEncodingSetting encoding_setting;
	gboolean res;
	gboolean add_cr;
	
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (doc != NULL, FALSE);
	g_return_val_if_fail (doc->priv != NULL, FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	retval = FALSE;

	filename = NULL;
	real_filename = NULL;
	backup_filename = NULL;
	temp_filename = NULL;

	/* We don't support non-file:/// stuff */

	if (!gedit_utils_uri_has_file_scheme (uri))
	{
		gchar *error_message;
		gchar *scheme_string;

		gchar *temp = eel_uri_get_scheme (uri);
                scheme_string = eel_make_valid_utf8 (temp);
		g_free (temp);

		if (scheme_string != NULL)
		{
			error_message = g_strdup_printf (
				_("gedit cannot handle %s: locations in write mode."),
                               	scheme_string);

			g_free (scheme_string);
		}
		else
			error_message = g_strdup_printf (
				_("gedit cannot handle this kind of location in write mode."));

		g_set_error (error, GEDIT_DOCUMENT_IO_ERROR, EROFS, error_message);
		g_free (error_message);
		return FALSE;
	}

	/* Get filename from uri */
	filename = gnome_vfs_get_local_path_from_uri (uri);

	if (!filename)
	{
		g_set_error (error, GEDIT_DOCUMENT_IO_ERROR, 0,
			     _("Invalid filename."));
		goto out;
	}

	/* Get the real filename and file permissions */

	real_filename = follow_symlinks (filename, error);

	if (!real_filename)
		goto out;

	/* Get the directory in which the real filename lives */

	slashpos = strrchr (real_filename, G_DIR_SEPARATOR);

	if (slashpos)
	{
		dirname = g_strdup (real_filename);
		dirname[slashpos - real_filename + 1] = '\0';
	}
	else
		dirname = g_strdup (".");

	/* If there is not an existing file with that name, compute the
	 * permissions and uid/gid that we will use for the newly-created file.
	 */

	if (stat (real_filename, &st) != 0)
	{
		struct stat dir_st;
		int result;

		/* File does not exist? */
		create_backup_copy = FALSE;

		/* Use default permissions */
		saved_umask = umask(0);
		st.st_mode = 0666 & ~saved_umask;
		umask(saved_umask);
		st.st_uid = getuid ();

		result = stat (dirname, &dir_st);

		if (result == 0 && (dir_st.st_mode & S_ISGID))
			st.st_gid = dir_st.st_gid;
		else
			st.st_gid = getgid ();
	}

	/* Save to a temporary file.  We set the umask because some (buggy)
	 * implementations of mkstemp() use permissions 0666 and we want 0600.
	 */

	temp_filename = g_build_filename (dirname, ".gedit-save-XXXXXX", NULL);
	g_free (dirname);

	saved_umask = umask (0077);
	fd = g_mkstemp (temp_filename); /* this modifies temp_filename to the used name */
	umask (saved_umask);

	if (fd == -1)
	{
		g_set_error (error, GEDIT_DOCUMENT_IO_ERROR, errno, " ");
		goto out;
	}

	chars = gedit_document_get_chars (doc, 0, -1);

	encoding_setting = gedit_prefs_manager_get_save_encoding ();

	if (encoding_setting == GEDIT_SAVE_CURRENT_LOCALE_IF_POSSIBLE)
	{
		GError *conv_error = NULL;
		gchar* converted_file_contents = NULL;

		gedit_debug (DEBUG_DOCUMENT, "Using current locale's encoding");

		converted_file_contents = g_locale_from_utf8 (chars, -1, NULL, NULL, &conv_error);

		if (conv_error != NULL)
		{
			/* Conversion error */
			g_error_free (conv_error);
		}
		else
		{
			g_free (chars);
			chars = converted_file_contents;
		}
	}
	else
	{
		if ((doc->priv->encoding != NULL) &&
		    ((encoding_setting == GEDIT_SAVE_ORIGINAL_FILE_ENCODING_IF_POSSIBLE) ||
		     (encoding_setting == GEDIT_SAVE_ORIGINAL_FILE_ENCODING_IF_POSSIBLE_NCL)))
		{
			GError *conv_error = NULL;
			gchar* converted_file_contents = NULL;

			gedit_debug (DEBUG_DOCUMENT, "Using encoding %s", doc->priv->encoding);

			/* Try to convert it from UTF-8 to original encoding */
			converted_file_contents = g_convert (chars, -1, 
							     doc->priv->encoding, "UTF-8", 
							     NULL, NULL, &conv_error); 

			if (conv_error != NULL)
			{
				/* Conversion error */
				g_error_free (conv_error);
			}
			else
			{
				g_free (chars);
				chars = converted_file_contents;
			}
		}
		else
			gedit_debug (DEBUG_DOCUMENT, "Using UTF-8 (Null)");

	}	    

	chars_len = strlen (chars);

	add_cr = (*(chars + chars_len - 1) != '\n');

	/* Save the file content */
	res = (write (fd, chars, chars_len) == chars_len);

	if (res && add_cr)
		/* Add \n if needed */
		res = (write (fd, "\n", 1) == 1);

	if (!res)
	{
		gchar *msg;

		switch (errno)
		{
			case ENOSPC:
				msg = _("There is not enough disk space to save the file.\n"
					"Please free some disk space and try again.");
				break;

			case EFBIG:
				msg = _("The disk where you are trying to save the file has "
					"a limitation on file sizes.  Please try saving "
					"a smaller file or saving it to a disk that does not "
					"have this limitation.");
				break;

			default:
				msg = " ";
				break;
		}

		g_set_error (error, GEDIT_DOCUMENT_IO_ERROR, errno, msg);
		close (fd);
		unlink (temp_filename);
		goto out;
	}

	if (close (fd) != 0)
	{
		g_set_error (error, GEDIT_DOCUMENT_IO_ERROR, errno, " ");
		unlink (temp_filename);
		goto out;
	}

	/* Move the original file to a backup */

	if (create_backup_copy)
	{
		gchar *bak_ext;
		gint result;

		bak_ext = gedit_prefs_manager_get_backup_extension ();
		
		backup_filename = g_strconcat (real_filename, 
					       bak_ext,
					       NULL);

		g_free (bak_ext);
		
		result = rename (real_filename, backup_filename);

		if (result != 0)
		{
			g_set_error (error, GEDIT_DOCUMENT_IO_ERROR, errno,
				     _("Could not create a backup file."));
			unlink (temp_filename);
			goto out;
		}
	}

	/* Move the temp file to the original file */

	if (rename (temp_filename, real_filename) != 0)
	{
		gint saved_errno;

		saved_errno = errno;

		if (create_backup_copy && rename (backup_filename, real_filename) != 0)
			g_set_error (error, GEDIT_DOCUMENT_IO_ERROR, errno,
				     " ");
		else
			g_set_error (error, GEDIT_DOCUMENT_IO_ERROR, saved_errno,
				     " ");

		goto out;
	}

	/* Restore permissions.  There is not much error checking we can do
	 * here, I'm afraid.  The final data is saved anyways.
	 */

	chmod (real_filename, st.st_mode);
	chown (real_filename, st.st_uid, st.st_gid);

	gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (doc), FALSE);

	retval = TRUE;

	/* Done */

 out:

	g_free (chars);

	g_free (filename);
	g_free (real_filename);
	g_free (backup_filename);
	g_free (temp_filename);

	return retval;
}

gboolean	
gedit_document_is_untitled (const GeditDocument* doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (doc != NULL, FALSE);
	g_return_val_if_fail (doc->priv != NULL, FALSE);

	return (doc->priv->uri == NULL);
}

gboolean	
gedit_document_get_modified (const GeditDocument* doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (doc != NULL, FALSE);

	return gtk_text_buffer_get_modified (GTK_TEXT_BUFFER (doc));
}

/**
 * gedit_document_get_char_count:
 * @doc: a #GeditDocument 
 * 
 * Gets the number of characters in the buffer; note that characters
 * and bytes are not the same, you can't e.g. expect the contents of
 * the buffer in string form to be this many bytes long. 
 * 
 * Return value: number of characters in the document
 **/
gint 
gedit_document_get_char_count (const GeditDocument *doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (doc != NULL, FALSE);

	return gtk_text_buffer_get_char_count (GTK_TEXT_BUFFER (doc));
}

/**
 * gedit_document_get_line_count:
 * @doc: a #GeditDocument 
 * 	
 * Obtains the number of lines in the buffer. 
 * 
 * Return value: number of lines in the document
 **/
gint 
gedit_document_get_line_count (const GeditDocument *doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (doc != NULL, FALSE);

	return gtk_text_buffer_get_line_count (GTK_TEXT_BUFFER (doc));
}

gboolean 
gedit_document_revert (GeditDocument *doc,  GError **error)
{
	gchar* buffer = NULL;
	
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (doc != NULL, FALSE);

	if (gedit_document_is_untitled (doc))
	{
		g_set_error (error, GEDIT_DOCUMENT_IO_ERROR, GEDIT_ERROR_UNTITLED,
			_("It is not possible to revert an Untitled document"));
		return FALSE;
	}
			
	buffer = gedit_document_get_chars (doc, 0, -1);

	gedit_undo_manager_begin_not_undoable_action (doc->priv->undo_manager);

	gedit_document_delete_text (doc, 0, -1);

	if (!gedit_document_load (doc, doc->priv->uri, error))
	{
		GtkTextIter iter;
		
		/* Insert text in the buffer */
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &iter, 0);
		gtk_text_buffer_insert (GTK_TEXT_BUFFER (doc), &iter, buffer, -1);

		g_free (buffer);

		gedit_undo_manager_end_not_undoable_action (doc->priv->undo_manager);

		return FALSE;
	}

	gedit_undo_manager_end_not_undoable_action (doc->priv->undo_manager);

	g_free (buffer);

	return TRUE;
}

void 
gedit_document_insert_text (GeditDocument *doc, gint pos, const gchar *text, gint len)
{
	GtkTextIter iter;
	
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));
	g_return_if_fail (pos >= 0);
	g_return_if_fail (text != NULL);
	g_return_if_fail (g_utf8_validate (text, len, NULL));

	gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &iter, pos);
	
	gtk_text_buffer_insert (GTK_TEXT_BUFFER (doc), &iter, text, len);
}

void 
gedit_document_insert_text_at_cursor (GeditDocument *doc, const gchar *text, gint len)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));
	g_return_if_fail (text != NULL);
	g_return_if_fail (g_utf8_validate (text, len, NULL));

	gtk_text_buffer_insert_at_cursor (GTK_TEXT_BUFFER (doc), text, len);
}


void 
gedit_document_delete_text (GeditDocument *doc, gint start, gint end)
{
	GtkTextIter start_iter;
	GtkTextIter end_iter;

	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));
	g_return_if_fail (start >= 0);
	g_return_if_fail ((end > start) || end < 0);

	gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &start_iter, start);

	if (end < 0)
		gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (doc), &end_iter);
	else
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &end_iter, end);

	gtk_text_buffer_delete (GTK_TEXT_BUFFER (doc), &start_iter, &end_iter);
}

gchar*
gedit_document_get_chars (GeditDocument *doc, gint start, gint end)
{
	GtkTextIter start_iter;
	GtkTextIter end_iter;

	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), NULL);
	g_return_val_if_fail (start >= 0, NULL);
	g_return_val_if_fail ((end > start) || (end < 0), NULL);

	gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &start_iter, start);

	if (end < 0)
		gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (doc), &end_iter);
	else
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &end_iter, end);

	return gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (doc), &start_iter, &end_iter, TRUE);
}


gboolean
gedit_document_can_undo	(const GeditDocument *doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), FALSE);
	g_return_val_if_fail (doc->priv != NULL, FALSE);

	return gedit_undo_manager_can_undo (doc->priv->undo_manager);
}

gboolean
gedit_document_can_redo (const GeditDocument *doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), FALSE);
	g_return_val_if_fail (doc->priv != NULL, FALSE);

	return gedit_undo_manager_can_redo (doc->priv->undo_manager);	
}

void 
gedit_document_undo (GeditDocument *doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));
	g_return_if_fail (doc->priv != NULL);
	g_return_if_fail (gedit_undo_manager_can_undo (doc->priv->undo_manager));

	gedit_undo_manager_undo (doc->priv->undo_manager);
}

void 
gedit_document_redo (GeditDocument *doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));
	g_return_if_fail (doc->priv != NULL);
	g_return_if_fail (gedit_undo_manager_can_redo (doc->priv->undo_manager));

	gedit_undo_manager_redo (doc->priv->undo_manager);
}

void
gedit_document_begin_not_undoable_action (GeditDocument *doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));
	g_return_if_fail (doc->priv != NULL);

	gedit_undo_manager_begin_not_undoable_action (doc->priv->undo_manager);
}

void	
gedit_document_end_not_undoable_action	(GeditDocument *doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));
	g_return_if_fail (doc->priv != NULL);

	gedit_undo_manager_end_not_undoable_action (doc->priv->undo_manager);
}

void		
gedit_document_begin_user_action (GeditDocument *doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));

	gtk_text_buffer_begin_user_action (GTK_TEXT_BUFFER (doc));
}	

void		
gedit_document_end_user_action (GeditDocument *doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));

	gtk_text_buffer_end_user_action (GTK_TEXT_BUFFER (doc));
}


static void
gedit_document_can_undo_handler (GeditUndoManager* um, gboolean can_undo, GeditDocument* doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));

	g_signal_emit (G_OBJECT (doc),
                       document_signals [CAN_UNDO],
                       0,
		       can_undo);
}

static void
gedit_document_can_redo_handler (GeditUndoManager* um, gboolean can_redo, GeditDocument* doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));

	g_signal_emit (G_OBJECT (doc),
                       document_signals [CAN_REDO],
                       0,
		       can_redo);
}

void
gedit_document_goto_line (GeditDocument* doc, guint line)
{
	guint line_count;
	GtkTextIter iter;
	
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));
	g_return_if_fail (doc->priv != NULL);
	
	line_count = gtk_text_buffer_get_line_count (GTK_TEXT_BUFFER (doc));
	
	if (line > line_count)
		line = line_count;

	gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (doc), &iter, line);
	gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (doc), &iter);
}

gchar* 
gedit_document_get_last_searched_text (GeditDocument* doc)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), FALSE);
	g_return_val_if_fail (doc->priv != NULL, FALSE);
	
	return doc->priv->last_searched_text != NULL ? 
		g_strdup (doc->priv->last_searched_text) : NULL;	
}

gchar*
gedit_document_get_last_replace_text (GeditDocument* doc)
{
	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), FALSE);
	g_return_val_if_fail (doc->priv != NULL, FALSE);

	return doc->priv->last_replace_text != NULL ? 
		g_strdup (doc->priv->last_replace_text) : NULL;	
}

void 
gedit_document_set_last_searched_text (GeditDocument* doc, const gchar *text)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));
	g_return_if_fail (doc->priv != NULL);
	g_return_if_fail (text != NULL);

	if (doc->priv->last_searched_text != NULL)
		g_free (doc->priv->last_searched_text);

	doc->priv->last_searched_text = g_strdup (text);
}

void 
gedit_document_set_last_replace_text (GeditDocument* doc, const gchar *text)
{
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));
	g_return_if_fail (doc->priv != NULL);
	g_return_if_fail (text != NULL);

	if (doc->priv->last_replace_text != NULL)
		g_free (doc->priv->last_replace_text);

	doc->priv->last_replace_text = g_strdup (text);
}

gboolean
gedit_document_find (GeditDocument* doc, const gchar* str, 
		gboolean from_cursor, gboolean case_sensitive, gboolean entire_word)
{
	GtkTextIter iter;
	gboolean found = FALSE;
	GtkTextSearchFlags search_flags;
	gchar *converted_str;

	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), FALSE);
	g_return_val_if_fail (doc->priv != NULL, FALSE);
	g_return_val_if_fail (str != NULL, FALSE);

	converted_str = gedit_utils_convert_search_text (str);
	g_return_val_if_fail (converted_str != NULL, FALSE);
	
	gedit_debug (DEBUG_DOCUMENT, "str: %s", str);
	gedit_debug (DEBUG_DOCUMENT, "converted_str: %s", converted_str);

	search_flags = GTK_TEXT_SEARCH_VISIBLE_ONLY | GTK_TEXT_SEARCH_TEXT_ONLY;
	
	if (!case_sensitive)
	{
		search_flags = search_flags | GTK_TEXT_SEARCH_CASE_INSENSITIVE;
	}

	if (from_cursor)
	{
		GtkTextIter sel_bound;
		
		gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc),			
                                    &iter,
                                    gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (doc),
					                      "insert"));
		
		gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc),			
                                    &sel_bound,
                                    gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (doc),
					                      "selection_bound"));
		
		gtk_text_iter_order (&sel_bound, &iter);		
	}
	else		
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &iter, 0);

	found = FALSE;
	
	if (*converted_str != '\0')
    	{
      		GtkTextIter match_start, match_end;

		while (!found)
		{
          		found = gedit_text_iter_forward_search (&iter, converted_str, search_flags,
                        	                	&match_start, &match_end,
                                	               	NULL);	

			if (found && entire_word)
			{
				found = gtk_text_iter_starts_word (&match_start) && 
					gtk_text_iter_ends_word (&match_end);

				if (!found) 
					iter = match_end;

			}
			else
				break;
		}
		
		if (found)
		{
			gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (doc),
					&match_start);

			gtk_text_buffer_move_mark_by_name (GTK_TEXT_BUFFER (doc),
					"selection_bound", &match_end);
		}

		if (doc->priv->last_searched_text)
			g_free (doc->priv->last_searched_text);

		doc->priv->last_searched_text = g_strdup (str);
		doc->priv->last_search_was_case_sensitive = case_sensitive;
		doc->priv->last_search_was_entire_word = entire_word;
	}

	g_free (converted_str);

	return found;
}

gboolean
gedit_document_find_again (GeditDocument* doc, gboolean from_cursor)
{
	gchar* last_searched_text;
	gboolean found;
	
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), FALSE);
	g_return_val_if_fail (doc->priv != NULL, FALSE);

	last_searched_text = gedit_document_get_last_searched_text (doc);
	
	if (last_searched_text == NULL)
		return FALSE;
	
	found = gedit_document_find (doc, 
				     last_searched_text, 
				     from_cursor, 
				     doc->priv->last_search_was_case_sensitive, 
				     doc->priv->last_search_was_entire_word);

	g_free (last_searched_text);

	return found;
}

gboolean 
gedit_document_get_selection (GeditDocument *doc, gint *start, gint *end)
{
	GtkTextIter iter;
	GtkTextIter sel_bound;

	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), FALSE);

	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc),			
                                    &iter,
                                    gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (doc),
					                      "insert"));
		
	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc),			
                                    &sel_bound,
                                    gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (doc),
					                      "selection_bound"));
	gtk_text_iter_order (&iter, &sel_bound);	

	if (start != NULL)
		*start = gtk_text_iter_get_offset (&iter); 

	if (end != NULL)
		*end = gtk_text_iter_get_offset (&sel_bound); 

	return !gtk_text_iter_equal (&sel_bound, &iter);	
}

void
gedit_document_replace_selected_text (GeditDocument *doc, const gchar *replace)
{
	GtkTextIter iter;
	GtkTextIter sel_bound;
	gchar *converted_replace;

	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));
	g_return_if_fail (replace != NULL);	

	converted_replace = gedit_utils_convert_search_text (replace);

	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc),			
                                    &iter,
                                    gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (doc),
					                      "insert"));
		
	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc),			
                                    &sel_bound,
                                    gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (doc),
					                      "selection_bound"));

	if (gtk_text_iter_equal (&sel_bound, &iter))
	{
		gedit_debug (DEBUG_DOCUMENT, "There is no selected text");

		return;
	}
	
	gtk_text_iter_order (&sel_bound, &iter);		

	gedit_document_begin_user_action (doc);

	gtk_text_buffer_delete (GTK_TEXT_BUFFER (doc),
				&iter,
				&sel_bound);

	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc),			
                                    &iter,
                                    gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (doc),
					                      "insert"));
	if (*converted_replace != '\0')
		gtk_text_buffer_insert (GTK_TEXT_BUFFER (doc),
					&iter,
					converted_replace, strlen (converted_replace));

	if (doc->priv->last_replace_text != NULL)
		g_free (doc->priv->last_replace_text);

	doc->priv->last_replace_text = g_strdup (replace);

	gedit_document_end_user_action (doc);
	g_free (converted_replace);
}

gboolean
gedit_document_replace_all (GeditDocument *doc,
	      	const gchar *find, const gchar *replace, gboolean case_sensitive, gboolean entire_word)
{
	gboolean from_cursor = FALSE;
	gboolean cont = 0;

	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), FALSE);
	g_return_val_if_fail (find != NULL, FALSE);
	g_return_val_if_fail (replace != NULL, FALSE);

	gedit_document_begin_user_action (doc);

	while (gedit_document_find (doc, find, from_cursor, case_sensitive, entire_word)) 
	{
		gedit_document_replace_selected_text (doc, replace);
		
		from_cursor = TRUE;
		++cont;
	}

	gedit_document_end_user_action (doc);

	return cont;
}

guint
gedit_document_get_line_at_offset (const GeditDocument *doc, guint offset)
{
	GtkTextIter iter;
	
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), 0);

	gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &iter, offset);
	
	return gtk_text_iter_get_line (&iter);
}

gint gedit_document_get_cursor (GeditDocument *doc)
{
	GtkTextIter iter;
	
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_val_if_fail (GEDIT_IS_DOCUMENT (doc), 0);

	gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (doc),			
                                    &iter,
                                    gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (doc),
					                      "insert"));

	return gtk_text_iter_get_offset (&iter); 
}

void
gedit_document_set_cursor (GeditDocument *doc, gint cursor)
{
	GtkTextIter iter;
	
	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));
	
	/* Place the cursor at the requested position */
	gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &iter, cursor);
	gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (doc), &iter);
}

void
gedit_document_set_selection (GeditDocument *doc, gint start, gint end)
{
 	GtkTextIter start_iter;
	GtkTextIter end_iter;

	gedit_debug (DEBUG_DOCUMENT, "");

	g_return_if_fail (GEDIT_IS_DOCUMENT (doc));
	g_return_if_fail (start >= 0);
	g_return_if_fail ((end > start) || (end < 0));

	gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &start_iter, start);

	if (end < 0)
		gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (doc), &end_iter);
	else
		gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (doc), &end_iter, end);

	gtk_text_buffer_place_cursor (GTK_TEXT_BUFFER (doc), &end_iter);

	gtk_text_buffer_move_mark_by_name (GTK_TEXT_BUFFER (doc),
					"selection_bound", &start_iter);
}