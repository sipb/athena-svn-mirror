/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/dash/compile_menu.c,v $
 * $Author: ghudson $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/dash/compile_menu.c,v 1.7 1996-09-19 22:20:38 ghudson Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <X11/Xos.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Jets.h>
#include <Window.h>
#include <Menu.h>
#include <Button.h>
#include <warn.h>

#include <sys/time.h>


#define FONTNAME \
 "-Adobe-New Century Schoolbook-Medium-R-Normal--14-140-75-75-P-82-ISO8859-1"

#if defined(NEED_ERRNO_DEFS)
extern int errno;
extern char *sys_errlist[];
extern int sys_nerr;
#endif


MenuClassRec menuClassRec = {
  {
    /* class name */		"Menu",
    /* jet size   */		sizeof(MenuRec),
    /* classInitialize */	NULL,
    /* classInitialized? */	1,
    /* initialize */		NULL,
    /* prerealize */    	NULL,
    /* realize */		NULL,
    /* event */			NULL,
    /* expose */		NULL,
    /* querySize */     	NULL,
    /* move */			NULL,
    /* resize */        	NULL,
    /* destroy */       	NULL,
    /* resources */		NULL,
    /* number of 'em */		0,
  }
};

JetClass menuJetClass = (JetClass)&menuClassRec;


char *ConvertStringToCallback(address)
     char **address;
{
  char *ptr, *end, *ret_str;
  int barfed = 0;
  static char ret[BUFSIZ];
  char *ret_ptr;

  memset(ret, 0, BUFSIZ);
  ret_ptr=ret;
  
  if (*address == NULL)
    return NULL;

  ptr = *address;

 begin:
  while (isspace(*ptr)) ptr++;

  end = strchr(ptr, '(');
  if (end == NULL)
    {
      /* we don't advance the pointer in this case. */
      fprintf(stderr, "missing '(' in callback string: %s\n", ptr);
      return NULL;
    }

  strncpy(ret_ptr, ptr, end - ptr);
  ret_ptr += (end-ptr);
  *ret_ptr++ = *end;

  ptr = end + 1;
  while (isspace(*ptr))
    ptr++;

  if (isdigit(*ptr)  ||  *ptr == '-')
    while (isdigit(*ptr)  ||  *ptr == '-')
      *ret_ptr++ = *ptr++;

  else if (*ptr != ')')
    {
      char delim = *ptr;
      *ret_ptr++ = *ptr++;
      end = strchr(ptr, delim);
      if (end == NULL)
	{
	  fprintf(stderr, "missing close quote in callback string: %s\n",
		  *address);
	  barfed = 1;
	}
      else
	{
	  strncpy(ret_ptr, ptr, end+1-ptr);
	  ret_ptr += (end+1-ptr);
	  
	  ptr = end + 1;
	}
    }

  end = strchr(ptr, ')');
  if (end == NULL || barfed)
    {
      if (!barfed)
	{
	  fprintf(stderr, "missing close parenthesis in callback string: %s\n",
		  *address);
	}

      /* bugs in here - if first callback unknown, rest oddly punted */

      if (end != NULL)
	ptr = end + 1;

      *address = ptr;
      return NULL;
    }

  *ret_ptr++ = *end;
  ptr = end + 1;

  while (isspace(*ptr)) ptr++;
  if (*ptr == ',')
    {
      *ret_ptr++ = *ptr++;
      goto begin;
    }

  *address = ptr;
  ret_str = XjNewString(ret);
  return ret_str;
}


/************************************************************************
 *
 *  stringToQuark  --  quarkifies first string on comma-seperated list
 *	and returns it, leaving string pointing to next item.
 *
 ************************************************************************/
static XrmQuark stringToQuark(string)
     char **string;
{
  char *ptr, tmp;
  XrmQuark q;

  ptr = *string;
  while (isalnum(*ptr)) ptr++;
  if (ptr == *string)
    return (XrmQuark) NULL;
  tmp = *ptr;
  *ptr = '\0';
  q = XrmStringToQuark(*string);
  *ptr = tmp;

  if (*ptr == ',') ptr++;
  while (isspace(*ptr)) ptr++;
  *string = ptr;
  return q;
}


/************************************************************************
 *
 *  skippast  --  skip past all text until first occurrence of 'what',
 *	except if 'inquotes' is true, then just find the end of the quote.
 *
 ************************************************************************/
static void skippast(ptr, inquotes, what)
     char **ptr;
     int inquotes;
     char what;
{
  while (1)
    {
      if (inquotes)
	{
	  while (**ptr != '\0' && **ptr != '"')
	    (*ptr)++;
	  if (**ptr == '\0')
	    break;
	  (*ptr)++;
	  inquotes = 0;
	}
      else
	{
	  while (**ptr != '\0' && **ptr != '"' && **ptr != what)
	    (*ptr)++;
	  if (**ptr == '\0')
	    break;
	  if (**ptr == what)
	    {
	      (*ptr)++;
	      break;
	    }
	  (*ptr)++;
	  inquotes = 1;
	}
    }
}


/************************************************************************
 *
 *  countMenuEntries  --  count the number of menu entries in a string.
 *
 ************************************************************************/
static int countMenuEntries(me, string)
     MenuJet me;
     char *string;
{
  int count = 0;

  while (*string != '\0')
    {
      if ((!strncmp(string, "item", 4)) ||
	  (!strncmp(string, "menu", 4)) ||
	  (!strncmp(string, "title", 5)) ||
	  (!strncmp(string, "separator", 9)))
	count++;
      skippast(&string, False, ';');
      while (isspace(*string)) string++;
    }

  return count;
}



/************************************************************************
 *
 *  parseMenuEntry  --  parse entry in 'string' and shove it into 'info'.
 *
 ************************************************************************/
/*
 * Some of this code is *sooooo* gross
 */
static Boolean parseMenuEntry(me, string, info)
     MenuJet me;
     char **string;
     Item *info;
{
  int qnum, done, got_one;
  int inquotes = 0;
  char *ptr, *end, *beginning;
  Item *lookup;
  Boolean unfinished = True;

  beginning = ptr = *string;	/* remember beginning for better diagnostics */

  /*
   * Zeroes flags for us, too.
   */
  memset(info, 0, sizeof(Item));

  /*
   * Parse menu/item field
   */
  if (*ptr == '!')
    goto skipit;

  if (!strncmp(ptr, "item", 4))
    {
      info->type = ItemITEM;
      ptr += 5;
    }
  else if (!strncmp(ptr, "menu", 4))
    {
      info->type = MenuITEM;
      ptr += 5;
    }
  else if (!strncmp(ptr, "help", 4))
    {
      info->type = HelpITEM;
      ptr += 5;
    }
  else if (!strncmp(ptr, "title", 5))
    {
      info->type = TitleITEM;
      ptr += 6;
    }
  else if (!strncmp(ptr, "separator", 9))
    {
      info->type = SeparatorITEM;
      ptr += 10;
    }
  else
    {
      fprintf(stderr,
	      "menu line -\n %.70s\n- %s%s",
	      beginning,
	      "does not begin with menu, item, title, separator, ",
	      "or help; ignoring");
      goto youlose;
    }

  while (isspace(*ptr)) ptr++;

  /*
   * Parse label field
   */
  if (0 == (end = strchr(ptr, ':')))
    {
      fprintf(stderr,
	      "menu line -\n %.70s\n- %s%s",
	      beginning,
	      "does not contain ':' terminated label; ignoring\n");
      goto youlose;
    }
  *end = '\0';
  info->name = XrmStringToQuark(ptr);
  *end = ':';
  ptr = end + 1;

  /* The rest may come in any order */
  while (unfinished)
    {
      while (isspace(*ptr)) ptr++;
      switch(*ptr)
	{
 /*
  *  Parse machtype or menu orientation
  */
	case '+': /* should do this right someday... */
	case '-':
	  if (info->type == ItemITEM)
	    {
	      /*
	       * Machtype flags
	       */
	      if (strncasecmp(ptr + 1, VAX, strlen(VAX)) == 0)
		{
		  info->u.i.machtype |= (VAXNUM << ((*ptr == '+') ? 0 : 8));
		  ptr += strlen(VAX) + 1;
		  break;
		}

	      if (strncasecmp(ptr + 1, RT, strlen(RT)) == 0)
		{
		  info->u.i.machtype |= (RTNUM << ((*ptr == '+') ? 0 : 8));
		  ptr += strlen(RT) + 1;
		  break;
		}

	      if (strncasecmp(ptr + 1, DECMIPS, strlen(DECMIPS)) == 0)
		{
		  info->u.i.machtype |=
		    (DECMIPSNUM << ((*ptr == '+') ? 0 : 8));
		  ptr += strlen(DECMIPS) + 1;
		  break;
		}

	      if (strncasecmp(ptr + 1, PS2, strlen(PS2)) == 0)
		{
		  info->u.i.machtype |= (PS2NUM << ((*ptr == '+') ? 0 : 8));
		  ptr += strlen(PS2) + 1;
		  break;
		}

	      if (strncasecmp(ptr + 1, RSAIX, strlen(RSAIX)) == 0)
		{
		  info->u.i.machtype |= (RSAIXNUM << ((*ptr == '+') ? 0 : 8));
		  ptr += strlen(RSAIX) + 1;
		  break;
		}

	      if (strncasecmp(ptr + 1, SUN4, strlen(SUN4)) == 0)
		{
		  info->u.i.machtype |= (SUN4NUM << ((*ptr == '+') ? 0 : 8));
		  ptr += strlen(SUN4) + 1;
		  break;
		}

	      if (strncasecmp(ptr + 1, "verify", 6) == 0)
		{
		  info->flags |= verifyFLAG;
		  info->u.i.verify = ((*ptr) == '+');
		  ptr += 6 + 1;
		  break;
		}
	    }
	  /*
	   * Menu orientation flags
	   */
	  if ((info->type == MenuITEM) &&
	      (*(ptr + 1) == 'h'))
	    {
	      info->u.m.orientation = HORIZONTAL;
	      info->flags |= orientationFLAG;
	      ptr += 2;
	      break;
	    }

	  if ((info->type == MenuITEM) && 
	      (*(ptr + 1) == 'v'))
	    {
	      info->u.m.orientation = VERTICAL;
	      info->flags |= orientationFLAG;
	      ptr += 2;
	      break;
	    }

	  fprintf(stderr, "'%s' contains unknown flag; ignored\n",
		  XrmQuarkToString(info->name));

	  while (*ptr != '\0' &&
		 !isspace(*ptr) &&
		 *ptr != ';') ptr++;
	  break;

 /*
  *  Parse title field
  */
	case '"':
	  ptr++;

				/* Deal with star BS... */
	  if (ptr[0] == ' '  &&  ptr[1] == ' '  &&  ptr[2] == ' ')
	    ptr += 3;
	  if (ptr[0] == '*'  &&  ptr[1] == ' ')
	    ptr += 2;
				/* Rip this code out in a future */
				/* release... */

	  inquotes = 1;
	  if (0 == (end = strchr(ptr, '"')))
	    {
	      if (info->type == HelpITEM)
		{
		  fprintf(stderr,
			  "'%s' help does not have closing quote; ignoring\n",
			  XrmQuarkToString(info->name));
		}
	      else
		{
		  fprintf(stderr,
			  "'%s' menu definition title does not have closing quote; ignoring\n",
			  XrmQuarkToString(info->name));
		}
	      goto youlose;
	    }
	  *end = '\0';
	  info->title = ptr;
	  ptr = end + 1;
	  inquotes = 0;
	  info->flags |= titleFLAG;

	  if (info->type == HelpITEM)
	    {
	      /*
	       * This help is to be appended to an
	       * existing menu item
	       */
	      lookup = (Item *)hash_lookup(me->menu.Names, info->name);
	      if (lookup == NULL)
		{
		  fprintf(stderr,
			  "'%s' help entry references nonexistent item\n",
			  XrmQuarkToString(info->name));
		  goto youlose;
		}
	      if (lookup->type == MenuITEM)
		{
		  fprintf(stderr,
			  "'%s' help entry cannot be added to a menu\n",
			  XrmQuarkToString(info->name));
		  goto youlose;
		}
	      lookup->u.i.help = info->title;
	      goto skipit;
	    }
	  break;

 /*
  *  Parse (possibly) child field
  */
	case '{':
	  ptr++;
	  if (info->type == MenuITEM)
	    {
	      qnum = 0; done = 0;
	      while ((qnum < MAXCHILDREN) && !done)
		{
		  info->u.m.children[qnum++] = stringToQuark(&ptr);
		  if (info->u.m.children[qnum - 1] == (XrmQuark) NULL)
		    done = 1;
		}
	      if (!done)
		{
		  if ((XrmQuark) NULL != stringToQuark(&ptr))
		    {
		      while (stringToQuark(&ptr) != (XrmQuark) NULL);
		      fprintf(stderr,
			      "'%s' - more than %d child types; truncated\n",
			      XrmQuarkToString(info->name), MAXCHILDREN);
		    }
		}
	      if (info->u.m.children[0] != (XrmQuark) NULL)
		info->flags |= childrenFLAG;
	    }
	  else /* type != MenuITEM */
	    {
	      fprintf(stderr, "'%s' non-menu specifies children\n",
		      XrmQuarkToString(info->name));
	    }

	  if (0 == (end = strchr(ptr, '}')))
	    {
	      fprintf(stderr,
		  "%s: child specifier does not have '}'; trying to be smart\n",
		      XrmQuarkToString(info->name));
	    }
	  else
	    ptr = end + 1;
	  break;

 /*
  *  Parse parent field
  *  qnum always points to an empty slot...
  */
	case '[':
	  qnum = 0; done = 0; got_one = 0;
	  while (!done)
	    {
	      if (*ptr != '[')
		{
		  done = 1;
		  if (!got_one)
		    {
		      fprintf(stderr, 
			      "'%s' missing '[' in menu definition; ignoring\n",
			      XrmQuarkToString(info->name));
		      goto youlose;
		    }
		  else
		    info->weight[qnum - 1] = 0; /* true termination */
		}
	      else
		{
		  ptr++;

		  while ((qnum < MAXPARENTS) && !done)
		    {
		      info->parents[qnum++] = stringToQuark(&ptr);
		      if (info->parents[qnum - 1] == (XrmQuark) NULL)
			done = 1;
		      else
			{
			  /* if weight not specified, use 0 */
			  if (*ptr == '/')
			    {
			      ptr++;
			      info->weight[qnum - 1] = atoi(ptr);
			      while (isdigit(*ptr) || *ptr == '-')
				ptr++;
			      while (isspace(*ptr)) ptr++;
			    }
			  else
			    info->weight[qnum - 1] = 0;

			  if (*ptr == ',') ptr++;
			}
		    }

		  if (!done) /* out of space... */
		    {
		      if (isalnum(*ptr)) /* we weren't done yet... */
			{
			  fprintf(stderr,
				  "'%s' more than %d parent types; truncated\n",
				  XrmQuarkToString(info->name), MAXCHILDREN);
			  if (0 == (end = strchr(ptr, ']')))
			    {
			      fprintf(stderr,
			      "and mismatched brackets, too! ignoring\n");
			      goto youlose;
			    }
			  done = 1;

			  if (0 != (end = strchr(ptr, ';')))
			    ptr = end + 1;
			  else
			    {
			      fprintf(stderr,
				      "'%s' missing semicolon; ignoring\n",
				      XrmQuarkToString(info->name));
			      goto youlose;
			    }
			}
		    }
		  else /* we finished the group; do next */
		    {
		      done = 0; got_one = 1;
		      info->parents[qnum - 1] = (XrmQuark) NULL;
		      info->weight[qnum - 1] = 1; /* short termination */
		      if (*ptr == ']') ptr++;
		      else
			{
			  fprintf(stderr, "'%s' missing ']'; ignoring\n",
				  XrmQuarkToString(info->name));
			  goto youlose;
			}
		    }
		}
	    }

/*
	  if (info->parents[0] != NULL)
*/
	    info->flags |= parentsFLAG;
	  break;

 /*
  *  End of line?
  */
	case ';':
	  ptr++;
	  unfinished = False;
	  break;

 /*
  *  Default case
  */
	default:
	  if (info->type == ItemITEM)
	    {
	      char *ret;

	      if (NULL != (ret = ConvertStringToCallback(&ptr)))
		info->u.i.activateString = ret;
	      else
		{
		  fprintf(stderr,
			  "'%s' - couldn't grok callback; ignoring entry\n",
			  XrmQuarkToString(info->name));
		  goto youlose;
		}

	      *(ptr-1) = '\0';
	      info->flags |= activateFLAG;
	    }
	  else
	    {
	      fprintf(stderr, "'%s' - garbage at end of line; ignoring entry\n",
		      XrmQuarkToString(info->name));
	      goto youlose;
	    }
	  break;
	} /* switch */
    } /* while */

  while (isspace(*ptr)) ptr++;
  *string = ptr;

/*
  fprintf(stdout, "%s: ", XrmQuarkToString(info->name));

  if (info->flags & titleFLAG)
    fprintf(stdout, "title ");

  if (info->flags & activateFLAG)
    fprintf(stdout, "callback ");

  if (info->flags & orientationFLAG)
    fprintf(stdout, "orientation ");

  if (info->flags & parentsFLAG)
    fprintf(stdout, "parents ");

  if (info->flags & childrenFLAG)
    fprintf(stdout, "children");

  fprintf(stdout, "\n");
*/

  return True;

 youlose:
 skipit:
  skippast(&ptr, inquotes, ';');
  while (isspace(*ptr)) ptr++;
  *string = ptr;
  return False;
}



/************************************************************************
 *
 *  addMenuEntry  --  add a menu entry to the menu struct.
 *
 ************************************************************************/
static Boolean addMenuEntry(me, info, i)
     MenuJet me;
     Item *info, *i; /* location to put it if new */
{
  Item *j;
  TypeDef *t;
  int m, n;

  j = (Item *)hash_lookup(me->menu.Names, info->name);
  if (j != NULL)
    i = j;

  if (j != NULL &&
      j->type != info->type)
    {

      fprintf(stderr, "'%s' - type %s can't override type %s\n",
	      XrmQuarkToString(info->name),
	      &"item\000menu"[info->type*5],
	      &"item\000menu"[j->type*5]);
      return False;
    }

  /*
   * Register the item's name and zero its structure if not an override
   */
  if (j == NULL)
    {
      (void)hash_store(me->menu.Names, info->name, i);
      memset(i, 0, sizeof(Item)); /* if new, init to zeroes */
      i->u.i.machtype =
	VAXNUM | RTNUM | DECMIPSNUM | PS2NUM | RSAIXNUM	| SUN4NUM;
      i->u.i.verify = True;
    }

  /*
   * Put into list if not override...
   */
  if (j == NULL)
    switch(info->type)
      {
      case ItemITEM:
	i->next = me->menu.firstItem;
	me->menu.firstItem = i;
	break;
      case MenuITEM:
	i->next = me->menu.firstMenu;
	me->menu.firstMenu = i;
	break;
      }

  /*
   * Now merge in information
   */
  i->name = info->name;
  i->type = info->type;
  i->flags |= info->flags; /* hey, what the heck! */

  if (info->flags & titleFLAG)
    i->title = info->title;

  if (info->flags & activateFLAG)
    i->u.i.activateString = info->u.i.activateString;

  if (info->flags & orientationFLAG)
    i->u.m.orientation = info->u.m.orientation;

  if (info->flags & verifyFLAG)
    i->u.i.verify = info->u.i.verify;

  i->u.i.machtype &= ~((info->u.i.machtype >> 8) & 255);
  i->u.i.machtype |= (info->u.i.machtype & 255);

  if (info->flags & parentsFLAG)
    {
      memcpy(i->parents, info->parents, MAXPARENTS * sizeof(XrmQuark));
      memcpy(i->weight, info->weight, MAXPARENTS * sizeof(int));
    }

  if (info->flags & childrenFLAG)
    {
      /*
       * First, remove all the old ones. Then add all the new ones.
       * There may be wasted work here, but at least the computer
       * will be doing it and not me.
       */
      for (n = 0; i->u.m.children[n] != 0 && n < MAXCHILDREN; n++)
	{
	  t = (TypeDef *)hash_lookup(me->menu.Types, i->u.m.children[n]);
	  if (t != NULL) /* better not be NULL! */
	    {
	      for (m = 0;
		   m < MAXMENUSPERTYPE &&
		     t->menus[m] != 0 && /* shouldn't happen */
		     t->menus[m] != i;
		   m++);
	      if (m < MAXMENUSPERTYPE && t->menus[m] == i)
		{
		  for (; m < MAXMENUSPERTYPE - 1; m++)
		    t->menus[m] = t->menus[m + 1];
		  t->menus[m] = 0;
		}
	    }
	}

      memcpy(i->u.m.children,
	    info->u.m.children,
	    MAXCHILDREN * sizeof(XrmQuark));

      /*
       * Add the new ones...
       */
      for (n = 0; i->u.m.children[n] != 0 && n < MAXCHILDREN; n++)
	{
	  t = (TypeDef *)hash_lookup(me->menu.Types, i->u.m.children[n]);
	  if (t == NULL)
	    {
	      t = (TypeDef *)XjMalloc(sizeof(TypeDef));
	      memset(t, 0, sizeof(TypeDef));
	      (void)hash_store(me->menu.Types, i->u.m.children[n], t);
	      t->type = i->u.m.children[n];
	      t->menus[0] = i;
	    }
	  else
	    {
	      for (m = 0; m < MAXMENUSPERTYPE && t->menus[m] != 0; m++);
	      if (m == MAXMENUSPERTYPE)
		{

		  fprintf(stderr, "'%s' not typed as %s due to overflow\n",
			  XrmQuarkToString(i->name),
			  XrmQuarkToString(t->type));
		}
	      else
		t->menus[m] = i;
	    }
	}
    }

  if (j == NULL)
    return True;
  else
    return False;
}



/************************************************************************
 *
 *  printTable  --  prints out the menu table - for debugging...
 *
 ************************************************************************/
static void printTable(fs, t)
     XFontStruct *fs;
     Item *t;
{
  int i, done;

  while (t != NULL)
    {
      fprintf(stdout, "%d %04.4d %s %05.5d %05.5d %d %04.4d %s %05.5d\n",
	      t->type,
	      strlen(XrmQuarkToString(t->name)),
	      XrmQuarkToString(t->name),
	      t->flags,
	      (t->type == MenuITEM) ? t->u.m.orientation : t->u.i.machtype,
	      (t->type == ItemITEM) ? t->u.i.verify : 0,
	      strlen(t->title),
	      t->title,
	      XTextWidth(fs,
			 t->title,
			 strlen(t->title)));

      if (t->type == MenuITEM)
	{
	  for (i = 0; i < MAXCHILDREN && t->u.m.children[i] != 0; i++)
	    {
	      fprintf(stdout, "%04.4d %s ",
		      strlen(XrmQuarkToString(t->u.m.children[i])),
		      XrmQuarkToString(t->u.m.children[i]));
/*	      if ((i < MAXCHILDREN - 1) && t->u.m.children[i+1] != 0)
		fprintf(stdout, " ");
*/
	    }
	  fprintf(stdout, "\n");
	}


      done = 0; i = 0;
      while (!done)
	{
	  if (t->parents[i] == 0)
	    done = 1;
	  while (!done)
	    {
	      if (t->parents[i] != 0)
		{
		  fprintf(stdout, "%s", XrmQuarkToString(t->parents[i]));
		  fprintf(stdout, " %04.4d ", t->weight[i]);
		  if ((i < MAXPARENTS - 1) && t->parents[i+1] == 0)
/*		    fprintf(stdout, " ");
		  else */
		    {
		      done = 1;
		      fprintf(stdout, "\n");
		    }
		  i++;
		}
	    }
	  if ((i < MAXPARENTS - 1) &&
	      t->weight[i] == 0)
	    done = 1;
	  else
	    {
	      i++;
	      done = 0;
	    }
	}
      fprintf(stdout, "!\n");

#define stringlen(s) ((s) == NULL) ? 6 : strlen((s))

      if (t->type == ItemITEM)
	{
	  int width, height;
	  char *ptr, *end;

	  printf("%04.4d %s\n", stringlen(t->u.i.activateString),
		 ((t->u.i.activateString == NULL)
		  ? "(null)" : t->u.i.activateString));
	  printf("%04.4d %s\n", stringlen(t->u.i.help),
		 ((t->u.i.help == NULL)
		  ? "(null)" : t->u.i.help));

	  width = 0;
	  height = 0;

	  if (t->u.i.help != NULL)
	    {
	      ptr = t->u.i.help;
	      while (*ptr != '\0')
		{
		  end = ptr;
		  while (*end != '\n' && *end != '\0')
		    end++;

		  if (end > ptr)
		    width = MAX(width,
				XTextWidth(fs,
					   ptr,
					   (int)(end - ptr)));

		  height +=  fs->ascent + fs->descent;

		  if (*end != '\0')
		    ptr = end + 1;
		  else
		    ptr = end;
		}
	    }

	  printf("! %05.5d %05.5d\n", width, height);
	}


      t = t->next;
    }
}


/************************************************************************
 *
 *  createTablesFromString  --  create menu tables from a menu string.
 *
 ************************************************************************/
static void createTablesFromString(me, string)
     MenuJet me;
     char *string;
{
  int counter = 0;
  char *ptr;
  Item info, *location;
  int num;

  ptr = string;
  num = countMenuEntries(me, ptr);
  printf("%04.4d\n", num);
  location = (Item *)XjMalloc(num * sizeof(Item));

  printf("%03.3d %s\n", strlen(FONTNAME), FONTNAME);
  while (*ptr != '\0')
    {
      if (parseMenuEntry(me, &ptr, &info))
	{
	  if (addMenuEntry(me, &info, location))
	    location++;
	  counter++;
	}
    }
  if (num != counter)
  fprintf(stderr, "number of entries = %d,  but only %d compiled.\n",
	  num, counter);
}



/************************************************************************
 *
 *  loadFile  --  read in a file and return it as one big ol' string.
 *
 ************************************************************************/
static char *loadFile(filename)
     char *filename;
{
    int fd, size;
    struct stat buf;
    char *info;

    if (-1 == (fd = open(filename, O_RDONLY, 0)))
      {
	if (errno == 0 || errno > sys_nerr)
	  fprintf(stderr, "loading file - error %d opening '%s'\n",
		  errno, filename);
	else
	  fprintf(stderr, "opening '%s': %s\n", filename, sys_errlist[errno]);

	return NULL;
      }

    if (-1 == fstat(fd, &buf)) /* could only return EIO */
      {
	if (errno == 0 || errno > sys_nerr)
	  fprintf(stderr, "loading file - error %d in fstat for '%s'\n",
		  errno, filename);
	else
	  fprintf(stderr, "fstat '%s': %s\n", filename, sys_errlist[errno]);

	close(fd);
	return NULL;
      }

    size = (int)buf.st_size;

    info = (char *)XjMalloc(size + 1);	/* one extra for the NULL */

    if (-1 == read(fd, info, size))
      {
	if (errno == 0 || errno > sys_nerr)
	  fprintf(stderr, "loading file - error %d reading '%s'\n",
		  errno, filename);
	else
	  fprintf(stderr, "reading '%s': %s\n", filename, sys_errlist[errno]);

	close(fd);
	XjFree(info);
	return NULL;
      }
    close(fd);

    info[size] = '\0';
    return info;
}



/************************************************************************
 *
 *  main  --  
 *
 ************************************************************************/
void main(argc, argv)
     int argc;
     char **argv;
{
  Jet root;
  MenuJet me;
  XFontStruct *fs;
  Display *display;

  root = XjCreateRoot(&argc, argv, "Dash", NULL,
		      NULL, 0);
  me = (MenuJet) XjVaCreateJet("menubar", menuJetClass, root, NULL, NULL);

  me->menu.Names = create_hash(HASHSIZE);
  me->menu.Types = create_hash(HASHSIZE);
  me->menu.firstItem = NULL;
  me->menu.firstMenu = NULL;

  /*
   * load up default menus...
   */
  
  me->menu.items = loadFile(argv[1]);

  if (me->menu.items == NULL)
    XjExit(-1);

  display = root->core.display;
  fs = XLoadQueryFont(display, FONTNAME);

  createTablesFromString(me, me->menu.items);

  printTable(fs, me->menu.firstMenu);
  printTable(fs, me->menu.firstItem);

  XjExit(0);
}
