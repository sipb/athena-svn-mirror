/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* This file implements the main athdir library calls. */

static const char rcsid[] = "$Id: athdir.c,v 1.6 1999-10-23 19:28:46 danw Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>		/* MAXPATHLEN */
#include <sys/types.h>
#include <sys/stat.h>
#include "athdir.h"
#include "stringlist.h"

/* Define HOSTTYPE for the platforms where `machtype`bin has historic
 * use.  Never add new platforms to this list.
 */
#if defined(HOSTTYPE_sun4)
#define HOSTTYPE "sun4"
#elif defined(HOSTTYPE_linux)
#define HOSTTYPE "linux"
#elif defined(HOSTTYPE_inbsd)
#define HOSTTYPE "inbsd"
#endif

#ifdef HOSTTYPE
char *hosttype = HOSTTYPE;
#else
char *hosttype = NULL;
#endif

/* Definition of known conventions and what flavors they are.  */

typedef struct {
  char *name;
  int flavor;
} Convention;

#define ARCHflavor (1<<0)
#define MACHflavor (1<<1)
#define PLAINflavor (1<<2)
#define SYSflavor (1<<3)
#define DEPENDENTflavor (1<<8)
#define INDEPENDENTflavor (1<<9)
#define ATSYSflag (1<<16)

#define NUMCONVENTIONS 5

Convention conventions[NUMCONVENTIONS] = {
  { NULL,		ARCHflavor | MACHflavor | PLAINflavor |
      			DEPENDENTflavor | INDEPENDENTflavor },
  { "%p/arch/%s/%t",	ARCHflavor | DEPENDENTflavor | ATSYSflag },
  { "%p/%s/%t",		SYSflavor | DEPENDENTflavor | ATSYSflag },
  { "%p/%m%t",		MACHflavor | DEPENDENTflavor },
  { "%p/%t",		PLAINflavor | INDEPENDENTflavor }
};

/* Editorial tagging for what conventions are acceptable or
 * preferable for what types.
 */

typedef struct {
  char *type;
  int allowedFlavors;	/* searching parameters */
  int preferredFlavor;	/* creating paramaters */
} Editorial;

Editorial editorials[] = {
  { "bin",	ARCHflavor | SYSflavor | MACHflavor,	DEPENDENTflavor },
  { "lib",	ARCHflavor | SYSflavor | MACHflavor,	DEPENDENTflavor },
  { "etc",	ARCHflavor | SYSflavor | MACHflavor,	DEPENDENTflavor },
  { "man",	ARCHflavor | PLAINflavor,		INDEPENDENTflavor },
  { "include",	ARCHflavor | PLAINflavor,		INDEPENDENTflavor },
  { NULL,	ARCHflavor | PLAINflavor,		DEPENDENTflavor }
};

/* path = template(dir, type, sys, machine)
 *	%p = path (dir)
 *	%t = type
 *	%s = sys
 *	%m = machine
 * %foo is inserted if the corresponding string is NULL.
 * If this happens, expand returns 1 (not a valid path).
 * If expand runs out of memory, it returns -1 (and path is NULL).
 * Otherwise, expand returns 0.
 */
static int expand(char **path, char *template, char *dir,
	   char *type, char *sys, char *machine)
{
  char *src, *dst, *oldpath;
  int somenull = 0, size;

  src = template;
  size = strlen(template) + 1;
  dst = oldpath = *path = malloc(size);
  *dst = '\0';

  while (*src != '\0')
    {
      if (*src != '%')
	*dst++ = *src++;
      else
	{

#define copystring(casec, cases, string)			\
	case casec:						\
	  src++;						\
	  if (string)						\
	    {							\
	      *path = realloc(*path, size += strlen(string));	\
	      if (!*path)					\
		return -1;					\
	      dst = *path + (dst - oldpath);			\
	      oldpath = *path;					\
	      strcpy(dst, string);				\
	      dst += strlen(string);				\
	    }							\
	  else							\
	    {							\
	      strcpy(dst, cases);				\
	      dst += 2;						\
	      somenull = 1;					\
	    }							\
	  break;

	  src++;
	  switch(*src)
	    {
	      copystring('p', "%p", dir);
	      copystring('t', "%t", type);
	      copystring('s', "%s", sys);
	      copystring('m', "%m", machine);

#undef copystring

	    case '\0':
	      break;

	    default:
	      *dst++ = '%';
	      *dst++ = *src++;
	      break;
	    }
	}
    }

  *dst = '\0';
  return somenull;
}

static int template_flavor(char *template)
{
  int flavor = ARCHflavor | MACHflavor | PLAINflavor
    | DEPENDENTflavor | INDEPENDENTflavor;
  char *ptr;

  if (template == NULL)
    return 0;

  for (ptr = strchr(template, '%'); ptr != NULL; ptr = strchr(ptr + 1, '%'))
    {
      if (*(ptr+1) == 's')
	flavor |= ATSYSflag;
    }

  return flavor;
}

int athdir_native(char *what, char *sys)
{
  char *ptr;

  /* If sys is NULL, fall back to ATHENA_SYS if it's set, otherwise
   * use the compiled-in value.
   */
  if (sys == NULL)
    {
      sys = getenv("ATHENA_SYS");
      if (sys == NULL || strchr(sys, '/') != NULL)
	sys = ATHSYS;
    }

  for (ptr = strchr(what, sys[0]); ptr != NULL; ptr = strchr(ptr + 1, sys[0]))
    {
      if (!strncmp(ptr, sys, strlen(sys)))
	return 1;
    }

  return 0;
}

/* You are in a twisty little maze of interconnecting flags, all different.
 */
char **athdir_get_paths(char *base_path, char *type,
			char *sys, char **syscompat, char *machine,
			char *aux, int flags)
{
  char *path;
  string_list *path_list = NULL, *compat_list = NULL;
  int t, j, complete, preferredFlavor, want_break;
  struct stat statbuf;
  char **current_compat, **mysyscompat = NULL, *compat_env;

  /* If sys is NULL, fall back to ATHENA_SYS if it's set, otherwise
   * use the compiled-in value.
   */
  if (sys == NULL)
    {
      sys = getenv("ATHENA_SYS");
      if (sys == NULL || strchr(sys, '/') != NULL)
	sys = ATHSYS;
    }

  /* Generate the syscompat array from the environment if it wasn't
   * passed in.
   */
  if (syscompat == NULL)
    {
      /* We're compatible with ourselves. */
      if (athdir__add_string(&compat_list, sys, 0))
	return NULL;
      compat_env = getenv("ATHENA_SYS_COMPAT");
      if (compat_env != NULL && !strchr(compat_env, '/'))
	{
	  if (athdir__parse_string(&compat_list, compat_env, ':'))
	    return NULL;
	}
      syscompat = mysyscompat = athdir__make_string_array(&compat_list);
    }

  /* If machine is NULL, use whatever was compiled in. */
  if (machine == NULL)
    machine = hosttype;

  /* Zeroeth convention is optionally provided by the caller. */
  conventions[0].name = aux;
  conventions[0].flavor = template_flavor(aux);

  /* Find matching editorial for the type of directory requested
   * (to be consulted later).
   */
  for (t = 0; editorials[t].type != NULL; t++)
    {
      if (type != NULL && !strcmp(type, editorials[t].type))
	break;
    }

  if (flags & ATHDIR_MACHINEDEPENDENT)
    preferredFlavor = DEPENDENTflavor;
  else
    {
      if (flags & ATHDIR_MACHINEINDEPENDENT)
	preferredFlavor = INDEPENDENTflavor;
      else
	preferredFlavor = editorials[t].preferredFlavor;
    }

  /* Cycle through matching conventions */
  for (j = 0; j < NUMCONVENTIONS; j++)
    {
      if (conventions[j].name == NULL)
	continue; /* conventions[0], the caller specified convention,
		   * is not set. */
      want_break = 0;

      if (
	  /* If the editorial says this is a reasonable convention
	   * for this type
	   */
	  (editorials[t].allowedFlavors & conventions[j].flavor) ||

	  /* or we don't care what the editorials say */
	  (flags & ATHDIR_SUPPRESSEDITORIALS) || 

	  /* or something more explicit than the editorials has been
	   * specified
	   */
	  (((ATHDIR_MACHINEDEPENDENT | ATHDIR_MACHINEINDEPENDENT) & flags) &&
	   (flags & ATHDIR_SUPPRESSSEARCH)))
	{
	  /* If we're looking for a specific flavor (machine dependent/
	   * machine independent) and this isn't it, keep going.
	   */
	  if ((ATHDIR_SUPPRESSSEARCH & flags) &&
	      !(preferredFlavor & conventions[j].flavor))
	    continue;

	  for (current_compat = syscompat; *current_compat != NULL;
	       current_compat++)
	    {
	      complete = !expand(&path, conventions[j].name,
				 base_path, type, *current_compat, hosttype);

	      if (!path)
		return NULL;

	      /* If we're listing everything, or we only care about the
	       * first match for creation purposes (both cases where we
	       * don't want to stat()), store this match.
	       */
	      if ((flags & ATHDIR_LISTSEARCHDIRECTORIES) ||
		  ((flags & ATHDIR_SUPPRESSSEARCH) && complete))
		{
		  if (athdir__add_string(&path_list, path, 0))
		    {
		      free(path);
		      return NULL;
		    }
		  free(path);
		  
		  /* In this case (first match for creation) we're done. */
		  if (flags & ATHDIR_SUPPRESSSEARCH)
		    {
		      want_break = 1;
		      break;
		    }
		}
	      else /* If it's there, store it and be done. */
		if (complete && !stat(path, &statbuf))
		  {
		    if (athdir__add_string(&path_list, path, 0))
		      {
			free(path);
			return NULL;
		      }
		    free(path);
		    want_break = 1;
		    break;
		  }
	      else
		free(path);

	      /* Don't loop over @sys values unless ATSYSflag. */
	      if (!(conventions[j].flavor & ATSYSflag))
		break;
	    }
	}

      if (want_break)
	break;
    }

  athdir__free_string_array(mysyscompat);
  return athdir__make_string_array(&path_list);
}

void athdir_free_paths(char **paths)
{
  athdir__free_string_array(paths);
}
