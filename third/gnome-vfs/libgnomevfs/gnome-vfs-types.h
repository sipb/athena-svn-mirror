/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-types.h - Types used by the GNOME Virtual File System.

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

   Author: Ettore Perazzoli <ettore@gnu.org>
*/

#ifndef GNOME_VFS_TYPES_H
#define GNOME_VFS_TYPES_H

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <glib.h>

/*
 * This defines GnomeVFSFileSize and GnomeVFSFileOffset
 *
 * It also defines GNOME_VFS_SIZE_IS_<type> and GNOME_VFS_OFFSET_IS_<type>
 * where type is INT, UNSIGNED_INT, LONG, UNSIGNED_LONG, LONG_LONG
 * or UNSIGNED_LONG_LONG.  Note that size is always unsigned and offset
 * is always signed.
 *
 * It also defines GNOME_VFS_SIZE_FORMAT_STR and GNOME_VFS_OFFSET_FORMAT_STR
 * which is the string representation to be used in printf style expressions.
 * This is without the %, so for example for long it would be "ld"
 */
#include <libgnomevfs/gnome-vfs-file-size.h>

/* Basic enumerations.  */

/* IMPORTANT NOTICE: If you add error types here, please also add the
   corresponsing descriptions in `gnome-vfs-result.c'.  Moreover, *always* add
   new values at the end of the list, and *never* remove values.  */
typedef enum {
	GNOME_VFS_OK,
	GNOME_VFS_ERROR_NOT_FOUND,
	GNOME_VFS_ERROR_GENERIC,
	GNOME_VFS_ERROR_INTERNAL,
	GNOME_VFS_ERROR_BAD_PARAMETERS,
	GNOME_VFS_ERROR_NOT_SUPPORTED,
	GNOME_VFS_ERROR_IO,
	GNOME_VFS_ERROR_CORRUPTED_DATA,
	GNOME_VFS_ERROR_WRONG_FORMAT,
	GNOME_VFS_ERROR_BAD_FILE,
	GNOME_VFS_ERROR_TOO_BIG,
	GNOME_VFS_ERROR_NO_SPACE,
	GNOME_VFS_ERROR_READ_ONLY,
	GNOME_VFS_ERROR_INVALID_URI,
	GNOME_VFS_ERROR_NOT_OPEN,
	GNOME_VFS_ERROR_INVALID_OPEN_MODE,
	GNOME_VFS_ERROR_ACCESS_DENIED,
	GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES,
	GNOME_VFS_ERROR_EOF,
	GNOME_VFS_ERROR_NOT_A_DIRECTORY,
	GNOME_VFS_ERROR_IN_PROGRESS,
	GNOME_VFS_ERROR_INTERRUPTED,
	GNOME_VFS_ERROR_FILE_EXISTS,
	GNOME_VFS_ERROR_LOOP,
	GNOME_VFS_ERROR_NOT_PERMITTED,
	GNOME_VFS_ERROR_IS_DIRECTORY,
	GNOME_VFS_ERROR_NO_MEMORY,
	GNOME_VFS_ERROR_HOST_NOT_FOUND,
	GNOME_VFS_ERROR_INVALID_HOST_NAME,
	GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS,
	GNOME_VFS_ERROR_LOGIN_FAILED,
	GNOME_VFS_ERROR_CANCELLED,
	GNOME_VFS_ERROR_DIRECTORY_BUSY,
	GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY,
	GNOME_VFS_ERROR_TOO_MANY_LINKS,
	GNOME_VFS_ERROR_READ_ONLY_FILE_SYSTEM,
	GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM,
	GNOME_VFS_ERROR_NAME_TOO_LONG,
	GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE,
	GNOME_VFS_ERROR_SERVICE_OBSOLETE,
	GNOME_VFS_NUM_ERRORS
} GnomeVFSResult;

/* Open mode.  If you don't set `GNOME_VFS_OPEN_RANDOM', you have to access the
   file sequentially.  */
typedef enum {
	GNOME_VFS_OPEN_NONE = 0,
	GNOME_VFS_OPEN_READ = 1 << 0,
	GNOME_VFS_OPEN_WRITE = 1 << 1,
	GNOME_VFS_OPEN_RANDOM = 1 << 2
} GnomeVFSOpenMode;

/* The file type.  */
typedef enum {
	GNOME_VFS_FILE_TYPE_UNKNOWN,
	GNOME_VFS_FILE_TYPE_REGULAR,
	GNOME_VFS_FILE_TYPE_DIRECTORY,
	GNOME_VFS_FILE_TYPE_FIFO,
	GNOME_VFS_FILE_TYPE_SOCKET,
	GNOME_VFS_FILE_TYPE_CHARACTER_DEVICE,
	GNOME_VFS_FILE_TYPE_BLOCK_DEVICE,
	GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK
} GnomeVFSFileType;

/* File permissions.  These are the same as the Unix ones, but we wrap them
   into a nicer VFS-like enum.  */
typedef enum {
	GNOME_VFS_PERM_SUID = S_ISUID,
	GNOME_VFS_PERM_SGID = S_ISGID,	
	GNOME_VFS_PERM_STICKY = 01000,	/* S_ISVTX not defined on all systems */
	GNOME_VFS_PERM_USER_READ = S_IRUSR,
	GNOME_VFS_PERM_USER_WRITE = S_IWUSR,
	GNOME_VFS_PERM_USER_EXEC = S_IXUSR,
	GNOME_VFS_PERM_USER_ALL = S_IRUSR | S_IWUSR | S_IXUSR,
	GNOME_VFS_PERM_GROUP_READ = S_IRGRP,
	GNOME_VFS_PERM_GROUP_WRITE = S_IWGRP,
	GNOME_VFS_PERM_GROUP_EXEC = S_IXGRP,
	GNOME_VFS_PERM_GROUP_ALL = S_IRGRP | S_IWGRP | S_IXGRP,
	GNOME_VFS_PERM_OTHER_READ = S_IROTH,
	GNOME_VFS_PERM_OTHER_WRITE = S_IWOTH,
	GNOME_VFS_PERM_OTHER_EXEC = S_IXOTH,
	GNOME_VFS_PERM_OTHER_ALL = S_IROTH | S_IWOTH | S_IXOTH
} GnomeVFSFilePermissions;

/* This is used to specify the start position for seek operations.  */
typedef enum {
	GNOME_VFS_SEEK_START,
	GNOME_VFS_SEEK_CURRENT,
	GNOME_VFS_SEEK_END
} GnomeVFSSeekPosition;

/* Basic types.  */

/* A file handle.  */
typedef struct GnomeVFSHandle GnomeVFSHandle;

/* Structure describing an access method.  */
typedef struct GnomeVFSMethod GnomeVFSMethod;

/* This describes a URI element.  */
typedef struct GnomeVFSURI {
	/* Reference count.  */
	guint ref_count;

	/* Text for the element: eg. some/path/name.  */
	gchar *text;

	/* Text for the element: eg. some/path/name.  */
	gchar *fragment_id;
	
	/* Method string: eg. `gzip', `tar', `http'.  This is necessary as
	   one GnomeVFSMethod can be used for different method strings
	   (e.g. extfs handles zip, rar, zoo and several other ones).  */
	gchar *method_string;

	/* VFS method to access the element.  */
	GnomeVFSMethod *method;

	/* Pointer to the parent element, or NULL for toplevel elements.  */
	struct GnomeVFSURI *parent;
} GnomeVFSURI;

/* This is the toplevel URI element.  A toplevel method implementations should
   cast the `GnomeVFSURI' argument to this type to get the additional host/auth
   information.  If any of the elements is 0, it is unspecified.  */
typedef struct {
	/* Base object.  */
	GnomeVFSURI uri;

	/* Server location information.  */
	gchar *host_name;
	guint host_port;

	/* Authorization information.  */
	gchar *user_name;
	gchar *password;

	/* The parent URN, if it exists */
	gchar *urn;
} GnomeVFSToplevelURI;

/* This is used for hiding information when transforming the GnomeVFSURI into a
   string.  */
typedef enum {
	GNOME_VFS_URI_HIDE_NONE = 0,
	GNOME_VFS_URI_HIDE_USER_NAME = 1 << 0,
	GNOME_VFS_URI_HIDE_PASSWORD = 1 << 1,
	GNOME_VFS_URI_HIDE_HOST_NAME = 1 << 2,
	GNOME_VFS_URI_HIDE_HOST_PORT = 1 << 3,
	GNOME_VFS_URI_HIDE_TOPLEVEL_METHOD = 1 << 4,
	GNOME_VFS_URI_HIDE_FRAGMENT_IDENTIFIER = 1 << 8
} GnomeVFSURIHideOptions;



/* File flags.  */
typedef enum {
	GNOME_VFS_FILE_FLAGS_NONE = 0,
	/* Whether the file is a symlink.  */
	GNOME_VFS_FILE_FLAGS_SYMLINK = 1 << 0,
	/* Whether the file is on a local file system.  */
	GNOME_VFS_FILE_FLAGS_LOCAL = 1 << 1,
} GnomeVFSFileFlags;

/* Flags indicating what fields in a GnomeVFSFileInfo struct are valid. 
   Name is always assumed valid (how else would you have gotten a
   FileInfo struct otherwise?)
 */

typedef enum {
	GNOME_VFS_FILE_INFO_FIELDS_NONE = 0,
	GNOME_VFS_FILE_INFO_FIELDS_TYPE = 1 << 0,
	GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS = 1 << 1,
	GNOME_VFS_FILE_INFO_FIELDS_FLAGS = 1 << 2,
	GNOME_VFS_FILE_INFO_FIELDS_DEVICE = 1 << 3,
	GNOME_VFS_FILE_INFO_FIELDS_INODE = 1 << 4,
	GNOME_VFS_FILE_INFO_FIELDS_LINK_COUNT = 1 << 5,
	GNOME_VFS_FILE_INFO_FIELDS_SIZE = 1 << 6,
	GNOME_VFS_FILE_INFO_FIELDS_BLOCK_COUNT = 1 << 7,
	GNOME_VFS_FILE_INFO_FIELDS_IO_BLOCK_SIZE = 1 << 8,
	GNOME_VFS_FILE_INFO_FIELDS_ATIME = 1 << 9,
	GNOME_VFS_FILE_INFO_FIELDS_MTIME = 1 << 10,
	GNOME_VFS_FILE_INFO_FIELDS_CTIME = 1 << 11,
	GNOME_VFS_FILE_INFO_FIELDS_SYMLINK_NAME = 1 << 12,
	GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE = 1 << 13
} GnomeVFSFileInfoFields;

typedef struct {
	/* Base name of the file (no path).  */
	gchar *name;

	/* Fields which are actually valid in this strcture. */
	GnomeVFSFileInfoFields valid_fields;

	/* File type (i.e. regular, directory, block device...).  */
	GnomeVFSFileType type;

	/* File permissions.  */
	GnomeVFSFilePermissions permissions;

	/* Flags for this file.  */
	GnomeVFSFileFlags flags;

	/* This is only valid if `is_local' is TRUE (see below).  */
	dev_t device;
	ino_t inode;

	/* Link count.  */
	guint link_count;

	/* UID, GID.  */
	guint uid;
	guint gid;

	/* Size in bytes.  */
	GnomeVFSFileSize size;

	/* Size measured in units of 512-byte blocks.  */
	GnomeVFSFileSize block_count;

	/* Optimal buffer size for reading/writing the file.  */
	guint io_block_size;

	/* Access, modification and change times.  */
	time_t atime;
	time_t mtime;
	time_t ctime;

	/* If the file is a symlink (see `flags'), this specifies the file the
           link points to.  */
	gchar *symlink_name;

	/* MIME type.  */
	gchar *mime_type;

	guint refcount;
} GnomeVFSFileInfo;

typedef enum {
	GNOME_VFS_FILE_INFO_DEFAULT = 0, /* FIXME bugzilla.eazel.com 1203: name sucks */
	GNOME_VFS_FILE_INFO_GET_MIME_TYPE = 1 << 0,
	GNOME_VFS_FILE_INFO_FORCE_FAST_MIME_TYPE = 1 << 1,
	GNOME_VFS_FILE_INFO_FORCE_SLOW_MIME_TYPE = 1 << 2,
	GNOME_VFS_FILE_INFO_FOLLOW_LINKS = 1 << 3
} GnomeVFSFileInfoOptions;

typedef enum {
	GNOME_VFS_SET_FILE_INFO_NONE = 0,
	GNOME_VFS_SET_FILE_INFO_NAME = 1 << 0,
	GNOME_VFS_SET_FILE_INFO_PERMISSIONS = 1 << 1,
	GNOME_VFS_SET_FILE_INFO_OWNER = 1 << 2,
	GNOME_VFS_SET_FILE_INFO_TIME = 1 << 3
} GnomeVFSSetFileInfoMask;

/* Directory stuff.  */

typedef enum {
	GNOME_VFS_DIRECTORY_KIND_DESKTOP = 1000,
	GNOME_VFS_DIRECTORY_KIND_TRASH = 1001
} GnomeVFSFindDirectoryKind;

typedef struct GnomeVFSDirectoryList GnomeVFSDirectoryList;

typedef gpointer GnomeVFSDirectoryListPosition;

#define GNOME_VFS_DIRECTORY_LIST_POSITION_NONE NULL

typedef enum {
	GNOME_VFS_DIRECTORY_SORT_NONE,
	GNOME_VFS_DIRECTORY_SORT_DIRECTORYFIRST,
	GNOME_VFS_DIRECTORY_SORT_BYNAME,
	GNOME_VFS_DIRECTORY_SORT_BYNAME_IGNORECASE,
	GNOME_VFS_DIRECTORY_SORT_BYSIZE,
	GNOME_VFS_DIRECTORY_SORT_BYBLOCKCOUNT,
	GNOME_VFS_DIRECTORY_SORT_BYATIME,
	GNOME_VFS_DIRECTORY_SORT_BYMTIME,
	GNOME_VFS_DIRECTORY_SORT_BYCTIME,
	GNOME_VFS_DIRECTORY_SORT_BYMIMETYPE
} GnomeVFSDirectorySortRule;

typedef enum {
	GNOME_VFS_DIRECTORY_FILTER_NONE,
	GNOME_VFS_DIRECTORY_FILTER_SHELLPATTERN,
	GNOME_VFS_DIRECTORY_FILTER_REGEXP
} GnomeVFSDirectoryFilterType;

typedef enum {
	GNOME_VFS_DIRECTORY_FILTER_DEFAULT = 0,
	GNOME_VFS_DIRECTORY_FILTER_NODIRS = 1 << 0,
	GNOME_VFS_DIRECTORY_FILTER_DIRSONLY = 1 << 1,
	GNOME_VFS_DIRECTORY_FILTER_NODOTFILES = 1 << 2,
	GNOME_VFS_DIRECTORY_FILTER_IGNORECASE = 1 << 3,
	GNOME_VFS_DIRECTORY_FILTER_EXTENDEDREGEXP =  1 << 4,
	GNOME_VFS_DIRECTORY_FILTER_NOSELFDIR = 1 << 5,
	GNOME_VFS_DIRECTORY_FILTER_NOPARENTDIR = 1 << 6,
	GNOME_VFS_DIRECTORY_FILTER_NOBACKUPFILES = 1 << 7
} GnomeVFSDirectoryFilterOptions;

typedef enum {
	GNOME_VFS_DIRECTORY_FILTER_NEEDS_NOTHING = 0,
	GNOME_VFS_DIRECTORY_FILTER_NEEDS_NAME = 1 << 0,
	GNOME_VFS_DIRECTORY_FILTER_NEEDS_TYPE = 1 << 1,
	GNOME_VFS_DIRECTORY_FILTER_NEEDS_STAT = 1 << 2,
	GNOME_VFS_DIRECTORY_FILTER_NEEDS_MIMETYPE = 1 << 3,
} GnomeVFSDirectoryFilterNeeds;

typedef enum {
	GNOME_VFS_DIRECTORY_VISIT_DEFAULT = 0,
	GNOME_VFS_DIRECTORY_VISIT_SAMEFS = 1 << 0,
	GNOME_VFS_DIRECTORY_VISIT_LOOPCHECK = 1 << 1
} GnomeVFSDirectoryVisitOptions;

typedef struct GnomeVFSDirectoryFilter GnomeVFSDirectoryFilter;

typedef gboolean (* GnomeVFSDirectoryFilterFunc) (const GnomeVFSFileInfo *info,
						  gpointer data);
typedef gint     (* GnomeVFSDirectorySortFunc)   (const GnomeVFSFileInfo *a,
						  const GnomeVFSFileInfo *b,
						  gpointer data);
typedef gboolean (* GnomeVFSDirectoryVisitFunc)	 (const gchar *rel_path,
						  GnomeVFSFileInfo *info,
						  gboolean recursing_will_loop,
						  gpointer data,
						  gboolean *recurse);

/* Xfer options.  
 * FIXME bugzilla.eazel.com 1205:
 * Split these up into xfer options and xfer actions
 */
typedef enum {
	GNOME_VFS_XFER_DEFAULT = 0,
	GNOME_VFS_XFER_UNUSED_1 = 1 << 0,
	GNOME_VFS_XFER_FOLLOW_LINKS = 1 << 1,
	GNOME_VFS_XFER_UNUSED_2 = 1 << 2,
	GNOME_VFS_XFER_RECURSIVE = 1 << 3,
	GNOME_VFS_XFER_SAMEFS = 1 << 4,
	GNOME_VFS_XFER_DELETE_ITEMS = 1 << 5,
	GNOME_VFS_XFER_EMPTY_DIRECTORIES = 1 << 6,
	GNOME_VFS_XFER_NEW_UNIQUE_DIRECTORY = 1 << 7,
	GNOME_VFS_XFER_REMOVESOURCE = 1 << 8,
	GNOME_VFS_XFER_USE_UNIQUE_NAMES = 1 << 9,
	GNOME_VFS_XFER_LINK_ITEMS = 1 << 10
} GnomeVFSXferOptions;

/* Progress status, to be reported to the caller of the transfer operation.  */
typedef enum {
	GNOME_VFS_XFER_PROGRESS_STATUS_OK = 0,
	GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR = 1,
	GNOME_VFS_XFER_PROGRESS_STATUS_OVERWRITE = 2,
	/* during the duplicate status the progress callback is asked to
	   supply a new unique name */
	GNOME_VFS_XFER_PROGRESS_STATUS_DUPLICATE = 3
} GnomeVFSXferProgressStatus;

/* The different ways to deal with overwriting during a transfer operation.  */
typedef enum {
	/* Interrupt transferring with an error (GNOME_VFS_ERROR_FILEEXISTS).  */
	GNOME_VFS_XFER_OVERWRITE_MODE_ABORT = 0,
	/* Invoke the progress callback with a
	   `GNOME_VFS_XFER_PROGRESS_STATUS_OVERWRITE' status code. */
	GNOME_VFS_XFER_OVERWRITE_MODE_QUERY = 1,
	/* Overwrite files silently.  */
	GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE = 2,
	/* Ignore files silently.  */
	GNOME_VFS_XFER_OVERWRITE_MODE_SKIP = 3
} GnomeVFSXferOverwriteMode;

/* This defines the actions to perform before a file is being overwritten
   (i.e., these are the answers that can be given to a replace query).  */
typedef enum {
	GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT = 0,
	GNOME_VFS_XFER_OVERWRITE_ACTION_REPLACE = 1,
	GNOME_VFS_XFER_OVERWRITE_ACTION_REPLACE_ALL = 2,
	GNOME_VFS_XFER_OVERWRITE_ACTION_SKIP = 3,
	GNOME_VFS_XFER_OVERWRITE_ACTION_SKIP_ALL = 4,
} GnomeVFSXferOverwriteAction;

typedef enum {
	/* Interrupt transferring with an error (code returned is code of the
           operation that has caused the error).  */
	GNOME_VFS_XFER_ERROR_MODE_ABORT = 0,
	/* Invoke the progress callback with a
	   `GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR' status code. */
	GNOME_VFS_XFER_ERROR_MODE_QUERY = 1,
} GnomeVFSXferErrorMode;

/* This defines the possible actions to be performed after an error has
   occurred.  */
typedef enum {
	/* Interrupt operation and return `GNOME_VFS_ERROR_INTERRUPTED'.  */
	GNOME_VFS_XFER_ERROR_ACTION_ABORT = 0,
	/* Try the same operation again.  */
	GNOME_VFS_XFER_ERROR_ACTION_RETRY = 1,
	/* Skip this file and continue normally.  */
	GNOME_VFS_XFER_ERROR_ACTION_SKIP = 2
} GnomeVFSXferErrorAction;

/* This specifies the current phase in the transfer operation.  Phases whose
   comments are marked with `(*)' are always reported in "normal" (i.e. no
   error) condition; the other ones are only reported if an error happens in
   that specific phase.  */
typedef enum {
	/* Initial phase */
	GNOME_VFS_XFER_PHASE_INITIAL,
	/* Checking if destination can handle move/copy */
	GNOME_VFS_XFER_CHECKING_DESTINATION,
	/* Collecting file list */
	GNOME_VFS_XFER_PHASE_COLLECTING,
	/* File list collected (*) */
	GNOME_VFS_XFER_PHASE_READYTOGO,
	/* Opening source file for reading */
	GNOME_VFS_XFER_PHASE_OPENSOURCE,
	/* Creating target file for copy */
	GNOME_VFS_XFER_PHASE_OPENTARGET,
	/* Copying data from source to target (*) */
	GNOME_VFS_XFER_PHASE_COPYING,
	/* Moving file from source to target (*) */
	GNOME_VFS_XFER_PHASE_MOVING,
	/* Reading data from source file */
	GNOME_VFS_XFER_PHASE_READSOURCE,
	/* Writing data to target file */
	GNOME_VFS_XFER_PHASE_WRITETARGET,
	/* Closing source file */
	GNOME_VFS_XFER_PHASE_CLOSESOURCE,
	/* Closing target file */
	GNOME_VFS_XFER_PHASE_CLOSETARGET,
	/* Deleting source file */
	GNOME_VFS_XFER_PHASE_DELETESOURCE,
	/* Setting attributes on target file */
	GNOME_VFS_XFER_PHASE_SETATTRIBUTES,
	/* Go to the next file (*) */
	GNOME_VFS_XFER_PHASE_FILECOMPLETED,
	/* cleaning up after a move (removing source files, etc.) */
	GNOME_VFS_XFER_PHASE_CLEANUP,
	/* Operation finished (*) */
	GNOME_VFS_XFER_PHASE_COMPLETED,
	GNOME_VFS_XFER_NUM_PHASES
} GnomeVFSXferPhase;

/* Progress information for the transfer operation.  This is especially useful
   for interactive programs.  */
typedef struct {
	/* Progress status (see above for a description).  */
	GnomeVFSXferProgressStatus status;

	/* VFS status code.  If `status' is
           `GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR', you should look at this
           member to know what has happened.  */
	GnomeVFSResult vfs_status;

	/* Current phase in the transferring process.  */
	GnomeVFSXferPhase phase;

	/* Source URI. FIXME bugzilla.eazel.com 1206: change name? */
	gchar *source_name;

	/* Destination URI. FIXME bugzilla.eazel.com 1206: change name? */
	gchar *target_name;

	/* Index of file being copied. */
	gulong file_index;

	/* Total number of files to be copied.  */
	gulong files_total;

	/* Total number of bytes to be copied.  */
	GnomeVFSFileSize bytes_total;

	/* Total size of this file (in bytes).  */
	GnomeVFSFileSize file_size;

	/* Bytes copied for this file so far.  */
	GnomeVFSFileSize bytes_copied;

	/* Total amount of data copied from the beginning.  */
	GnomeVFSFileSize total_bytes_copied;
	
	/* Target unique name used when duplicating, etc. to avoid collisions */ 
	gchar *duplicate_name;

	/* Count used in the unique name e.g. (copy 2), etc. */
	int duplicate_count;

	gboolean top_level_item;
	/* indicates that the copied/moved/deleted item is an actual item
	 * passed in the uri list rather than one encountered by recursively
	 * traversing directories. Used by metadata copying.
	 */

} GnomeVFSXferProgressInfo;

/* This is the prototype for functions called during a transfer operation to
   report progress.  If the return value is `FALSE' (0), the operation is
   interrupted immediately: the transfer function returns with the value of
   `vfs_status' if it is different from `GNOME_VFS_OK', or with
   `GNOME_VFS_ERROR_INTERRUPTED' otherwise.  The effect of other values depend
   on the value of `info->status':

   - If the status is `GNOME_VFS_XFER_PROGRESS_STATUS_OK', the transfer
     operation is resumed normally.

   - If the status is `GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR', the return
     value is interpreted as a `GnomeVFSXferErrorAction' and operation is
     interrupted, continued or retried accordingly.

   - If the status is `GNOME_VFS_XFER_PROGRESS_STATUS_OVERWRITE', the return
     value is interpreted as a `GnomeVFSXferOverwriteAction'.  */

typedef gint (* GnomeVFSXferProgressCallback) 	(GnomeVFSXferProgressInfo *info,
						 gpointer data);

/* Types for asynchronous operations.  */

typedef struct GnomeVFSAsyncHandle GnomeVFSAsyncHandle;

typedef void	(* GnomeVFSAsyncCallback)	(GnomeVFSAsyncHandle *handle,
						 GnomeVFSResult result,
						 gpointer callback_data);

typedef GnomeVFSAsyncCallback GnomeVFSAsyncOpenCallback;
typedef GnomeVFSAsyncCallback GnomeVFSAsyncCreateCallback;

typedef void	(* GnomeVFSAsyncOpenAsChannelCallback)
						(GnomeVFSAsyncHandle *handle,
						 GIOChannel *channel,
						 GnomeVFSResult result,
						 gpointer callback_data);

typedef GnomeVFSAsyncOpenAsChannelCallback GnomeVFSAsyncCreateAsChannelCallback;

#define GnomeVFSAsyncCloseCallback	GnomeVFSAsyncCallback

typedef void	(* GnomeVFSAsyncReadCallback)	(GnomeVFSAsyncHandle *handle,
						 GnomeVFSResult result,
						 gpointer buffer,
						 GnomeVFSFileSize bytes_requested,
						 GnomeVFSFileSize bytes_read,
						 gpointer callback_data);

typedef void	(* GnomeVFSAsyncWriteCallback)	(GnomeVFSAsyncHandle *handle,
						 GnomeVFSResult result,
						 gconstpointer buffer,
						 GnomeVFSFileSize bytes_requested,
						 GnomeVFSFileSize bytes_written,
						 gpointer callback_data);

typedef struct {
	GnomeVFSURI *uri;
	GnomeVFSResult result;
	GnomeVFSFileInfo *file_info;
} GnomeVFSGetFileInfoResult;

typedef void    (* GnomeVFSAsyncGetFileInfoCallback)
                                                (GnomeVFSAsyncHandle *handle,
						 GList *results, /* GnomeVFSGetFileInfoResult* items */
						 gpointer callback_data);

typedef void	(* GnomeVFSAsyncSetFileInfoCallback)	
						(GnomeVFSAsyncHandle *handle,
						 GnomeVFSResult result,
						 GnomeVFSFileInfo *file_info,
						 gpointer callback_data);

typedef void	(* GnomeVFSAsyncDirectoryLoadCallback)
						(GnomeVFSAsyncHandle *handle,
						 GnomeVFSResult result,
						 GnomeVFSDirectoryList *list,
						 guint entries_read,
						 gpointer callback_data);

typedef gint    (* GnomeVFSAsyncXferProgressCallback)
						(GnomeVFSAsyncHandle *handle,
						 GnomeVFSXferProgressInfo *info,
						 gpointer data);

typedef struct {
	GnomeVFSURI *uri;
	GnomeVFSResult result;
} GnomeVFSFindDirectoryResult;

typedef void    (* GnomeVFSAsyncFindDirectoryCallback)
						(GnomeVFSAsyncHandle *handle,
						 GList *results /* GnomeVFSFindDirectoryResult */,
						 gpointer data);


/* Used to report user-friendly status messages you might want to display. */
typedef void    (* GnomeVFSStatusCallback)      (const gchar *message,
						 gpointer     callback_data);

#endif /* _GNOME_VFS_TYPES_H */
