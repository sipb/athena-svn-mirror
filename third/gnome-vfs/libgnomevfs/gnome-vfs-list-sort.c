/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-list-sort.c - GList sorting function for the GNOME
   Virtual File System.

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

   Author: Sven Oliver <sven.over@ob.kamp.net>
   Modified by Ettore Perazzoli <ettore@comm2000.it> to let the compare
   functions get an additional gpointer parameter.  
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"

/* This is the same as the GLIB implementation of `g_list_sort()', but
   uses GnomeVFSListCompareFunc instead of GCompareFunc, which lets us
   pass an additional gpointer parameter to the sorting function.  */

static GList *
gnome_vfs_list_sort_merge (GList *l1,
			   GList *l2,
			   GnomeVFSListCompareFunc compare_func,
			   gpointer data)
{
	GList list, *l, *lprev;

	l = &list; 
	lprev = NULL;

	while (l1 && l2) {
		if (compare_func (l1->data, l2->data, data) < 0) {
			l->next = l1;
			l = l->next;
			l->prev = lprev; 
			lprev = l;
			l1 = l1->next;
		} else {
			l->next = l2;
			l = l->next;
			l->prev = lprev; 
			lprev = l;
			l2 = l2->next;
		}
	}

	l->next = l1 ? l1 : l2;
	l->next->prev = l;

	return list.next;
}

GList *
gnome_vfs_list_sort (GList *list,
		     GnomeVFSListCompareFunc compare_func,
		     gpointer data)
{
	GList *l1, *l2;
  
	if (!list) 
		return NULL;
	if (!list->next) 
		return list;
  
	l1 = list; 
	l2 = list->next;

	while ((l2 = l2->next) != NULL) {
		if ((l2 = l2->next) == NULL) 
			break;
		l1 = l1->next;
	}

	l2 = l1->next; 
	l1->next = NULL; 

	return gnome_vfs_list_sort_merge
		(gnome_vfs_list_sort (list, compare_func, data),
		 gnome_vfs_list_sort (l2, compare_func, data),
		 compare_func,
		 data);
}

