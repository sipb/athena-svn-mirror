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
#include <bonobo-activation/bonobo-activation-private.h>
#include <bonobo-activation/bonobo-activation-i18n.h>

/*
 * FIXME: this code should be in glib.
 *
 */

/* ALEX:
 *        I collected the code fixes from libgnome and gnome-vfs and
 *        made them all use this code, since the dupliated alias tables
 *        were using quite a lot of memory.
 *        Even though this function is technically private it shouldn't
 *        be changed since gnome-vfs and libgnome use it too.
 */
#include <string.h>
#include <stdio.h>
#include <locale.h>

static GHashTable *alias_table = NULL;

/*read an alias file for the locales*/
static void
read_aliases (char *file)
{
  FILE *fp;
  char buf[256];
  if (!alias_table)
    alias_table = g_hash_table_new (g_str_hash, g_str_equal);
  fp = fopen (file,"r");
  if (!fp)
    return;
  while (fgets (buf,256,fp))
    {
      char *p, *q, *shared_q, *key, *val;

      g_strstrip(buf);
      
      /* Line is a comment */
      if (buf[0]=='#' || buf[0]=='\0')
	continue;

      /* Reads first column */
      for (p = buf, q = NULL; *p; p++) {
	if ((*p == '\t') || (*p == ' ') || (*p == ':')) {
	  *p = '\0';
	  q = p+1;
	  while ((*q == '\t') || (*q == ' ')) {
	    q++;
	  }
	  break;
	}
      }
      /* The line only had one column */
      if (!q || *q == '\0')
	continue;
      
      /* Read second column */
      for (p = q; *p; p++) {
	if ((*p == '\t') || (*p == ' ')) {
	  *p = '\0';
	  break;
	}
      }
      
      /* To save memory we only store each locale string once in the
       * hash-table (as a key). The value when you look up this key is
       * the same string as the key for the hashtable entry with that
       * string. If the value is NULL the key has no alias.
       */
      val = NULL;
      if (!g_hash_table_lookup_extended (alias_table, buf, NULL, (gpointer *)&val)) {
	g_hash_table_insert (alias_table, g_strdup (buf), NULL);
      }
      if (val == NULL) {
	/* There was no existing alias value for this locale */
	
	if (!g_hash_table_lookup_extended (alias_table, p, (gpointer *)&shared_q, NULL)) {
	  shared_q = g_strdup (q);
	  g_hash_table_insert (alias_table, shared_q, NULL);
	}
	g_hash_table_insert (alias_table, buf, shared_q);
      }
    }
  fclose (fp);
}

/*return the un-aliased language as a newly allocated string*/
static char *
unalias_lang (char *lang)
{
  char *p;
  int i;
  if (!alias_table)
    {
      read_aliases (BONOBO_ACTIVATION_LOCALEDIR "/locale.alias");
      read_aliases ("/usr/share/locale/locale.alias");
      read_aliases ("/usr/local/share/locale/locale.alias");
      read_aliases ("/usr/lib/X11/locale/locale.alias");
      read_aliases ("/usr/openwin/lib/locale/locale.alias");
    }
  i = 0;
  while ((p=g_hash_table_lookup(alias_table,lang)) && strcmp(p, lang))
    {
      lang = p;
	if (i++ == 30)
	{
          static gboolean said_before = FALSE;
	if (!said_before)
            g_warning (_("Too many alias levels for a locale, "
			 "may indicate a loop"));
	said_before = TRUE;
	return lang;
    }
    }
  return lang;
}

/* Mask for components of locale spec. The ordering here is from
 * least significant to most significant
 */
enum
{
  COMPONENT_CODESET =	1 << 0,
  COMPONENT_TERRITORY = 1 << 1,
  COMPONENT_MODIFIER =	1 << 2
};

/* Break an X/Open style locale specification into components
 */
static guint
explode_locale (const gchar *locale,
	gchar **language,
	gchar **territory,
	gchar **codeset,
	gchar **modifier)
{
  const gchar *uscore_pos;
  const gchar *at_pos;
  const gchar *dot_pos;

  guint mask = 0;

  uscore_pos = strchr (locale, '_');
  dot_pos = strchr (uscore_pos ? uscore_pos : locale, '.');
  at_pos = strchr (dot_pos ? dot_pos : (uscore_pos ? uscore_pos : locale), '@');

  if (at_pos)
    {
      mask |= COMPONENT_MODIFIER;
      *modifier = g_strdup (at_pos);
    }
  else
    at_pos = locale + strlen (locale);

  if (dot_pos)
    {
      mask |= COMPONENT_CODESET;
      *codeset = g_strndup (dot_pos, at_pos - dot_pos);
    }
  else
    dot_pos = at_pos;

  if (uscore_pos)
    {
      mask |= COMPONENT_TERRITORY;
      *territory = g_strndup (uscore_pos, dot_pos - uscore_pos);
    }
  else
    uscore_pos = dot_pos;

  *language = g_strndup (locale, uscore_pos - locale);

  return mask;
}

/*
 * Compute all interesting variants for a given locale name -
 * by stripping off different components of the value.
 *
 * For simplicity, we assume that the locale is in
 * X/Open format: language[_territory][.codeset][@modifier]
 *
 * TODO: Extend this to handle the CEN format (see the GNUlibc docs)
 *	 as well. We could just copy the code from glibc wholesale
 *	 but it is big, ugly, and complicated, so I'm reluctant
 *	 to do so when this should handle 99% of the time...
 */
static GList *
compute_locale_variants (const gchar *locale)
{
  GList *retval = NULL;

  gchar *language;
  gchar *territory;
  gchar *codeset;
  gchar *modifier;

  guint mask;
  guint i;

  g_return_val_if_fail (locale != NULL, NULL);

  mask = explode_locale (locale, &language, &territory, &codeset, &modifier);

  /* Iterate through all possible combinations, from least attractive
   * to most attractive.
   */
  for (i=0; i<=mask; i++)
    if ((i & ~mask) == 0)
	{
    gchar *val = g_strconcat(language,
		 (i & COMPONENT_TERRITORY) ? territory : "",
		 (i & COMPONENT_CODESET) ? codeset : "",
		 (i & COMPONENT_MODIFIER) ? modifier : "",
		 NULL);
    retval = g_list_prepend (retval, val);
	}

  g_free (language);
  if (mask & COMPONENT_CODESET)
    g_free (codeset);
  if (mask & COMPONENT_TERRITORY)
    g_free (territory);
  if (mask & COMPONENT_MODIFIER)
    g_free (modifier);

  return retval;
}

/* The following is (partly) taken from the gettext package.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.	*/

static const gchar *
guess_category_value (const gchar *categoryname)
{
  const gchar *retval;

  /* The highest priority value is the `LANGUAGE' environment
	variable.	This is a GNU extension.  */
  retval = g_getenv ("LANGUAGE");
  if (retval != NULL && retval[0] != '\0')
    return retval;

  /* `LANGUAGE' is not set.  So we have to proceed with the POSIX
	methods of looking to `LC_ALL', `LC_xxx', and `LANG'.  On some
	systems this can be done by the `setlocale' function itself.  */

  /* Setting of LC_ALL overwrites all other.  */
  retval = g_getenv ("LC_ALL");
  if (retval != NULL && retval[0] != '\0')
    return retval;

  /* Next comes the name of the desired category.  */
  retval = g_getenv (categoryname);
  if (retval != NULL && retval[0] != '\0')
    return retval;

  /* Last possibility is the LANG environment variable.	 */
  retval = g_getenv ("LANG");
  if (retval != NULL && retval[0] != '\0')
    return retval;

  return NULL;
}


static GHashTable *category_table = NULL;

/*
 * bonobo_activation_i18n_get_language_list:
 * @category_name: Name of category to look up, e.g. %"LC_MESSAGES".
 * 
 * This computes a list of language strings that the user wants.  It searches in
 * the standard environment variables to find the list, which is sorted in order
 * from most desirable to least desirable.  The `C' locale is appended to the
 * list if it does not already appear (other routines depend on this
 * behaviour). If @category_name is %NULL, then %LC_ALL is assumed.
 * 
 * Return value: the list of languages, this list should not be freed as it is
 * owned by gnome-i18n.
 */
const GList *
bonobo_activation_i18n_get_language_list (const gchar *category_name)
{
  GList *list;

  BONOBO_ACTIVATION_LOCK ();

  if (!category_name)
    category_name= "LC_ALL";

  if (category_table)
    {
      list= g_hash_table_lookup (category_table, (const gpointer) category_name);
    }
  else
    {
      category_table= g_hash_table_new (g_str_hash, g_str_equal);
      list= NULL;
    }

  if (!list)
    {
      gint c_locale_defined= FALSE;
  
      const gchar *category_value;
      gchar *category_memory, *orig_category_memory;

      category_value = guess_category_value (category_name);
      if (! category_value)
	category_value = "C";
      orig_category_memory = category_memory =
	g_malloc (strlen (category_value)+1);
      
      while (category_value[0] != '\0')
	{
	  while (category_value[0] != '\0' && category_value[0] == ':')
	    ++category_value;
	  
	  if (category_value[0] != '\0')
	    {
	      char *cp= category_memory;
	      
	      while (category_value[0] != '\0' && category_value[0] != ':')
		*category_memory++= *category_value++;
	      
	      category_memory[0]= '\0'; 
	      category_memory++;
	      
	      cp = unalias_lang(cp);
	      
	      if (strcmp (cp, "C") == 0)
		c_locale_defined= TRUE;
	      
	      list= g_list_concat (list, compute_locale_variants (cp));
	    }
	}

      g_free (orig_category_memory);
      
      if (!c_locale_defined)
	list= g_list_append (list, "C");

      g_hash_table_insert (category_table, (gpointer) category_name, list);
    }

  BONOBO_ACTIVATION_UNLOCK ();
  
  return list;
}

/* end of copy code from libgnome */


