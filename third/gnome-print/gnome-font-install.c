/* gnome-font-install.c - Search for .font files and combine them into
 * one single fontmap file.
 *
 * Copyright (C) 1999 Chris Lahey.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.  */

/* If the glyphs field starts with *, it takes the text between the *
 * and the first / and uses it as the key to search a list of paths
 * set using --pfb-assignment.  It uses the rest of the field as the
 * rest of the path.  So
 * --pfb-assignment=ghostscript,/usr/share/ghostscript/fonts would
 * cause *ghostscript/something.pfb to be searched for as
 * /usr/share/ghostscript/fonts/something.pfb.  You can have multiple
 * --pfb-assignment options for the same key, in which case, fonts
 * using that key will search in multiple locations.  This all applies
 * to the metrics field and the command option --afm-assignment.
 *
 * If the glyphs field starts with neither a * or a /, then each
 * --pfb-path is searched in term.  Again, the same applies to the
 * metrics field and --afm-path.
 *
 * */
#include "config.h"
#include <gnome.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <xmlmemory.h>
#if defined(LIBXML_VERSION) && LIBXML_VERSION >= 20000
#include <libxml/parser.h>
#include <libxml/parser.h>
#define root children
#define childs children
#else
#include <gnome-xml/parser.h>
#include <gnome-xml/xmlmemory.h>
#endif


/* This program does not delete everything it allocates since it's
   just an installer program. */

#if 0
gboolean test_only;
gboolean verbose;
#endif
static GHashTable *included_fonts;
static gboolean debug = FALSE;
static GString *fontmap_path = NULL;
static GSList *afm_path = NULL;
static GSList *pfb_path = NULL;
static GHashTable *afm_assignments;
static GHashTable *pfb_assignments;

/* These three variables are ignored, but they are set by the command
   line arguments. */
static gboolean sys = FALSE;
static gboolean scan = FALSE;
static gboolean no_copy = FALSE;

typedef struct format_name
{
  gchar *format;
  gchar *name;
} format_name;

static gint
g_format_name_equal (gconstpointer v, gconstpointer v2)
{
  const format_name *a = v;
  const format_name *a2 = v2;
  return ( ( ( a->format == a2->format ) || ( !strcmp( a->format, a2->format ) ) ) &&
	   ( ( a->name   == a2->name   ) || ( !strcmp( a->name,   a2->name   ) ) ) );
}

static guint
g_format_name_hash (gconstpointer v)
{
  guint h = 0;
  const format_name *a = v;
  if ( a->format )
    h ^= g_str_hash( a->format );
  if ( a->name )
    h ^= g_str_hash( a->name );

  return h /* % M */;
}


/*
 * Get a value for a node either carried as an attibute or as
 * the content of a child.
 */
static char *
xmlGetValue (xmlNodePtr node, const char *name)
{
  char *ret;
  xmlNodePtr child;

  ret = (char *) xmlGetProp (node, name);
  if (ret != NULL)
    {
      char *gret;
      gret = g_strdup (ret);
      xmlFree (ret);
      return gret;
    }

  child = node->childs;
  while (child != NULL)
    {
      if (!strcmp (child->name, name))
	{
	  /*
	   * !!! Inefficient, but ...
	   */
	  ret = xmlNodeGetContent(child);
	  if (ret != NULL)
	    return (ret);
	}
      child = child->next;
    }

  return NULL;
}

/*
 * Set a string value for a node either carried as an attibute or as
 * the content of a child.
 */
static void
xmlSetValue (xmlNodePtr node, const char *name, const char *val)
{
  char *ret;
  xmlNodePtr child;

  ret = xmlGetProp (node, name);
  if (ret != NULL)
    {
      xmlFree (ret);
      xmlSetProp (node, name, val);
      return;
    }

  child = node->childs;
  while (child != NULL)
    {
      if (!strcmp (child->name, name))
	{
	  xmlNodeSetContent (child, val);
	  return;
	}
      child = child->next;
    }
  xmlSetProp (node, name, val);
}

static gboolean
already_included( const gchar *format, const gchar *name )
{
  format_name newfont;
  /* The following two casts don't mean that we are going to change things.  */
  newfont.format = (gchar *)format;
  newfont.name = (gchar *)name;
  return g_hash_table_lookup( included_fonts, &newfont ) != NULL;
}

static void
append_font( xmlDoc *fontmap, xmlNode *font )
{
  format_name *newfont = g_new( format_name, 1 );
  newfont->format = xmlGetValue( font, "format" );
  newfont->name = xmlGetValue( font, "name" );
  g_hash_table_insert( included_fonts,
		       newfont, newfont );
  xmlUnlinkNode( font );
  xmlAddChild( fontmap->root, font );
}

static void
free_font (gpointer key, gpointer value, gpointer user_data)
{
  format_name *font;

  font = key;
  g_free (font->format);
  g_free (font->name);
  g_free (key);
}


static gchar *
search_path_for_file( GSList *path, const gchar *file )
{
  struct stat buf;
  for ( ; path != NULL; path = path->next )
    {
      gchar *tempfile = g_strdup_printf( "%s/%s", (gchar *) path->data, file );
      if ( tempfile && !stat( tempfile, &buf ) )
	{
	  return tempfile;
	}
      else
        g_free( tempfile );
    }

  return NULL;
}

static gint
check_font( xmlDoc *fontmap, xmlNode *font )
{
  struct stat buf;
  gchar *name = xmlGetValue( font, "name" );
  gchar *format = xmlGetValue( font, "format" );
  gchar *glyphs = xmlGetValue( font, "glyphs" );
  gchar *metrics = xmlGetValue( font, "metrics" );
  gint ret_val = 0;
  gboolean glyphs_found = FALSE;
  gboolean metrics_found = FALSE;
  if ( ! already_included( format, name ) )
    {
      if ( *glyphs == '/' )
	{
	  if ( glyphs && !stat( glyphs, &buf ) )
	    glyphs_found = TRUE;
	  else if ( debug )
	    g_print( "file: %s not found.\n", glyphs );
	}
      else if ( *glyphs == '*' )
	{
	  gchar *first_slash = strchr( glyphs, '/' );
	  GSList *path = NULL;
	  if ( first_slash )
	    {
	      gchar *tempglyphs;
	      *first_slash = 0;
	      path = g_hash_table_lookup( pfb_assignments, glyphs + 1);
	      *first_slash = '/';
	      tempglyphs = search_path_for_file( path,
						 first_slash + 1 );
	      if ( tempglyphs )
		{
		  glyphs_found = TRUE;
		  xmlSetValue( font, "glyphs", tempglyphs );
		  g_free( glyphs );
		  glyphs = tempglyphs;
		}
	    }
	  if ( debug && !glyphs_found )
	    g_print( "file: %s not found.\n", glyphs );
	}
      else
	{
	  gchar *tempglyphs = search_path_for_file( pfb_path,
						    glyphs );
	  if ( tempglyphs )
	    {
	      glyphs_found = TRUE;
	      xmlSetValue( font, "glyphs", tempglyphs );
	      g_free( glyphs );
	      glyphs = tempglyphs;
	    }
	  else if ( debug )
	    g_print( "file: %s not found.\n", glyphs );
	}
      if ( *metrics == '/' )
	{
	  if ( metrics && !stat( metrics, &buf ) )
	    metrics_found = TRUE;
	  else if ( debug )
	    g_print( "file: %s not found.\n", metrics );
	}
      else if ( *metrics == '*' )
	{
	  gchar *first_slash = strchr( metrics, '/' );
	  GSList *path = NULL;
	  if ( first_slash )
	    {
	      gchar *tempmetrics;
	      *first_slash = 0;
	      path = g_hash_table_lookup( afm_assignments, metrics + 1 );
	      *first_slash = '/';
	      tempmetrics = search_path_for_file( path,
						  first_slash + 1 );
	      if ( tempmetrics )
		{
		  metrics_found = TRUE;
		  xmlSetValue( font, "metrics", tempmetrics );
		  g_free( metrics );
		  metrics = tempmetrics;
		}
	    }
	  if ( debug && !metrics_found )
	    g_print( "file: %s not found.\n", metrics );
	}
      else
	{
	  gchar *tempmetrics = search_path_for_file( afm_path,
						     metrics );
	  if ( tempmetrics )
	    {
	      metrics_found = TRUE;
	      xmlSetValue( font, "metrics", tempmetrics );
	      g_free( metrics );
	      metrics = tempmetrics;
	    }
	  else if ( debug )
	    g_print( "file: %s not found.\n", metrics );
	}
      if ( glyphs_found && metrics_found )
	{
	  append_font( fontmap, font );
	  ret_val = 1;
	}
    }
  else
    {
      if ( debug )
	g_print( "%s:%s already included.\n", format, name );
    }
  g_free( name );
  g_free( format );
  g_free( glyphs );
  g_free( metrics );
  return ret_val;
}

static void
check_fontfile( xmlDoc *fontmap, const gchar *filename )
{
  xmlDoc *fontfile;
  
#if defined(LIBXML_VERSION) && LIBXML_VERSION >= 20000
  xmlKeepBlanksDefault(0);
#endif
  fontfile = xmlParseFile( filename );
  if ( fontfile && fontfile->root && fontfile->root->name ) /* && fontfile->root->ns && fontfile->root->ns->href && ( ! strcmp( fontfile->root->ns->href, "http://www.gnome.org/gnome-font/0.0" ) ) ) */
    {
      if (! strcmp( fontfile->root->name, "font" ))
	{
	  if ( check_font( fontmap, fontfile->root ) )
	    {
	      fontfile->root = NULL;
	    }
	}
      else if (! strcmp( fontfile->root->name, "fontfile"))
	{
	  xmlNode *font = fontfile->root->childs;
	  while(font)
	    {
	      xmlNode *next = font->next;
	      check_font( fontmap, font );
	      font = next;
	    }
	}
      xmlFreeDoc( fontfile );
    }
  else
    {
      g_warning( "font file %s is in a non-standard format.", filename );
    }
}

static void
scan_fonts( xmlDoc *fontmap, const gchar *path )
{
  DIR *dir;
  struct dirent *direntry;

  dir = opendir( path );

  if ( dir )
    {
      while ( ( direntry = readdir( dir ) ) )
	{
	  if ( strrchr( direntry->d_name, '.' ) && ! strcmp( strrchr( direntry->d_name, '.' ), ".font" ) )
	    {
	      const gchar *shortname;
	      if ( strrchr( direntry->d_name, '/' ) )
		shortname = strrchr( direntry->d_name, '/' ) + 1;
	      else
		shortname = direntry->d_name;
	      if ( strcmp( shortname, "." ) && strcmp( shortname, ".." ) )
		{
		  gchar *long_name;
		  if ( *direntry->d_name != '/' )
		    long_name = g_strdup_printf( "%s/%s", path, direntry->d_name );
		  else
		    long_name = direntry->d_name;

		  check_fontfile( fontmap, long_name );

		  if ( *direntry->d_name != '/' )
		    g_free( long_name );
		}
	    }
	}
      closedir( dir );
    }
}

static xmlDoc *
read_fontmap(const gchar *location)
{
  xmlDoc *doc;

#if defined(LIBXML_VERSION) && LIBXML_VERSION >= 20000
  xmlKeepBlanksDefault(0);
#endif
  doc = xmlParseFile( location );
  if ( doc && doc->root && doc->root->name && ( ! strcmp( doc->root->name, "fontmap" ) ) ) /* && doc->root->ns && doc->root->ns->href && ( ! strcmp( doc->root->ns->href, "http://www.gnome.org/gnome-font/0.0" ) ) ) */
    {
      xmlNode *font = doc->root->childs;
      while(font)
	{
	  format_name *newfont = g_new( format_name, 1 );
	  newfont->format = xmlGetValue( font, "format" );
	  newfont->name = xmlGetValue( font, "name" );
	  g_hash_table_insert( included_fonts,
			       newfont, newfont );
	  font = font->next;
	}
      return doc;
    }
  else
    {
      return NULL;
    }
}

static gint
write_fontmap( xmlDoc *fontmap, const gchar *location )
{
  return xmlSaveFile (location, fontmap);
}

static void
check_directory(void)
{
  struct stat info;
  char *dirname = DATADIR "/fonts";
  if (fontmap_path != NULL) 
    {
       dirname = fontmap_path->str;
    }
  if ( stat( dirname, &info ) )
    {
      switch ( errno )
	{
	case ENOENT:
	  if ( mkdir( dirname, 0777 ) )
	    {
	      switch( errno )
		{
		case EEXIST:
		  /* Someone beat us to it.  */
		  break;
		default:
		  g_error( "Unable to create directory %s.", dirname );
		  break;
		}
	    }
	  break;
	}
    }
}

static void
add_assignment(poptContext ctx,
	 enum poptCallbackReason reason,
	 const struct poptOption *opt,
	 const char *arg, void *data)
{
  struct stat info;
  if ( opt->shortName == 'A' )
    {
      gchar *comma;
      gchar *path;
      gchar *key;

      comma = strchr( (char *) arg, ',' );
      if ( comma )
	{
	  *comma = 0;
	  path = g_strdup( comma + 1 );
	  if ( !stat( path, &info ) )
	    {
	      key = g_strdup( (char *) arg );
	      g_hash_table_insert( afm_assignments, key, g_slist_append( g_hash_table_lookup( afm_assignments, key ), path ) );
	    }
	  else
	    g_free (path);
	}
    }
  else if ( opt->shortName == 'P' )
    {
      gchar *comma;
      gchar *path;
      gchar *key;

      comma = strchr( (char *) arg, ',' );
      if ( comma )
	{
	  *comma = 0;
	  path = g_strdup( comma + 1 );
	  if ( !stat( path, &info ) )
	    {
	      key = g_strdup( (char *) arg );
	      g_hash_table_insert( pfb_assignments, key, g_slist_append( g_hash_table_lookup( pfb_assignments, key ), path ) );
	    }
	  else
	    g_free (path);

	  *comma = ',';
	}
    }
  /* else something's weird :) */
}

static void
free_assignment (gpointer key, gpointer value, gpointer user_data)
{
  GSList *path, *tmp;

  g_free (key);

  for (tmp = path = value; tmp; tmp = tmp->next)
    g_free (tmp->data);
  g_slist_free (path);
}

static void
add_path(poptContext ctx,
	 enum poptCallbackReason reason,
	 const struct poptOption *opt,
	 const char *arg, void *data)
{
  struct stat info;
  if(opt->shortName == 'a')
    {
      if ( !stat( (char *) arg, &info ) )
	{
	  afm_path = g_slist_append(afm_path, (char *) arg);
	}
    }
  else if(opt->shortName == 'p')
    {
      if ( !stat( (char *) arg, &info ) )
	{
	  pfb_path = g_slist_append(pfb_path, (char *) arg);
	}
    }
  else if(opt->shortName == 'f')
    {
        fontmap_path = g_string_new((gchar *) arg);
    }
  /* else something's weird :) */
}

static const struct poptOption options[] = {
  { "debug", 'd', POPT_ARG_NONE, &debug, 0,
    N_("Print out debugging information"), NULL },
  { "system", 's', POPT_ARG_NONE, &sys, 0,
    N_("Does nothing yet."), NULL },
  { "scan", 'c', POPT_ARG_NONE, &scan, 0,
    N_("Does nothing yet."), NULL },
  { "no-copy", 'n', POPT_ARG_NONE, &no_copy, 0,
    N_("Does nothing yet."), NULL },
  /* We want to use a callback for this launch-plugin thing so we
     can get multiple opts */
  { NULL, '\0', POPT_ARG_CALLBACK, &add_path, 0 },
  { "fontmap-path", 'f', POPT_ARG_STRING, NULL, 0,
    N_("Use this path to write the fontmap file."), N_("PATH") },
  { "afm-path", 'a', POPT_ARG_STRING, NULL, 0,
    N_("Use this path to search for relative afm files."), N_("PATH") },
  { "pfb-path", 'p', POPT_ARG_STRING, NULL, 0,
    N_("Use this path to search for relative pfb files."), N_("PATH") },
  { NULL, '\0', POPT_ARG_CALLBACK, &add_assignment, 0 },
  { "afm-assignment", 'A', POPT_ARG_STRING, NULL, 0,
    N_("Use this to set up a key/value pair for afm files."), N_("PATH") },
  { "pfb-assignment", 'P', POPT_ARG_STRING, NULL, 0,
    N_("Use this to set up a key/value pair for pfb files."), N_("PATH") },
  { NULL, '\0', 0, NULL, 0 }
};

static poptContext ctx;

static void
init( gint argc, gchar *argv[] )
{
  int flags = 0;

  gnomelib_init ("gnome-font-install", VERSION);

  if(options)
    {
      const char *appdesc = "gnome-font-install options";
      gnomelib_register_popt_table(options, appdesc);
    }

  ctx = gnomelib_parse_args(argc, argv, flags);

  /* reduce mem usage (hopefully) */
  gnome_config_sync();
  g_blow_chunks();
}

static void
warn_missing_font (const char *font_name)
{
  g_warning ("Warning: the %s font was not found. Printing is unlikely\n"
	     "to work unless basic fonts are installed. Please see:\n"
	     "\n"
	     "http://www.levien.com/gnome/font-install.html\n"
	     "\n"
	     "for more information.", font_name);
}

int
main (int argc, char *argv[])
{
  xmlDoc *fontmap;
  GString *fontmap_file = NULL;
  gchar *fontmap_loc = DATADIR "/fonts/fontmap";
  gint i;
  gboolean scan_dot = TRUE;
  char **args;

  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  afm_assignments = g_hash_table_new( g_str_hash,
				      g_str_equal );
  pfb_assignments = g_hash_table_new( g_str_hash,
				      g_str_equal );

  init( argc, argv );

  check_directory();

  if (fontmap_path != NULL) 
    {
       fontmap_file = g_string_append(fontmap_path,"/fontmap");
       fontmap_loc = fontmap_file->str;
    }
    printf("%s\n",fontmap_loc);

  included_fonts = g_hash_table_new( g_format_name_hash,
				     g_format_name_equal );

  /* I want this to overwrite the old data until the .font files are better reviewed. */
  if (0)
    fontmap = read_fontmap( fontmap_loc );
  else
    fontmap = NULL;
  if ( fontmap == NULL )
    {
      fontmap = xmlNewDoc( "1.0" );
      if ( fontmap == NULL )
	{
	  g_error( "Unable to read old fontmap and unable to open current fontmap." );
	  return -1;
	}
      fontmap->root = xmlNewDocNode( fontmap, NULL, "fontmap", NULL );
    }

  args = poptGetArgs(ctx);
  for(i = 0; args && args[i]; i++)
    {
      scan_fonts( fontmap, args[i] );
      scan_dot = FALSE;
    }
  if ( scan_dot )
    scan_fonts( fontmap, "." );
  if (write_fontmap( fontmap, fontmap_loc ) == -1)
    {
      fprintf (stderr, "Failed to write fontmap file (%s).\n", fontmap_loc);
      return 1;
    }

  poptFreeContext(ctx);
  xmlFreeDoc (fontmap);

  if (!already_included ("type1", "Times-Roman"))
    warn_missing_font ("Times");
  else if (!already_included ("type1", "Helvetica"))
    warn_missing_font ("Helvetica");
  else if (!already_included ("type1", "Courier"))
    warn_missing_font ("Courier");

  g_hash_table_foreach (afm_assignments, free_assignment, NULL);
  g_hash_table_destroy (afm_assignments);

  g_hash_table_foreach (pfb_assignments, free_assignment, NULL);
  g_hash_table_destroy (pfb_assignments);

  g_hash_table_foreach (included_fonts, free_font, NULL);
  g_hash_table_destroy (included_fonts);

  return 0;
}
