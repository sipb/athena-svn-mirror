/*
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include "gnome-vfs-i18n.h"

#include "gnome-vfs-private-utils.h"
#include <errno.h>
#include <fcntl.h>
#include <glib/ghash.h>
#include <glib/gmessages.h>
#include <glib/gstrfuncs.h>
#include <glib/gutils.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unistd.h>

const GList *bonobo_activation_i18n_get_language_list (const gchar *category_name);

/**
 * gnome_vfs_i18n_get_language_list:
 * @category_name: Name of category to look up, e.g. "LC_MESSAGES".
 * 
 * This computes a list of language strings.  It searches in the
 * standard environment variables to find the list, which is sorted
 * in order from most desirable to least desirable.  The `C' locale
 * is appended to the list if it does not already appear (other
 * routines depend on this behaviour).
 * If @category_name is %NULL, then LC_ALL is assumed.
 * 
 * Return value: a copy of the list of languages (which you need to free).
 **/
GList *
gnome_vfs_i18n_get_language_list (const gchar *category_name)
{
  const GList *list;

  list = bonobo_activation_i18n_get_language_list (category_name);

  return g_list_copy ((GList *)list);
}
