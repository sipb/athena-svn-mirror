#ifndef _E_CACHE_H_
#define _E_CACHE_H_

/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    Author: Lauris Kaplinski  <lauris@helixcode.com>
*/

#include <sys/types.h>
#include <glib.h>
#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

typedef struct _ECache ECache;

typedef gpointer (* ECacheDupFunc) (gconstpointer data);
typedef void (* ECacheFreeFunc) (gpointer data);

ECache * e_cache_new (GHashFunc key_hash_func,
		      GCompareFunc key_compare_func,
		      ECacheDupFunc key_dup_func,
		      ECacheFreeFunc key_free_func,
		      ECacheFreeFunc object_free_func,
		      size_t softlimit,
		      size_t hardlimit);

void e_cache_ref (ECache * cache);
void e_cache_unref (ECache * cache);

gpointer e_cache_lookup (ECache * cache, gconstpointer key);
gpointer e_cache_lookup_notouch (ECache * cache, gconstpointer key);

gboolean e_cache_insert (ECache * cache, gpointer key, gpointer data, size_t size);

void e_cache_invalidate (ECache * cache, gpointer key);
void e_cache_invalidate_all (ECache * cache);

size_t e_cache_query_object_size (ECache * cache, gconstpointer key);

END_GNOME_DECLS

#endif
