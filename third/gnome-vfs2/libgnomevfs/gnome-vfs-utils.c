/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-utils.c - Private utility functions for the GNOME Virtual
   File System.

   Copyright (C) 1999 Free Software Foundation
   Copyright (C) 2000, 2001 Eazel, Inc.

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

   Authors: Ettore Perazzoli <ettore@comm2000.it>
   	    John Sullivan <sullivan@eazel.com> 
            Darin Adler <darin@eazel.com>
*/

#include <config.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include "gnome-vfs-utils.h"

#include "gnome-vfs-i18n.h"
#include "gnome-vfs-private-utils.h"
#include "gnome-vfs-ops.h"
#include <glib/gstrfuncs.h>
#include <glib/gutils.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif

#if HAVE_SYS_VFS_H
#include <sys/vfs.h>
#elif HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif

#define KILOBYTE_FACTOR 1024.0
#define MEGABYTE_FACTOR (1024.0 * 1024.0)
#define GIGABYTE_FACTOR (1024.0 * 1024.0 * 1024.0)


#define READ_CHUNK_SIZE 8192

#define MAX_SYMLINKS_FOLLOWED 32


/**
 * gnome_vfs_format_file_size_for_display:
 * @size:
 * 
 * Formats the file size passed in @bytes in a way that is easy for
 * the user to read. Gives the size in bytes, kilobytes, megabytes or
 * gigabytes, choosing whatever is appropriate.
 * 
 * Returns: a newly allocated string with the size ready to be shown.
 **/

gchar*
gnome_vfs_format_file_size_for_display (GnomeVFSFileSize size)
{
	if (size < (GnomeVFSFileSize) KILOBYTE_FACTOR) {
		if (size == 1)
			return g_strdup (_("1 byte"));
		else
			return g_strdup_printf (_("%u bytes"),
						       (guint) size);
	} else {
		gdouble displayed_size;

		if (size < (GnomeVFSFileSize) MEGABYTE_FACTOR) {
			displayed_size = (gdouble) size / KILOBYTE_FACTOR;
			return g_strdup_printf (_("%.1f K"),
						       displayed_size);
		} else if (size < (GnomeVFSFileSize) GIGABYTE_FACTOR) {
			displayed_size = (gdouble) size / MEGABYTE_FACTOR;
			return g_strdup_printf (_("%.1f MB"),
						       displayed_size);
		} else {
			displayed_size = (gdouble) size / GIGABYTE_FACTOR;
			return g_strdup_printf (_("%.1f GB"),
						       displayed_size);
		}
	}
}

typedef enum {
	UNSAFE_ALL        = 0x1,  /* Escape all unsafe characters   */
	UNSAFE_ALLOW_PLUS = 0x2,  /* Allows '+'  */
	UNSAFE_PATH       = 0x4,  /* Allows '/' and '?' and '&' and '='  */
	UNSAFE_DOS_PATH   = 0x8,  /* Allows '/' and '?' and '&' and '=' and ':' */
	UNSAFE_HOST       = 0x10, /* Allows '/' and ':' and '@' */
	UNSAFE_SLASHES    = 0x20  /* Allows all characters except for '/' and '%' */
} UnsafeCharacterSet;

static const guchar acceptable[96] =
{ /* X0   X1   X2   X3   X4   X5   X6   X7   X8   X9   XA   XB   XC   XD   XE   XF */
    0x00,0x3F,0x20,0x20,0x20,0x00,0x2C,0x3F,0x3F,0x3F,0x3F,0x22,0x20,0x3F,0x3F,0x1C, /* 2X  !"#$%&'()*+,-./   */
    0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x38,0x20,0x20,0x2C,0x20,0x2C, /* 3X 0123456789:;<=>?   */
    0x30,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F, /* 4X @ABCDEFGHIJKLMNO   */
    0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x20,0x20,0x20,0x20,0x3F, /* 5X PQRSTUVWXYZ[\]^_   */
    0x20,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F, /* 6X `abcdefghijklmno   */
    0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x20,0x20,0x20,0x3F,0x20  /* 7X pqrstuvwxyz{|}~DEL */
};

enum {
	RESERVED = 1,
	UNRESERVED,
	DELIMITERS,
	UNWISE,
	CONTROL,
	SPACE	
};

static const guchar uri_character_kind[128] =
{
    CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   , 
    CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,
    CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,
    CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,CONTROL   ,
    /* ' '        !          "          #          $          %          &          '      */
    SPACE     ,UNRESERVED,DELIMITERS,DELIMITERS,RESERVED  ,DELIMITERS,RESERVED  ,UNRESERVED,
    /*  (         )          *          +          ,          -          .          /      */
    UNRESERVED,UNRESERVED,UNRESERVED,RESERVED  ,RESERVED  ,UNRESERVED,UNRESERVED,RESERVED  , 
    /*  0         1          2          3          4          5          6          7      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  8         9          :          ;          <          =          >          ?      */
    UNRESERVED,UNRESERVED,RESERVED  ,RESERVED  ,DELIMITERS,RESERVED  ,DELIMITERS,RESERVED  ,
    /*  @         A          B          C          D          E          F          G      */
    RESERVED  ,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  H         I          J          K          L          M          N          O      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  P         Q          R          S          T          U          V          W      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  X         Y          Z          [          \          ]          ^          _      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNWISE    ,UNWISE    ,UNWISE    ,UNWISE    ,UNRESERVED,
    /*  `         a          b          c          d          e          f          g      */
    UNWISE    ,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  h         i          j          k          l          m          n          o      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  p         q          r          s          t          u          v          w      */
    UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,UNRESERVED,
    /*  x         y          z         {           |          }          ~         DEL     */
    UNRESERVED,UNRESERVED,UNRESERVED,UNWISE    ,UNWISE    ,UNWISE    ,UNRESERVED,CONTROL
};


/*  Below modified from libwww HTEscape.c */

#define HEX_ESCAPE '%'

/*  Escape undesirable characters using %
 *  -------------------------------------
 *
 * This function takes a pointer to a string in which
 * some characters may be unacceptable unescaped.
 * It returns a string which has these characters
 * represented by a '%' character followed by two hex digits.
 *
 * This routine returns a g_malloced string.
 */

static const gchar hex[16] = "0123456789ABCDEF";

static gchar *
gnome_vfs_escape_string_internal (const gchar *string, 
				  UnsafeCharacterSet mask)
{
#define ACCEPTABLE_CHAR(a) ((a)>=32 && (a)<128 && (acceptable[(a)-32] & use_mask))

	const gchar *p;
	gchar *q;
	gchar *result;
	guchar c;
	gint unacceptable;
	UnsafeCharacterSet use_mask;

	g_return_val_if_fail (mask == UNSAFE_ALL
			      || mask == UNSAFE_ALLOW_PLUS
			      || mask == UNSAFE_PATH
			      || mask == UNSAFE_DOS_PATH
			      || mask == UNSAFE_HOST
			      || mask == UNSAFE_SLASHES, NULL);

	if (string == NULL) {
		return NULL;
	}
	
	unacceptable = 0;
	use_mask = mask;
	for (p = string; *p != '\0'; p++) {
		c = *p;
		if (!ACCEPTABLE_CHAR (c)) {
			unacceptable++;
		}
		if ((use_mask == UNSAFE_HOST) && 
		    (unacceptable || (c == '/'))) {
			/* when escaping a host, if we hit something that needs to be escaped, or we finally
			 * hit a path separator, revert to path mode (the host segment of the url is over).
			 */
			use_mask = UNSAFE_PATH;
		}
	}
	
	result = g_malloc (p - string + unacceptable * 2 + 1);

	use_mask = mask;
	for (q = result, p = string; *p != '\0'; p++){
		c = *p;
		
		if (!ACCEPTABLE_CHAR (c)) {
			*q++ = HEX_ESCAPE; /* means hex coming */
			*q++ = hex[c >> 4];
			*q++ = hex[c & 15];
		} else {
			*q++ = c;
		}
		if ((use_mask == UNSAFE_HOST) &&
		    (!ACCEPTABLE_CHAR (c) || (c == '/'))) {
			use_mask = UNSAFE_PATH;
		}
	}
	
	*q = '\0';
	
	return result;
}

/**
 * gnome_vfs_escape_string:
 * @string: string to be escaped
 *
 * Escapes @string, replacing any and all special characters 
 * with equivalent escape sequences.
 *
 * Return value: a newly allocated string equivalent to @string
 * but with all special characters escaped
 **/
gchar *
gnome_vfs_escape_string (const gchar *string)
{
	return gnome_vfs_escape_string_internal (string, UNSAFE_ALL);
}

/**
 * gnome_vfs_escape_path_string:
 * @path: string to be escaped
 *
 * Escapes @path, replacing only special characters that would not
 * be found in paths (so '/', '&', '=', and '?' will not be escaped by
 * this function).
 *
 * Return value: a newly allocated string equivalent to @path but
 * with non-path characters escaped
 **/
gchar *
gnome_vfs_escape_path_string (const gchar *path)
{
	return gnome_vfs_escape_string_internal (path, UNSAFE_PATH);
}

/**
 * gnome_vfs_escape_host_and_path_string:
 * @path: string to be escaped
 *
 * Escapes @path, replacing only special characters that would not
 * be found in paths or host name (so '/', '&', '=', ':', '@' 
 * and '?' will not be escaped by this function).
 *
 * Return value: a newly allocated string equivalent to @path but
 * with non-path/host characters escaped
 **/
gchar *
gnome_vfs_escape_host_and_path_string (const gchar *path)
{
	return gnome_vfs_escape_string_internal (path, UNSAFE_HOST);
}

/**
 * gnome_vfs_escape_slashes:
 * @string: string to be escaped
 *
 * Escapes only '/' and '%' characters in @string, replacing
 * them with their escape sequence equivalents.
 *
 * Return value: a newly allocated string equivalent to @string,
 * but with no unescaped '/' or '%' characters
 **/
gchar *
gnome_vfs_escape_slashes (const gchar *string)
{
	return gnome_vfs_escape_string_internal (string, UNSAFE_SLASHES);
}

char *
gnome_vfs_escape_set (const char *string,
	              const char *match_set)
{
	char *result;
	const char *scanner;
	char *result_scanner;
	int escape_count;

	escape_count = 0;

	if (string == NULL) {
		return NULL;
	}

	if (match_set == NULL) {
		return g_strdup (string);
	}
	
	for (scanner = string; *scanner != '\0'; scanner++) {
		if (strchr(match_set, *scanner) != NULL) {
			/* this character is in the set of characters 
			 * we want escaped.
			 */
			escape_count++;
		}
	}
	
	if (escape_count == 0) {
		return g_strdup (string);
	}

	/* allocate two extra characters for every character that
	 * needs escaping and space for a trailing zero
	 */
	result = g_malloc (scanner - string + escape_count * 2 + 1);
	for (scanner = string, result_scanner = result; *scanner != '\0'; scanner++) {
		if (strchr(match_set, *scanner) != NULL) {
			/* this character is in the set of characters 
			 * we want escaped.
			 */
			*result_scanner++ = HEX_ESCAPE;
			*result_scanner++ = hex[*scanner >> 4];
			*result_scanner++ = hex[*scanner & 15];
			
		} else {
			*result_scanner++ = *scanner;
		}
	}

	*result_scanner = '\0';

	return result;
}

/**
 * gnome_vfs_expand_initial_tilde:
 * @path: a local file path which may start with a '~'
 *
 * If @path starts with a ~, representing the user's home
 * directory, expand it to the actual path location.
 *
 * Return value: a newly allocated string with the initial
 * tilde (if there was one) converted to an actual path
 **/
char *
gnome_vfs_expand_initial_tilde (const char *path)
{
	char *slash_after_user_name, *user_name;
	struct passwd *passwd_file_entry;

	g_return_val_if_fail (path != NULL, NULL);

	if (path[0] != '~') {
		return g_strdup (path);
	}
	
	if (path[1] == '/' || path[1] == '\0') {
		return g_strconcat (g_get_home_dir (), &path[1], NULL);
	}

	slash_after_user_name = strchr (&path[1], '/');
	if (slash_after_user_name == NULL) {
		user_name = g_strdup (&path[1]);
	} else {
		user_name = g_strndup (&path[1],
				       slash_after_user_name - &path[1]);
	}
	passwd_file_entry = getpwnam (user_name);
	g_free (user_name);

	if (passwd_file_entry == NULL || passwd_file_entry->pw_dir == NULL) {
		return g_strdup (path);
	}

	return g_strconcat (passwd_file_entry->pw_dir,
			    slash_after_user_name,
			    NULL);
}

static int
hex_to_int (gchar c)
{
	return  c >= '0' && c <= '9' ? c - '0'
		: c >= 'A' && c <= 'F' ? c - 'A' + 10
		: c >= 'a' && c <= 'f' ? c - 'a' + 10
		: -1;
}

static int
unescape_character (const char *scanner)
{
	int first_digit;
	int second_digit;

	first_digit = hex_to_int (*scanner++);
	if (first_digit < 0) {
		return -1;
	}

	second_digit = hex_to_int (*scanner++);
	if (second_digit < 0) {
		return -1;
	}

	return (first_digit << 4) | second_digit;
}

/**
 * gnome_vfs_unescape_string:
 * @escaped_string: an escaped URI, path, or other string
 * @illegal_characters: a string containing a sequence of characters
 * considered "illegal", '\0' is automatically in this list.
 *
 * Decodes escaped characters (i.e. PERCENTxx sequences) in @escaped_string.
 * Characters are encoded in PERCENTxy form, where xy is the ASCII hex code 
 * for character 16x+y.
 * 
 * Return value: a newly allocated string with the unescaped equivalents, 
 * or %NULL if @escaped_string contained one of the characters 
 * in @illegal_characters.
 **/
char *
gnome_vfs_unescape_string (const gchar *escaped_string, 
			   const gchar *illegal_characters)
{
	const gchar *in;
	gchar *out, *result;
	gint character;
	
	if (escaped_string == NULL) {
		return NULL;
	}

	result = g_malloc (strlen (escaped_string) + 1);
	
	out = result;
	for (in = escaped_string; *in != '\0'; in++) {
		character = *in;
		if (*in == HEX_ESCAPE) {
			character = unescape_character (in + 1);

			/* Check for an illegal character. We consider '\0' illegal here. */
			if (character <= 0
			    || (illegal_characters != NULL
				&& strchr (illegal_characters, (char)character) != NULL)) {
				g_free (result);
				return NULL;
			}
			in += 2;
		}
		*out++ = (char)character;
	}
	
	*out = '\0';
	g_assert (out - result <= strlen (escaped_string));
	return result;
	
}

/**
 * gnome_vfs_unescape_for_display:
 * @escaped: The string encoded with escaped sequences
 * 
 * Similar to gnome_vfs_unescape_string, but it returns something
 * semi-intelligable to a user even upon receiving traumatic input
 * such as %00 or URIs in bad form.
 * 
 * See also: gnome_vfs_unescape_string.
 * 
 * Return value: A pointer to a g_malloc'd string with all characters
 *               replacing their escaped hex values
 *
 * WARNING: You should never use this function on a whole URI!  It
 * unescapes reserved characters, and can result in a mangled URI
 * that can not be re-entered.  For example, it unescapes "#" "&" and "?",
 * which have special meanings in URI strings.
 **/
gchar *
gnome_vfs_unescape_string_for_display (const gchar *escaped)
{
	const gchar *in, *start_escape;
	gchar *out, *result;
	gint i,j;
	gchar c;
	gint invalid_escape;

	if (escaped == NULL) {
		return NULL;
	}

	result = g_malloc (strlen (escaped) + 1);
	
	out = result;
	for (in = escaped; *in != '\0'; ) {
		start_escape = in;
		c = *in++;
		invalid_escape = 0;
		
		if (c == HEX_ESCAPE) {
			/* Get the first hex digit. */
			i = hex_to_int (*in++);
			if (i < 0) {
				invalid_escape = 1;
				in--;
			}
			c = i << 4;
			
			if (invalid_escape == 0) {
				/* Get the second hex digit. */
				i = hex_to_int (*in++);
				if (i < 0) {
					invalid_escape = 2;
					in--;
				}
				c |= i;
			}
			if (invalid_escape == 0) {
				/* Check for an illegal character. */
				if (c == '\0') {
					invalid_escape = 3;
				}
			}
		}
		if (invalid_escape != 0) {
			for (j = 0; j < invalid_escape; j++) {
				*out++ = *start_escape++;
			}
		} else {
			*out++ = c;
		}
	}
	
	*out = '\0';
	g_assert (out - result <= strlen (escaped));
	return result;
}

/**
 * gnome_vfs_remove_optional_escapes:
 * @uri: an escaped uri
 * 
 * Scans the uri and converts characters that do not have to be 
 * escaped into an un-escaped form. The characters that get treated this
 * way are defined as unreserved by the RFC.
 * 
 * Return value: an error value if the uri is found to be malformed.
 **/
GnomeVFSResult
gnome_vfs_remove_optional_escapes (char *uri)
{
	guchar *scanner;
	int character;
	int length;

	if (uri == NULL) {
		return GNOME_VFS_OK;
	}
	
	length = strlen (uri);

	for (scanner = uri; *scanner != '\0'; scanner++, length--) {
		if (*scanner == HEX_ESCAPE) {
			character = unescape_character (scanner + 1);
			if (character < 0) {
				/* invalid hexadecimal character */
				return GNOME_VFS_ERROR_INVALID_URI;
			}

			if (uri_character_kind [character] == UNRESERVED) {
				/* This character does not need to be escaped, convert it
				 * to a non-escaped form.
				 */
				*scanner = (guchar)character;
				g_assert (length >= 3);

				/* Shrink the string covering up the two extra digits of the
				 * escaped character. Include the trailing '\0' in the copy
				 * to keep the string terminated.
				 */
				memmove (scanner + 1, scanner + 3, length - 2);
			} else {
				/* This character must stay escaped, skip the entire
				 * escaped sequence
				 */
				scanner += 2;
			}
			length -= 2;

		} else if (*scanner > 127
			|| uri_character_kind [*scanner] == DELIMITERS
			|| uri_character_kind [*scanner] == UNWISE
			|| uri_character_kind [*scanner] == CONTROL) {
			/* It is illegal for this character to be in an un-escaped form
			 * in the uri.
			 */
			return GNOME_VFS_ERROR_INVALID_URI;
		}
	}
	return GNOME_VFS_OK;
}

static char *
gnome_vfs_make_uri_canonical_old (const char *original_uri_text)
{
	GnomeVFSURI *uri;
	char *result;

	uri = gnome_vfs_uri_new_private (original_uri_text, TRUE, TRUE, FALSE);
	if (uri == NULL) {
		return NULL;;
	} 

	result = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	gnome_vfs_uri_unref (uri);

	return result;
}

/**
 * gnome_vfs_make_path_name_canonical:
 * @path: a file path, relative or absolute
 * 
 * Calls _gnome_vfs_canonicalize_pathname, allocating storage for the 
 * result and providing for a cleaner memory management.
 * 
 * Return value: a canonical version of @path
 **/
gchar *
gnome_vfs_make_path_name_canonical (const gchar *path)
{
	char *path_clone;
	char *result;

	path_clone = g_strdup (path);
	result = _gnome_vfs_canonicalize_pathname (path_clone);
	if (result != path_clone) {
		g_free (path_clone);
		return g_strdup (result);
	}

	return path_clone;
}

/**
 * gnome_vfs_list_deep_free:
 * @list: list to be freed
 *
 * Free @list, and call g_free() on all data members.
 **/
void
gnome_vfs_list_deep_free (GList *list)
{
	GList *p;

	if (list == NULL)
		return;

	for (p = list; p != NULL; p = p->next) {
		g_free (p->data);
	}
	g_list_free (list);
}

/**
 * gnome_vfs_get_local_path_from_uri:
 * @uri: URI to convert to a local path
 * 
 * Create a local path for a file:/// URI. Do not use with URIs
 * of other methods.
 *
 * Return value: the local path 
 * NULL is returned on error or if the uri isn't a file: URI
 * without a fragment identifier (or chained URI).
 **/
char *
gnome_vfs_get_local_path_from_uri (const char *uri)
{
	const char *path_part;

	if (!_gnome_vfs_istr_has_prefix (uri, "file:/")) {
		return NULL;
	}
	
	path_part = uri + strlen ("file:");
	if (strchr (path_part, '#') != NULL) {
		return NULL;
	}
	
	if (_gnome_vfs_istr_has_prefix (path_part, "///")) {
		path_part += 2;
	} else if (_gnome_vfs_istr_has_prefix (path_part, "//")) {
		return NULL;
	}

	return gnome_vfs_unescape_string (path_part, "/");
}

/**
 * gnome_vfs_get_uri_from_local_path:
 * @local_full_path: a full local filesystem path (i.e. not relative)
 * 
 * Returns a file:/// URI for the local path @local_full_path.
 *
 * Return value: the URI corresponding to @local_full_path 
 * (NULL for some bad errors).
 **/
char *
gnome_vfs_get_uri_from_local_path (const char *local_full_path)
{
	char *escaped_path, *result;
	
	if (local_full_path == NULL) {
		return NULL;
	}

	g_return_val_if_fail (local_full_path[0] == '/', NULL);

	escaped_path = gnome_vfs_escape_path_string (local_full_path);
	result = g_strconcat ("file://", escaped_path, NULL);
	g_free (escaped_path);
	return result;
}

/**
 * gnome_vfs_get_volume_free_space:
 * @vfs_uri:
 * @size:
 * 
 * Stores in @size the amount of free space on a volume.
 * This only works for local file systems with the file: scheme.
 *
 * Returns: GNOME_VFS_OK on success, otherwise an error code
 */
GnomeVFSResult
gnome_vfs_get_volume_free_space (const GnomeVFSURI *vfs_uri, 
				 GnomeVFSFileSize  *size)
{	
	GnomeVFSFileSize free_blocks, block_size;
       	int statfs_result;
	const char *path, *scheme;
	char *unescaped_path;
	GnomeVFSResult ret;
#if HAVE_STATVFS
	struct statvfs statfs_buffer;
#else
	struct statfs statfs_buffer;
#endif

 	*size = 0;

	/* We can't check non local systems */
	if (!gnome_vfs_uri_is_local (vfs_uri)) {
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	}

	path = gnome_vfs_uri_get_path (vfs_uri);

	unescaped_path = gnome_vfs_unescape_string (path, G_DIR_SEPARATOR_S);
	
	scheme = gnome_vfs_uri_get_scheme (vfs_uri);
	
        /* We only handle the file scheme for now */
	if (g_ascii_strcasecmp (scheme, "file") != 0 || !_gnome_vfs_istr_has_prefix (path, "/")) {
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	}

#if HAVE_STATVFS
	statfs_result = statvfs (unescaped_path, &statfs_buffer);
#else
	statfs_result = statfs (unescaped_path, &statfs_buffer);   
#endif  

	if (statfs_result == 0) {
		ret = GNOME_VFS_OK;
	} else {
		ret = gnome_vfs_result_from_errno ();
	}
	
	g_return_val_if_fail (statfs_result == 0, FALSE);
	block_size = statfs_buffer.f_bsize; 
	free_blocks = statfs_buffer.f_bavail;

	*size = block_size * free_blocks;

	g_free (unescaped_path);
	
	return ret;
}

/**
 * hack_file_exists
 * @filename: pathname to test for existance.
 *
 * Returns true if filename exists
 */
/* FIXME: Why is this here? Why not use g_file_exists in libgnome/gnome-util.h?
 * (I tried to simply replace but there were strange include dependencies, maybe
 * that's why this function exists.)
 */
static int
hack_file_exists (const char *filename)
{
	struct stat s;

	g_return_val_if_fail (filename != NULL,FALSE);
    
	return stat (filename, &s) == 0;
}

char *
gnome_vfs_icon_path_from_filename (const char *relative_filename)
{
	const char *gnome_var;
	char *full_filename;
	char **paths, **temp_paths;

	if (g_path_is_absolute (relative_filename) &&
	    hack_file_exists (relative_filename))
		return g_strdup (relative_filename);

	gnome_var = g_getenv ("GNOME_PATH");

	if (gnome_var == NULL) {
		gnome_var = PREFIX;
	}

	paths = g_strsplit (gnome_var, ":", 0); 

	for (temp_paths = paths; *temp_paths != NULL; temp_paths++) {
		full_filename = g_strconcat (*temp_paths, "/share/pixmaps/", relative_filename, NULL);
		if (hack_file_exists (full_filename)) {
			g_strfreev (paths);
			return full_filename;
		}
		g_free (full_filename);
		full_filename = NULL;
	}

	g_strfreev (paths);
	return NULL;
}

static char *
strdup_to (const char *string, const char *end)
{
	if (end == NULL) {
		return g_strdup (string);
	}
	return g_strndup (string, end - string);
}

static gboolean
is_executable_file (const char *path)
{
	struct stat stat_buffer;

	/* Check that it exists. */
	if (stat (path, &stat_buffer) != 0) {
		return FALSE;
	}

	/* Check that it is a file. */
	if (!S_ISREG (stat_buffer.st_mode)) {
		return FALSE;
	}

	/* Check that it's executable. */
	if (access (path, X_OK) != 0) {
		return FALSE;
	}

	return TRUE;
}

static gboolean
executable_in_path (const char *executable_name)
{
	const char *path_list, *piece_start, *piece_end;
	char *piece, *raw_path, *expanded_path;
	gboolean is_good;

	path_list = g_getenv ("PATH");

	for (piece_start = path_list; ; piece_start = piece_end + 1) {
		/* Find the next piece of PATH. */
		piece_end = strchr (piece_start, ':');
		piece = strdup_to (piece_start, piece_end);
		g_strstrip (piece);
		
		if (piece[0] == '\0') {
			is_good = FALSE;
		} else {
			/* Try out this path with the executable. */
			raw_path = g_strconcat (piece, "/", executable_name, NULL);
			expanded_path = gnome_vfs_expand_initial_tilde (raw_path);
			g_free (raw_path);
			
			is_good = is_executable_file (expanded_path);
			g_free (expanded_path);
		}
		
		g_free (piece);
		
		if (is_good) {
			return TRUE;
		}

		if (piece_end == NULL) {
			return FALSE;
		}
	}
}

static char *
get_executable_name_from_command_string (const char *command_string)
{
	/* FIXME bugzilla.eazel.com 2757: 
	 * We need to handle quoting here for the full-path case */
	return g_strstrip (strdup_to (command_string, strchr (command_string, ' ')));
}

/**
 * gnome_vfs_is_executable_command_string:
 * @command_string:
 * 
 * Checks if @command_string starts with the full path of an executable file
 * or an executable in $PATH.
 *
 * Returns: TRUE if command_string started with and executable file, 
 * FALSE otherwise.
 */
gboolean
gnome_vfs_is_executable_command_string (const char *command_string)
{
	char *executable_name;
	char *executable_path;
	gboolean found;

	/* Check whether command_string is a full path for an executable. */
	if (command_string[0] == '/') {

		/* FIXME bugzilla.eazel.com 2757:
		 * Because we don't handle quoting, we can check for full
		 * path including spaces, but no parameters, and full path
		 * with no spaces with or without parameters. But this will
		 * fail for quoted full path with spaces, and parameters.
		 */

		/* This works if command_string contains a space, but not
		 * if command_string has parameters.
		 */
		if (is_executable_file (command_string)) {
			return TRUE;
		}

		/* This works if full path has no spaces, with or without parameters */
		executable_path = get_executable_name_from_command_string (command_string);
		found = is_executable_file (executable_path);
		g_free (executable_path);

		return found;
	}
	
	executable_name = get_executable_name_from_command_string (command_string);
	found = executable_in_path (executable_name);
	g_free (executable_name);

	return found;
}

/**
 * gnome_vfs_read_entire_file:
 * @uri: URI of the file to read
 * @file_size: after reading the file, contains the size of the file read
 * @file_contents: contains the file_size bytes, the contents of the file at @uri.
 * 
 * Reads an entire file into memory for convenience. Beware accidentally
 * loading large files into memory with this function.
 *
 * Return value: An integer representing the result of the operation
 *
 * Since: 2.2
 */

GnomeVFSResult
gnome_vfs_read_entire_file (const char *uri,
			    int *file_size,
			    char **file_contents)
{
	GnomeVFSResult result;
	GnomeVFSHandle *handle;
	char *buffer;
	GnomeVFSFileSize total_bytes_read;
	GnomeVFSFileSize bytes_read;

	*file_size = 0;
	*file_contents = NULL;

	/* Open the file. */
	result = gnome_vfs_open (&handle, uri, GNOME_VFS_OPEN_READ);
	if (result != GNOME_VFS_OK) {
		return result;
	}

	/* Read the whole thing. */
	buffer = NULL;
	total_bytes_read = 0;
	do {
		buffer = g_realloc (buffer, total_bytes_read + READ_CHUNK_SIZE);
		result = gnome_vfs_read (handle,
					 buffer + total_bytes_read,
					 READ_CHUNK_SIZE,
					 &bytes_read);
		if (result != GNOME_VFS_OK && result != GNOME_VFS_ERROR_EOF) {
			g_free (buffer);
			gnome_vfs_close (handle);
			return result;
		}

		/* Check for overflow. */
		if (total_bytes_read + bytes_read < total_bytes_read) {
			g_free (buffer);
			gnome_vfs_close (handle);
			return GNOME_VFS_ERROR_TOO_BIG;
		}

		total_bytes_read += bytes_read;
	} while (result == GNOME_VFS_OK);

	/* Close the file. */
	result = gnome_vfs_close (handle);
	if (result != GNOME_VFS_OK) {
		g_free (buffer);
		return result;
	}

	/* Return the file. */
	*file_size = total_bytes_read;
	*file_contents = g_realloc (buffer, total_bytes_read);
	return GNOME_VFS_OK;
}

static char *
gnome_vfs_make_valid_utf8 (const char *name)
{
	GString *string;
	const char *remainder, *invalid;
	int remaining_bytes, valid_bytes;

	string = NULL;
	remainder = name;
	remaining_bytes = strlen (name);

	while (remaining_bytes != 0) {
		if (g_utf8_validate (remainder, remaining_bytes, &invalid)) {
			break;
		}
		valid_bytes = invalid - remainder;

		if (string == NULL) {
			string = g_string_sized_new (remaining_bytes);
		}
		g_string_append_len (string, remainder, valid_bytes);
		g_string_append_c (string, '?');

		remaining_bytes -= valid_bytes + 1;
		remainder = invalid + 1;
	}

	if (string == NULL) {
		return g_strdup (name);
	}

	g_string_append (string, remainder);
	g_string_append (string, _(" (invalid Unicode)"));
	g_assert (g_utf8_validate (string->str, -1, NULL));

	return g_string_free (string, FALSE);
}

static char *
gnome_vfs_format_uri_for_display_internal (const char *uri, gboolean filenames_are_locale_encoded)
{
	char *canonical_uri, *path, *utf8_path;

	g_return_val_if_fail (uri != NULL, g_strdup (""));

	canonical_uri = gnome_vfs_make_uri_canonical_old (uri);

	/* If there's no fragment and it's a local path. */
	path = gnome_vfs_get_local_path_from_uri (canonical_uri);
	
	if (path != NULL) {
		if (filenames_are_locale_encoded) {
			utf8_path = g_locale_to_utf8 (path, -1, NULL, NULL, NULL);
			if (utf8_path) {
				g_free (canonical_uri);
				g_free (path);
				return utf8_path;
			} 
		} else if (g_utf8_validate (path, -1, NULL)) {
			g_free (canonical_uri);
			return path;
		}
	}

	if (canonical_uri && !g_utf8_validate (canonical_uri, -1, NULL)) {
		utf8_path = gnome_vfs_make_valid_utf8 (canonical_uri);
		g_free (canonical_uri);
		canonical_uri = utf8_path;
	}

	g_free (path);
	return canonical_uri;
}


/**
 * gnome_vfs_format_uri_for_display:
 *
 * Filter, modify, unescape and change URIs to make them appropriate
 * to display to users. The conversion is done such that the roundtrip
 * to UTF-8 is reversible.
 * 
 * Rules:
 * 	file: URI's without fragments should appear as local paths
 * 	file: URI's with fragments should appear as file: URI's
 * 	All other URI's appear as expected
 *
 * @uri: a URI
 *
 * Returns: a newly allocated UTF-8 string
 *
 * Since: 2.2
 **/

char *
gnome_vfs_format_uri_for_display (const char *uri) 
{
	static gboolean broken_filenames;
	
	broken_filenames = g_getenv ("G_BROKEN_FILENAMES") != NULL;

	return gnome_vfs_format_uri_for_display_internal (uri, broken_filenames);
}

static gboolean
is_valid_scheme_character (char c)
{
	return g_ascii_isalnum (c) || c == '+' || c == '-' || c == '.';
}

static gboolean
has_valid_scheme (const char *uri)
{
	const char *p;

	p = uri;

	if (!is_valid_scheme_character (*p)) {
		return FALSE;
	}

	do {
		p++;
	} while (is_valid_scheme_character (*p));

	return *p == ':';
}

static char *
gnome_vfs_escape_high_chars (const guchar *string)
{
	char *result;
	const guchar *scanner;
	guchar *result_scanner;
	int escape_count;
	static const gchar hex[16] = "0123456789ABCDEF";

#define ACCEPTABLE(a) ((a)>=32 && (a)<128)
	
	escape_count = 0;

	if (string == NULL) {
		return NULL;
	}

	for (scanner = string; *scanner != '\0'; scanner++) {
		if (!ACCEPTABLE(*scanner)) {
			escape_count++;
		}
	}
	
	if (escape_count == 0) {
		return g_strdup (string);
	}

	/* allocate two extra characters for every character that
	 * needs escaping and space for a trailing zero
	 */
	result = g_malloc (scanner - string + escape_count * 2 + 1);
	for (scanner = string, result_scanner = result; *scanner != '\0'; scanner++) {
		if (!ACCEPTABLE(*scanner)) {
			*result_scanner++ = '%';
			*result_scanner++ = hex[*scanner >> 4];
			*result_scanner++ = hex[*scanner & 15];
			
		} else {
			*result_scanner++ = *scanner;
		}
	}

	*result_scanner = '\0';

	return result;
}

/* The strip_trailing_whitespace option is intended to make copy/paste of
 * URIs less error-prone when it is known that trailing whitespace isn't
 * part of the uri.
 */
static char *
gnome_vfs_make_uri_from_input_internal (const char *text,
				  gboolean filenames_are_locale_encoded,
				  gboolean strip_trailing_whitespace)
{
	char *stripped, *path, *uri, *locale_path, *filesystem_path, *escaped;

	g_return_val_if_fail (text != NULL, g_strdup (""));

	/* Strip off leading whitespaces (since they can't be part of a valid
	   uri).   Only strip off trailing whitespaces when requested since
	   they might be part of a valid uri.
	 */
	if (strip_trailing_whitespace) {
		stripped = g_strstrip (g_strdup (text));
	} else {
		stripped = g_strchug (g_strdup (text));
	}

	switch (stripped[0]) {
	case '\0':
		uri = g_strdup ("");
		break;
	case '/':
		if (filenames_are_locale_encoded) {
			GError *error = NULL;
			locale_path = g_locale_from_utf8 (stripped, -1, NULL, NULL, &error);
			if (locale_path != NULL) {
				uri = gnome_vfs_get_uri_from_local_path (locale_path);
				g_free (locale_path);
			} else {
				/* We couldn't convert to the locale. */
				/* FIXME: We should probably give a user-visible error here. */
				uri = g_strdup("");
			}
		} else {
			uri = gnome_vfs_get_uri_from_local_path (stripped);
		}
		break;
	case '~':
		if (filenames_are_locale_encoded) {
			filesystem_path = g_locale_from_utf8 (stripped, -1, NULL, NULL, NULL);
		} else {
			filesystem_path = g_strdup (stripped);
		}
                /* deliberately falling into default case on fail */
		if (filesystem_path != NULL) {
			path = gnome_vfs_expand_initial_tilde (filesystem_path);
			g_free (filesystem_path);
			if (*path == '/') {
				uri = gnome_vfs_get_uri_from_local_path (path);
				g_free (path);
				break;
			}
			g_free (path);
		}
                /* don't insert break here, read above comment */
	default:
		if (has_valid_scheme (stripped)) {
			uri = gnome_vfs_escape_high_chars (stripped);
		} else {
			escaped = gnome_vfs_escape_high_chars (stripped);
			uri = g_strconcat ("http://", escaped, NULL);
			g_free (escaped);
		}
	}

	g_free (stripped);

	return uri;
	
}

/**
 * gnome_vfs_make_uri_from_input:
 * @location: a possibly mangled "uri", in UTF8
 *
 * Takes a user input path/URI and makes a valid URI out of it.
 *
 * This function is the reverse of gnome_vfs_format_uri_for_display
 * but it also handles the fact that the user could have typed
 * arbitrary UTF8 in the entry showing the string.
 *
 * Returns: a newly allocated uri.
 *
 * Since: 2.2
 **/
char *
gnome_vfs_make_uri_from_input (const char *location)
{
	static gboolean broken_filenames;

	broken_filenames = g_getenv ("G_BROKEN_FILENAMES") != NULL;

	return gnome_vfs_make_uri_from_input_internal (location, broken_filenames, TRUE);
}

/**
 * gnome_vfs_make_uri_canonical_strip_fragment:
 * @uri:
 *
 * If the @uri passed contains a fragment (anything after a '#') strips if,
 * then makes the URI canonical.
 *
 * Returns: a newly allocated string containing a canonical URI.
 *
 * Since: 2.2
 **/

char *
gnome_vfs_make_uri_canonical_strip_fragment (const char *uri)
{
	const char *fragment;
	char *without_fragment, *canonical;

	fragment = strchr (uri, '#');
	if (fragment == NULL) {
		return gnome_vfs_make_uri_canonical (uri);
	}

	without_fragment = g_strndup (uri, fragment - uri);
	canonical = gnome_vfs_make_uri_canonical (without_fragment);
	g_free (without_fragment);
	return canonical;
}

static gboolean
uris_match (const char *uri_1, const char *uri_2, gboolean ignore_fragments)
{
	char *canonical_1, *canonical_2;
	gboolean result;

	if (ignore_fragments) {
		canonical_1 = gnome_vfs_make_uri_canonical_strip_fragment (uri_1);
		canonical_2 = gnome_vfs_make_uri_canonical_strip_fragment (uri_2);
	} else {
		canonical_1 = gnome_vfs_make_uri_canonical (uri_1);
		canonical_2 = gnome_vfs_make_uri_canonical (uri_2);
	}

	result = strcmp (canonical_1, canonical_2) ? FALSE : TRUE;

	g_free (canonical_1);
	g_free (canonical_2);
	
	return result;
}

/**
 * gnome_vfs_uris_match:
 * @uri_1: stringified URI to compare with @uri_2.
 * @uri_2: stringified URI to compare with @uri_1.
 * 
 * Compare two URIs.
 *
 * Return value: TRUE if they are the same, FALSE otherwise.
 *
 * Since: 2.2
 **/

gboolean
gnome_vfs_uris_match (const char *uri_1, const char *uri_2)
{
	return uris_match (uri_1, uri_2, FALSE);
}

static gboolean
gnome_vfs_str_has_prefix (const char *haystack, const char *needle)
{
        const char *h, *n;

        /* Eat one character at a time. */
        h = haystack == NULL ? "" : haystack;
        n = needle == NULL ? "" : needle;
        do {
                if (*n == '\0') {
                        return TRUE;
                }
                if (*h == '\0') {
                        return FALSE;
                }
        } while (*h++ == *n++);
        return FALSE;
}


static gboolean
gnome_vfs_uri_is_local_scheme (const char *uri)
{
	gboolean is_local_scheme;
	char *temp_scheme;
	int i;
	char *local_schemes[] = {"file:", "help:", "ghelp:", "gnome-help:",
				 "trash:", "man:", "info:", 
				 "hardware:", "search:", "pipe:",
				 "gnome-trash:", NULL};

	is_local_scheme = FALSE;
	for (temp_scheme = *local_schemes, i = 0; temp_scheme != NULL; i++, temp_scheme = local_schemes[i]) {
		is_local_scheme = _gnome_vfs_istr_has_prefix (uri, temp_scheme);
		if (is_local_scheme) {
			break;
		}
	}
	

	return is_local_scheme;
}

static char *
gnome_vfs_handle_trailing_slashes (const char *uri)
{
	char *temp, *uri_copy;
	gboolean previous_char_is_column, previous_chars_are_slashes_without_column;
	gboolean previous_chars_are_slashes_with_column;
	gboolean is_local_scheme;

	g_assert (uri != NULL);

	uri_copy = g_strdup (uri);
	if (strlen (uri_copy) <= 2) {
		return uri_copy;
	}

	is_local_scheme = gnome_vfs_uri_is_local_scheme (uri);

	previous_char_is_column = FALSE;
	previous_chars_are_slashes_without_column = FALSE;
	previous_chars_are_slashes_with_column = FALSE;

	/* remove multiple trailing slashes */
	for (temp = uri_copy; *temp != '\0'; temp++) {
		if (*temp == '/' && !previous_char_is_column) {
			previous_chars_are_slashes_without_column = TRUE;
		} else if (*temp == '/' && previous_char_is_column) {
			previous_chars_are_slashes_without_column = FALSE;
			previous_char_is_column = TRUE;
			previous_chars_are_slashes_with_column = TRUE;
		} else {
			previous_chars_are_slashes_without_column = FALSE;
			previous_char_is_column = FALSE;
			previous_chars_are_slashes_with_column = FALSE;
		}

		if (*temp == ':') {
			previous_char_is_column = TRUE;
		}
	}

	if (*temp == '\0' && previous_chars_are_slashes_without_column) {
		if (is_local_scheme) {
			/* go back till you remove them all. */
			for (temp--; *(temp) == '/'; temp--) {
				*temp = '\0';
			}
		} else {
			/* go back till you remove them all but one. */
			for (temp--; *(temp - 1) == '/'; temp--) {
				*temp = '\0';
			}			
		}
	}

	if (*temp == '\0' && previous_chars_are_slashes_with_column) {
		/* go back till you remove them all but three. */
		for (temp--; *(temp - 3) != ':' && *(temp - 2) != ':' && *(temp - 1) != ':'; temp--) {
			*temp = '\0';
		}
	}


	return uri_copy;
}

/**
 * gnome_vfs_make_uri_canonical:
 * @uri: and absolute or relative URI, it might have scheme.
 *
 * Standarizes the format of the uri being passed, so that it can be used
 * later in other functions that expect a canonical URI.
 *
 * Returns: a newly allocated string that contains the canonical 
 * representation of @uri.
 *
 * Since: 2.2
 **/

char *
gnome_vfs_make_uri_canonical (const char *uri)
{
	char *canonical_uri, *old_uri, *p;
	gboolean relative_uri;

	relative_uri = FALSE;

	if (uri == NULL) {
		return NULL;
	}

	/* FIXME bugzilla.eazel.com 648: 
	 * This currently ignores the issue of two uris that are not identical but point
	 * to the same data except for the specific cases of trailing '/' characters,
	 * file:/ and file:///, and "lack of file:".
	 */

	canonical_uri = gnome_vfs_handle_trailing_slashes (uri);

	/* Note: In some cases, a trailing slash means nothing, and can
	 * be considered equivalent to no trailing slash. But this is
	 * not true in every case; specifically not for web addresses passed
	 * to a web-browser. So we don't have the trailing-slash-equivalence
	 * logic here, but we do use that logic in EelDirectory where
	 * the rules are more strict.
	 */

	/* Add file: if there is no scheme. */
	if (strchr (canonical_uri, ':') == NULL) {
		old_uri = canonical_uri;

		if (old_uri[0] != '/') {
			/* FIXME bugzilla.eazel.com 5069: 
			 *  bandaid alert. Is this really the right thing to do?
			 * 
			 * We got what really is a relative path. We do a little bit of
			 * a stretch here and assume it was meant to be a cryptic absolute path,
			 * and convert it to one. Since we can't call gnome_vfs_uri_new and
			 * gnome_vfs_uri_to_string to do the right make-canonical conversion,
			 * we have to do it ourselves.
			 */
			relative_uri = TRUE;
			canonical_uri = gnome_vfs_make_path_name_canonical (old_uri);
			g_free (old_uri);
			old_uri = canonical_uri;
			canonical_uri = g_strconcat ("file:///", old_uri, NULL);
		} else {
			canonical_uri = g_strconcat ("file:", old_uri, NULL);
		}
		g_free (old_uri);
	}

	/* Lower-case the scheme. */
	for (p = canonical_uri; *p != ':'; p++) {
		g_assert (*p != '\0');
		*p = g_ascii_tolower (*p);
	}

	if (!relative_uri) {
		old_uri = canonical_uri;
		canonical_uri = gnome_vfs_make_uri_canonical_old (canonical_uri);
		if (canonical_uri != NULL) {
			g_free (old_uri);
		} else {
			canonical_uri = old_uri;
		}
	}
	
	/* FIXME bugzilla.eazel.com 2802:
	 * Work around gnome-vfs's desire to convert file:foo into file://foo
	 * by converting to file:///foo here. When you remove this, check that
	 * typing "foo" into location bar does not crash and returns an error
	 * rather than displaying the contents of /
	 */
	if (gnome_vfs_str_has_prefix (canonical_uri, "file://")
	    && !gnome_vfs_str_has_prefix (canonical_uri, "file:///")) {
		old_uri = canonical_uri;
		canonical_uri = g_strconcat ("file:/", old_uri + 5, NULL);
		g_free (old_uri);
	}

	return canonical_uri;
}

/**
 * gnome_vfs_get_uri_scheme:
 * @uri: a stringified URI
 *
 * Retrieve the scheme used in @uri 
 *
 * Return value: A string containing the scheme
 *
 * Since: 2.2
 **/

char *
gnome_vfs_get_uri_scheme (const char *uri)
{
	char *colon;

	g_return_val_if_fail (uri != NULL, NULL);

	colon = strchr (uri, ':');
	
	if (colon == NULL) {
		return NULL;
	}
	
	return g_strndup (uri, colon - uri);
}

/* Note that NULL's and full paths are also handled by this function.
 * A NULL location will return the current working directory
 */
static char *
file_uri_from_local_relative_path (const char *location)
{
	char *current_dir;
	char *base_uri, *base_uri_slash;
	char *location_escaped;
	char *uri;

	current_dir = g_get_current_dir ();
	base_uri = gnome_vfs_get_uri_from_local_path (current_dir);
	/* g_get_current_dir returns w/o trailing / */
	base_uri_slash = g_strconcat (base_uri, "/", NULL);

	location_escaped = gnome_vfs_escape_path_string (location);

	uri = gnome_vfs_make_uri_full_from_relative (base_uri_slash, location_escaped);

	g_free (location_escaped);
	g_free (base_uri_slash);
	g_free (base_uri);
	g_free (current_dir);

	return uri;
}

/**
 * gnome_vfs_make_uri_from_shell_arg:
 * @location: a possibly mangled "uri"
 *
 * Similar to gnome_vfs_make_uri_from_input, except that:
 * 
 * 1) guesses relative paths instead of http domains
 * 2) doesn't bother stripping leading/trailing white space
 * 3) doesn't bother with ~ expansion--that's done by the shell
 *
 * Returns: a newly allocated uri
 *
 * Since: 2.2
 **/

char *
gnome_vfs_make_uri_from_shell_arg (const char *location)
{
	char *uri;

	g_return_val_if_fail (location != NULL, g_strdup (""));

	switch (location[0]) {
	case '\0':
		uri = g_strdup ("");
		break;
	case '/':
		uri = gnome_vfs_get_uri_from_local_path (location);
		break;
	default:
		if (has_valid_scheme (location)) {
			uri = g_strdup (location);
		} else {
			uri = file_uri_from_local_relative_path (location);
		}
	}

	return uri;
}

static gboolean
is_uri_partial (const char *uri)
{
	const char *current;

	/* RFC 2396 section 3.1 */
	for (current = uri ; 
		*current
		&& 	((*current >= 'a' && *current <= 'z')
			 || (*current >= 'A' && *current <= 'Z')
			 || (*current >= '0' && *current <= '9')
			 || ('-' == *current)
			 || ('+' == *current)
			 || ('.' == *current)) ;
	     current++);

	return  !(':' == *current);
}

/*
 * FIXME this is not the simplest or most time-efficent way
 * to do this.  Probably a far more clear way of doing this processing
 * is to split the path into segments, rather than doing the processing
 * in place.
 */
static void
remove_internal_relative_components (char *uri_current)
{
	char *segment_prev, *segment_cur;
	size_t len_prev, len_cur;

	len_prev = len_cur = 0;
	segment_prev = NULL;

	g_return_if_fail (uri_current != NULL);

	segment_cur = uri_current;

	while (*segment_cur) {
		len_cur = strcspn (segment_cur, "/");

		if (len_cur == 1 && segment_cur[0] == '.') {
			/* Remove "." 's */
			if (segment_cur[1] == '\0') {
				segment_cur[0] = '\0';
				break;
			} else {
				memmove (segment_cur, segment_cur + 2, strlen (segment_cur + 2) + 1);
				continue;
			}
		} else if (len_cur == 2 && segment_cur[0] == '.' && segment_cur[1] == '.' ) {
			/* Remove ".."'s (and the component to the left of it) that aren't at the
			 * beginning or to the right of other ..'s
			 */
			if (segment_prev) {
				if (! (len_prev == 2
				       && segment_prev[0] == '.'
				       && segment_prev[1] == '.')) {
				       	if (segment_cur[2] == '\0') {
						segment_prev[0] = '\0';
						break;
				       	} else {
						memmove (segment_prev, segment_cur + 3, strlen (segment_cur + 3) + 1);

						segment_cur = segment_prev;
						len_cur = len_prev;

						/* now we find the previous segment_prev */
						if (segment_prev == uri_current) {
							segment_prev = NULL;
						} else if (segment_prev - uri_current >= 2) {
							segment_prev -= 2;
							for ( ; segment_prev > uri_current && segment_prev[0] != '/' 
							      ; segment_prev-- );
							if (segment_prev[0] == '/') {
								segment_prev++;
							}
						}
						continue;
					}
				}
			}
		}

		/*Forward to next segment */

		if (segment_cur [len_cur] == '\0') {
			break;
		}
		 
		segment_prev = segment_cur;
		len_prev = len_cur;
		segment_cur += len_cur + 1;	
	}	
}

/**
 * gnome_vfs_make_uri_full_from_relative:
 * 
 * Returns a full URI given a full base URI, and a secondary URI which may
 * be relative.
 *
 * Return value: the URI (NULL for some bad errors).
 *
 * Since: 2.2
 **/

char *
gnome_vfs_make_uri_full_from_relative (const char *base_uri, const char *relative_uri)
{
	char *result = NULL;

	/* See section 5.2 in RFC 2396 */

	if (base_uri == NULL && relative_uri == NULL) {
		result = NULL;
	} else if (base_uri == NULL) {
		result = g_strdup (relative_uri);
	} else if (relative_uri == NULL) {
		result = g_strdup (base_uri);
	} else if (!is_uri_partial (relative_uri)) {
		result = g_strdup (relative_uri);
	} else {
		char *mutable_base_uri;
		char *mutable_uri;

		char *uri_current;
		size_t base_uri_length;
		char *separator;

		mutable_base_uri = g_strdup (base_uri);
		uri_current = mutable_uri = g_strdup (relative_uri);

		/* Chew off Fragment and Query from the base_url */

		separator = strrchr (mutable_base_uri, '#'); 

		if (separator) {
			*separator = '\0';
		}

		separator = strrchr (mutable_base_uri, '?');

		if (separator) {
			*separator = '\0';
		}

		if ('/' == uri_current[0] && '/' == uri_current [1]) {
			/* Relative URI's beginning with the authority
			 * component inherit only the scheme from their parents
			 */

			separator = strchr (mutable_base_uri, ':');

			if (separator) {
				separator[1] = '\0';
			}			  
		} else if ('/' == uri_current[0]) {
			/* Relative URI's beginning with '/' absolute-path based
			 * at the root of the base uri
			 */

			separator = strchr (mutable_base_uri, ':');

			/* g_assert (separator), really */
			if (separator) {
				/* If we start with //, skip past the authority section */
				if ('/' == separator[1] && '/' == separator[2]) {
					separator = strchr (separator + 3, '/');
					if (separator) {
						separator[0] = '\0';
					}
				} else {
				/* If there's no //, just assume the scheme is the root */
					separator[1] = '\0';
				}
			}
		} else if ('#' != uri_current[0]) {
			/* Handle the ".." convention for relative uri's */

			/* If there's a trailing '/' on base_url, treat base_url
			 * as a directory path.
			 * Otherwise, treat it as a file path, and chop off the filename
			 */

			base_uri_length = strlen (mutable_base_uri);
			if ('/' == mutable_base_uri[base_uri_length-1]) {
				/* Trim off '/' for the operation below */
				mutable_base_uri[base_uri_length-1] = 0;
			} else {
				separator = strrchr (mutable_base_uri, '/');
				if (separator) {
					*separator = '\0';
				}
			}

			remove_internal_relative_components (uri_current);

			/* handle the "../"'s at the beginning of the relative URI */
			while (0 == strncmp ("../", uri_current, 3)) {
				uri_current += 3;
				separator = strrchr (mutable_base_uri, '/');
				if (separator) {
					*separator = '\0';
				} else {
					/* <shrug> */
					break;
				}
			}

			/* handle a ".." at the end */
			if (uri_current[0] == '.' && uri_current[1] == '.' 
			    && uri_current[2] == '\0') {

			    	uri_current += 2;
				separator = strrchr (mutable_base_uri, '/');
				if (separator) {
					*separator = '\0';
				}
			}

			/* Re-append the '/' */
			mutable_base_uri [strlen(mutable_base_uri)+1] = '\0';
			mutable_base_uri [strlen(mutable_base_uri)] = '/';
		}

		result = g_strconcat (mutable_base_uri, uri_current, NULL);
		g_free (mutable_base_uri); 
		g_free (mutable_uri); 
	}
	
	return result;
}

GnomeVFSResult
_gnome_vfs_uri_resolve_all_symlinks_uri (GnomeVFSURI *uri,
					 GnomeVFSURI **result_uri)
{
	GnomeVFSURI *new_uri, *resolved_uri;
	GnomeVFSFileInfo *info;
	GnomeVFSResult res;
	char *p;
	int n_followed_symlinks;

	/* Ref the original uri so we don't lose it */
	uri = gnome_vfs_uri_ref (uri);

	*result_uri = NULL;

	info = gnome_vfs_file_info_new ();

	p = uri->text;
	n_followed_symlinks = 0;
	while (*p != 0) {
		while (*p == GNOME_VFS_URI_PATH_CHR)
			p++;
		while (*p != 0 && *p != GNOME_VFS_URI_PATH_CHR)
			p++;

		new_uri = gnome_vfs_uri_dup (uri);
		g_free (new_uri->text);
		new_uri->text = g_strndup (uri->text, p - uri->text);
		
		gnome_vfs_file_info_clear (info);
		res = gnome_vfs_get_file_info_uri (new_uri, info, GNOME_VFS_FILE_INFO_DEFAULT);
		if (res != GNOME_VFS_OK) {
			gnome_vfs_uri_unref (new_uri);
			goto out;
		}
		if (info->type == GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK &&
		    info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SYMLINK_NAME) {
			n_followed_symlinks++;
			if (n_followed_symlinks > MAX_SYMLINKS_FOLLOWED) {
				res = GNOME_VFS_ERROR_TOO_MANY_LINKS;
				gnome_vfs_uri_unref (new_uri);
				goto out;
			}
			resolved_uri = gnome_vfs_uri_resolve_relative (new_uri,
								       info->symlink_name);
			if (*p != 0) {
				gnome_vfs_uri_unref (uri);
				uri = gnome_vfs_uri_append_path (resolved_uri, p);
				gnome_vfs_uri_unref (resolved_uri);
			} else {
				gnome_vfs_uri_unref (uri);
				uri = resolved_uri;
			}

			p = uri->text;
		} 
		gnome_vfs_uri_unref (new_uri);
	}

	res = GNOME_VFS_OK;
	*result_uri = gnome_vfs_uri_dup (uri);
 out:
	gnome_vfs_file_info_unref (info);
	gnome_vfs_uri_unref (uri);
	return res;
}

GnomeVFSResult
_gnome_vfs_uri_resolve_all_symlinks (const char *text_uri,
				     char **resolved_text_uri)
{
	GnomeVFSURI *uri, *resolved_uri;
	GnomeVFSResult res;

	*resolved_text_uri = NULL;

	uri = gnome_vfs_uri_new (text_uri);
	if (uri == NULL || uri->text == NULL) {
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	}

	res = _gnome_vfs_uri_resolve_all_symlinks_uri (uri, &resolved_uri);

	if (res == GNOME_VFS_OK) {
		*resolved_text_uri = gnome_vfs_uri_to_string (resolved_uri, GNOME_VFS_URI_HIDE_NONE);
		gnome_vfs_uri_unref (resolved_uri);
	}
	return res;
}

gboolean 
_gnome_vfs_uri_is_in_subdir (GnomeVFSURI *uri, GnomeVFSURI *dir)
{
	GnomeVFSFileInfo *dirinfo, *info;
	GnomeVFSURI *resolved_dir, *parent, *tmp;
	GnomeVFSResult res;
	gboolean is_in_dir;

	resolved_dir = NULL;
	parent = NULL;

	is_in_dir = FALSE;
	
	dirinfo = gnome_vfs_file_info_new ();
	info = gnome_vfs_file_info_new ();

	res = gnome_vfs_get_file_info_uri (dir, dirinfo, GNOME_VFS_FILE_INFO_DEFAULT);
	if (res != GNOME_VFS_OK || dirinfo->type != GNOME_VFS_FILE_TYPE_DIRECTORY) {
		goto out;
	}

	res = _gnome_vfs_uri_resolve_all_symlinks_uri (dir, &resolved_dir);
	if (res != GNOME_VFS_OK) {
		goto out;
	}
	
	res = _gnome_vfs_uri_resolve_all_symlinks_uri (uri, &tmp);
	if (res != GNOME_VFS_OK) {
		goto out;
	}
	
	parent = gnome_vfs_uri_get_parent (tmp);
	gnome_vfs_uri_unref (tmp);

	while (parent != NULL) {
		res = gnome_vfs_get_file_info_uri (parent, info, GNOME_VFS_FILE_INFO_DEFAULT);
		if (res != GNOME_VFS_OK) {
			break;
		}

		if (dirinfo->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_DEVICE &&
		    dirinfo->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_INODE &&
		    info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_DEVICE &&
		    info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_INODE) {
			if (dirinfo->device == info->device &&
			    dirinfo->inode == info->inode) {
				is_in_dir = TRUE;
				break;
			}
		} else {
			if (gnome_vfs_uri_equal (dir, parent)) {
				is_in_dir = TRUE;
				break;
			}
		}
		
		tmp = gnome_vfs_uri_get_parent (parent);
		gnome_vfs_uri_unref (parent);
		parent = tmp;
	}

 out:
	if (resolved_dir != NULL) {
		gnome_vfs_uri_unref (resolved_dir);
	}
	if (parent != NULL) {
		gnome_vfs_uri_unref (parent);
	}
	gnome_vfs_file_info_unref (info);
	return is_in_dir;
}

			  
