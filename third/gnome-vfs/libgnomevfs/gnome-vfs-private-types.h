/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-private-types.h - Declaration of private types for the
   GNOME Virtual File System.

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@gnu.org> */

#ifndef GNOME_VFS_PRIVATE_TYPES_H
#define GNOME_VFS_PRIVATE_TYPES_H

#include <glib.h>

/* Opaque types.  */

typedef struct GnomeVFSContext GnomeVFSContext;
typedef struct GnomeVFSCancellation GnomeVFSCancellation;
typedef struct GnomeVFSIOBuf GnomeVFSIOBuf;
typedef struct GnomeVFSInetConnection GnomeVFSInetConnection;
typedef struct GnomeVFSMessageCallbacks GnomeVFSMessageCallbacks;
typedef gpointer GnomeVFSMethodHandle;


/* VFS methods.  */

typedef GnomeVFSMethod * (* GnomeVFSMethodInitFunc)(const char *method_name, const char *config_args);
typedef void (*GnomeVFSMethodShutdownFunc)(GnomeVFSMethod *method);

typedef GnomeVFSResult (* GnomeVFSMethodOpenFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle
			       	 	**method_handle_return,
					 GnomeVFSURI *uri,
					 GnomeVFSOpenMode mode,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodCreateFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle
			       	 	**method_handle_return,
					 GnomeVFSURI *uri,
					 GnomeVFSOpenMode mode,
					 gboolean exclusive,
					 guint perm,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodCloseFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodReadFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 gpointer buffer,
					 GnomeVFSFileSize num_bytes,
					 GnomeVFSFileSize *bytes_read_return,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodWriteFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 gconstpointer buffer,
					 GnomeVFSFileSize num_bytes,
					 GnomeVFSFileSize *bytes_written_return,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodSeekFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 GnomeVFSSeekPosition  whence,
					 GnomeVFSFileOffset    offset,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodTellFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 GnomeVFSFileOffset *offset_return);

typedef GnomeVFSResult (* GnomeVFSMethodOpenDirectoryFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle **method_handle,
					 GnomeVFSURI *uri,
					 GnomeVFSFileInfoOptions options,
					 const GnomeVFSDirectoryFilter *filter,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodCloseDirectoryFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodReadDirectoryFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 GnomeVFSFileInfo *file_info,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodGetFileInfoFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSURI *uri,
					 GnomeVFSFileInfo *file_info,
					 GnomeVFSFileInfoOptions options,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodGetFileInfoFromHandleFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 GnomeVFSFileInfo *file_info,
					 GnomeVFSFileInfoOptions options,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodTruncateFunc) (GnomeVFSMethod *method,
						       GnomeVFSURI *uri,
						       GnomeVFSFileSize length,
						       GnomeVFSContext *context);
typedef GnomeVFSResult (* GnomeVFSMethodTruncateHandleFunc) (GnomeVFSMethod *method,
							     GnomeVFSMethodHandle *handle,
							     GnomeVFSFileSize length,
							     GnomeVFSContext *context);

typedef gboolean       (* GnomeVFSMethodIsLocalFunc)
					(GnomeVFSMethod *method,
					 const GnomeVFSURI *uri);

typedef GnomeVFSResult (* GnomeVFSMethodMakeDirectoryFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSURI *uri,
					 guint perm,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodFindDirectoryFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSURI *find_near_uri,
					 GnomeVFSFindDirectoryKind kind,
					 GnomeVFSURI **result_uri,
					 gboolean create_if_needed,
					 gboolean find_if_needed,
					 guint perm,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodRemoveDirectoryFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSURI *uri,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodMoveFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSURI *old_uri,
					 GnomeVFSURI *new_uri,
					 gboolean force_replace,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodUnlinkFunc)
                                        (GnomeVFSMethod *method,
					 GnomeVFSURI *uri,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodCheckSameFSFunc)
					(GnomeVFSMethod *method,
					 GnomeVFSURI *a,
					 GnomeVFSURI *b,
					 gboolean *same_fs_return,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodSetFileInfo)
					(GnomeVFSMethod *method,
					 GnomeVFSURI *a,
					 const GnomeVFSFileInfo *info,
					 GnomeVFSSetFileInfoMask mask,
					 GnomeVFSContext *context);

typedef GnomeVFSResult (* GnomeVFSMethodCreateSymbolicLinkFunc)
                                        (GnomeVFSMethod *method,
                                         GnomeVFSURI *uri,
                                         const gchar *target_reference,
                                         GnomeVFSContext *context);


/* Structure defining an access method.	 This is also defined as an
   opaque type in `gnome-vfs-types.h'.	*/
struct GnomeVFSMethod {
	GnomeVFSMethodOpenFunc open;
	GnomeVFSMethodCreateFunc create;
	GnomeVFSMethodCloseFunc close;
	GnomeVFSMethodReadFunc read;
	GnomeVFSMethodWriteFunc write;
	GnomeVFSMethodSeekFunc seek;
	GnomeVFSMethodTellFunc tell;
	GnomeVFSMethodTruncateHandleFunc truncate_handle;
	GnomeVFSMethodOpenDirectoryFunc open_directory;
	GnomeVFSMethodCloseDirectoryFunc close_directory;
	GnomeVFSMethodReadDirectoryFunc read_directory;
	GnomeVFSMethodGetFileInfoFunc get_file_info;
	GnomeVFSMethodGetFileInfoFromHandleFunc get_file_info_from_handle;
	GnomeVFSMethodIsLocalFunc is_local;
	GnomeVFSMethodMakeDirectoryFunc make_directory;
	GnomeVFSMethodRemoveDirectoryFunc remove_directory;
	GnomeVFSMethodMoveFunc move;
	GnomeVFSMethodUnlinkFunc unlink;
	GnomeVFSMethodCheckSameFSFunc check_same_fs;
	GnomeVFSMethodSetFileInfo set_file_info;
	GnomeVFSMethodTruncateFunc truncate;
	GnomeVFSMethodFindDirectoryFunc find_directory;
	GnomeVFSMethodCreateSymbolicLinkFunc create_symbolic_link;
};



/* VFS Transform */

typedef struct GnomeVFSTransform GnomeVFSTransform;
typedef GnomeVFSTransform * (* GnomeVFSTransformInitFunc)(const char *method_name, const char *config_args);

typedef GnomeVFSResult (* GnomeVFSTransformFunc) (GnomeVFSTransform *transform,
						  const gchar *old_uri,
						  gchar **new_uri,
						  GnomeVFSContext *context);

struct GnomeVFSTransform {
	GnomeVFSTransformFunc transform;
};

typedef struct GnomeVFSProgressCallbackState {

	/* xfer state */
	GnomeVFSXferProgressInfo *progress_info;	

	/* Callback called for every xfer operation. For async calls called 
	   in async xfer context. */
	GnomeVFSXferProgressCallback sync_callback;

	/* Callback called periodically every few hundred miliseconds
	   and whenever user interaction is needed. For async calls
	   called in the context of the async call caller. */
	GnomeVFSXferProgressCallback update_callback;

	/* User data passed to sync_callback. */
	gpointer user_data;

	/* Async job state passed to the update callback. */
	gpointer async_job_data;

	/* When will update_callback be called next. */
	gint64 next_update_callback_time;

	/* When will update_callback be called next. */
	gint64 next_text_update_callback_time;

	/* Period at which the update_callback will be called. */
	gint64 update_callback_period;
} GnomeVFSProgressCallbackState;


typedef struct GnomeVFSShellpatternFilter GnomeVFSShellpatternFilter;
typedef struct GnomeVFSRegexpFilter GnomeVFSRegexpFilter;

#endif /* _GNOME_VFS_PRIVATE_TYPES_H */
