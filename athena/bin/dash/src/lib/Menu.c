/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Menu.c,v $
 * $Author: cfields $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char *rcsid =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Menu.c,v 1.9 1995-05-26 04:12:52 cfields Exp $";
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
#include "Jets.h"
#include "Window.h"
#include "Menu.h"
#include "Button.h"
#include "warn.h"
#include <X11/Xatom.h>

#include <sys/time.h>

#if  defined(NEED_ERRNO_DEFS)
extern int errno;
extern char *sys_errlist[];
extern int sys_nerr;
#endif

extern int DEBUG;

/* #define DEBUGPARSING */

/* #define DEBUGMEM */

extern XrmDatabase rdb;

/* oops! broken abstraction! but it's just a string. :-) */
#define NOMENU "\
menu deadroot: \"empty file\" {none} [NULL];\
item quit: \"Quit Dash\" [none] -verify quit();"

/* come to think of it, should this be a class variable? */
static XContext context;
static int gotContextType = 0;
int me_moving = 0;

#define offset(field) XjOffset(MenuJet,field)

static XjResource resources[] = {
  { XjNfont, XjCFont, XjRFontStruct, sizeof(XFontStruct *),
     offset(menu.font), XjRString, XjDefaultFont },
  { XjNx, XjCX, XjRInt, sizeof(int),
      offset(core.x), XjRString, XjInheritValue },
  { XjNy, XjCY, XjRInt, sizeof(int),
      offset(core.y), XjRString, XjInheritValue },
  { XjNwidth, XjCWidth, XjRInt, sizeof(int),
      offset(core.width), XjRString, XjInheritValue },
  { XjNheight, XjCHeight, XjRInt, sizeof(int),
      offset(core.height), XjRString, XjInheritValue },
  { XjNhMenuPadding, XjCMenuPadding, XjRInt, sizeof(int),
      offset(menu.hMenuPadding), XjRString, "10" },
  { XjNvMenuPadding, XjCMenuPadding, XjRInt, sizeof(int),
      offset(menu.vMenuPadding), XjRString, "4" },
  { XjNitems, XjCItems, XjRString, sizeof(char *),
      offset(menu.items), XjRString, "" },
  { XjNfile, XjCFile, XjRString, sizeof(char *),
      offset(menu.file), XjRString, "" },
  { XjNfallback, XjCFallback, XjRString, sizeof(char *),
      offset(menu.fallback), XjRString, "" },
  { XjNuserItems, XjCItems, XjRString, sizeof(char *),
      offset(menu.userItems), XjRString, "" },
  { XjNuserFile, XjCFile, XjRString, sizeof(char *),
      offset(menu.userFile), XjRString, "" },
  { XjNforeground, XjCForeground, XjRColor, sizeof(int),
      offset(menu.foreground), XjRString, XjDefaultForeground },
  { XjNbackground, XjCBackground, XjRColor, sizeof(int),
      offset(menu.background), XjRString, XjDefaultBackground },
  { XjNreverseVideo, XjCReverseVideo, XjRBoolean, sizeof(Boolean),
      offset(menu.reverseVideo), XjRBoolean, (caddr_t)False },
  { XjNscreenWidth, XjCScreenWidth, XjRBoolean, sizeof(Boolean),
      offset(menu.screenWidth), XjRBoolean, (caddr_t) False },
  { XjNhelpPixmap, XjCPixmap, XjRPixmap, sizeof(XjPixmap *),
      offset(menu.helpPixmap), XjRString, "help.bits" },
  { XjNsubmenuPixmap, XjCPixmap, XjRPixmap, sizeof(XjPixmap *),
      offset(menu.submenuPixmap), XjRString, "submenu.bits" },
  { XjNshowHelp, XjCShowHelp, XjRBoolean, sizeof(Boolean),
      offset(menu.showHelp), XjRBoolean, (caddr_t)True },
  { XjNgrey, XjCGrey, XjRPixmap, sizeof(XjPixmap *),
      offset(menu.grey), XjRString, NULL },
  { XjNrude, XjCRude, XjRBoolean, sizeof(Boolean),
      offset(menu.rude), XjRBoolean, (caddr_t)True },
  { XjNverifyProc, XjCVerifyProc, XjRCallback, sizeof(XjCallback *),
      offset(menu.verifyProc), XjRString, NULL },
  { XjNverify, XjCVerify, XjRBoolean, sizeof(Boolean),
      offset(menu.verify), XjRBoolean, (caddr_t)True },
  { XjNautoRaise, XjCAutoRaise, XjRBoolean, sizeof(Boolean),
      offset(menu.autoRaise), XjRBoolean, (caddr_t)False },
  { XjNmoveMenus, XjCMoveMenus, XjRBoolean, sizeof(Boolean),
      offset(menu.moveMenus), XjRBoolean, (caddr_t)False },
  { XjNrightJet, XjCJet, XjRString, sizeof(Jet),
      offset(menu.rightJet), XjRString, NULL },
};

#undef offset

#define BORDER 1
#define SHADOW 1
#define SAME (Menu *)-1

static void initialize(), realize();
static Boolean event_handler();
void computeMenuSize();
static void closeMenuAndAncestorsToLevel();

MenuClassRec menuClassRec = {
  {
    /* class name */		"Menu",
    /* jet size   */		sizeof(MenuRec),
    /* classInitialize */	NULL,
    /* classInitialized? */	1,
    /* initialize */		initialize,
    /* prerealize */    	NULL,
    /* realize */		realize,
    /* event */			event_handler,
    /* expose */		NULL,
    /* querySize */     	NULL,
    /* move */			NULL,
    /* resize */        	NULL,
    /* destroy */       	NULL,
    /* resources */		resources,
    /* number of 'em */		XjNumber(resources)
  }
};

JetClass menuJetClass = (JetClass)&menuClassRec;


/************************************************************************
 *
 *  drawHelp  -- draws text in a help box when it is popped up.
 *
 ************************************************************************/
static void drawHelp(me, menu)
     MenuJet me;
     Menu *menu;
{
  char *ptr, *end;
  int y = 0;

  ptr = menu->child->title;
  while (*ptr != '\0')
    {
      end = ptr;
      while (*end != '\n' && *end != '\0')
	end++;

      if (end > ptr)
	XDrawString(me->core.display, menu->menuPane,
		    me->menu.gc,
		    me->menu.hMenuPadding / 2,
		    y + me->menu.vMenuPadding / 2 +
		    	me->menu.font->ascent,
		    ptr, (int)(end - ptr));

      y += me->menu.font->ascent + me->menu.font->descent;

      if (*end != '\0')
	ptr = end + 1;
      else
	ptr = end;
    }
}


/************************************************************************
 *
 *  computeHelpSize  -- figures out how big a help box is, based on the
 *	text that it contains.
 *
 ************************************************************************/
static void computeHelpSize(me, menu)
     MenuJet me;
     Menu *menu;
{
  char *ptr, *end;

  menu->pane_width = 0;
  menu->pane_height = me->menu.vMenuPadding;

  if (menu->child->title_width != 0)
    {
      menu->pane_width += me->menu.hMenuPadding + menu->child->title_width;
      menu->pane_height += menu->child->title_height;
    }
  else
    {
      ptr = menu->child->title;
      while (*ptr != '\0')
	{
	  end = ptr;
	  while (*end != '\n' && *end != '\0')
	    end++;

	  if (end > ptr)
	    menu->pane_width = MAX(menu->pane_width,
				   me->menu.hMenuPadding +
				   XTextWidth(me->menu.font,
					      ptr,
					      (int)(end - ptr)));

	  menu->pane_height +=  me->menu.font->ascent +
				me->menu.font->descent;

	  if (*end != '\0')
	    ptr = end + 1;
	  else
	    ptr = end;
	}
    }

  menu->child->label_x = menu->child->label_y = BORDER;
  menu->child->label_width = menu->pane_width;
  menu->child->label_height = menu->pane_height;

  menu->pane_width += 2 * BORDER + SHADOW;
  menu->pane_height += 2 * BORDER + SHADOW;
}


/************************************************************************
 *
 *  computeMenuSize  -- figures out how big a menu is, based on the
 *	items that it contains.
 *
 ************************************************************************/
void computeMenuSize(me, menu)
     MenuJet me;
     Menu *menu;
{
  int x, y;
  static int height=0;
  int maxPixmap = 0;
  Menu *m;

  if (height==0)
    height = me->menu.font->ascent + me->menu.font->descent
      + me->menu.vMenuPadding;

  if (menu->parent && menu->parent->paneType == HELP)
    {
      computeHelpSize(me, menu->parent);
      return;
    }

  if (menu->title_width == 0)
    {
      menu->title_width = XTextWidth(me->menu.font,
				     menu->title,
				     strlen(menu->title));
    }
  menu->label_width = me->menu.hMenuPadding + menu->title_width;
  menu->label_height = height;


  x = y = BORDER;

  if (menu->orientation == VERTICAL)
    x += 10;

  menu->pane_width = 0;
  menu->pane_height = 0;

  for (m = menu->child; m != NULL; m = m->sibling)
    {
      m->label_x = x;
      m->label_y = y;

      switch(menu->orientation)
	{
	case HORIZONTAL:
	  menu->pane_width += m->label_width;
	  if (m->label_height > menu->pane_height)
	    menu->pane_height = m->label_height;
	  x += m->label_width;
	  break;

	case VERTICAL:
	  if (m->child != NULL)
	    {
	      if (m->paneType != HELP)
		{
		  int tmp;
		  tmp = me->menu.submenuPixmap->width + me->menu.hMenuPadding;
		  if (tmp > maxPixmap)
		    maxPixmap = tmp;

		  tmp = me->menu.submenuPixmap->height + me->menu.vMenuPadding;
		  if (tmp > m->label_height)
		    m->label_height = tmp;
		}
	      else
		if (me->menu.showHelp)
		  {
		    int tmp;
		    tmp = me->menu.helpPixmap->width + me->menu.hMenuPadding;
		    if (tmp > maxPixmap)
		      maxPixmap = tmp;

		    tmp = me->menu.helpPixmap->height + me->menu.vMenuPadding;
		    if (tmp > m->label_height)
		      m->label_height = tmp;
		  }
	    }
	      
	  if (m->label_width > menu->pane_width)
	    menu->pane_width = m->label_width;
/*
	  menu->pane_width = MAX(menu->pane_width,
				 m->label_width + m->label_x);
*/
	  menu->pane_height += m->label_height;
	  y += m->label_height;

	  break;
	}
    }

  if (menu->orientation == VERTICAL)
    menu->pane_width += x;

  menu->pane_open_x = menu->pane_width + BORDER;

  menu->pane_width += 2 * BORDER + SHADOW + maxPixmap;
  menu->pane_height += 2 * BORDER + SHADOW;
}


/************************************************************************
 *
 *  computeAllMenuSizes  --
 *	Iteratively traverse the menu tree (bottom up) and
 *	recompute it. This function is usefully called
 *	when help mode is toggled.
 *
 ************************************************************************/
void computeAllMenuSizes(me, menu)
     MenuJet me;
     Menu *menu;
{
  Menu *m;

  struct timeval start, end;

  if (DEBUG)
    {
      gettimeofday(&start, NULL);
      printf("computeAllMenuSizes: - %d.%d + \n", start.tv_sec, start.tv_usec);
    }

  m = menu;

  while (m != NULL)
    {
      while (m->child != NULL)
	m = m->child;

      computeMenuSize(me, m);

      while (m != NULL && m->sibling == NULL)
	{
	  m = m->parent;
	  if (m != NULL)
	    computeMenuSize(me, m);
	}

      if (m != NULL /* && m->sibling != NULL */)
	m = m->sibling;
    }	  
  if (DEBUG)
    {
      gettimeofday(&end, NULL);
      printf("computeAllMenuSizes: %d.%d = %d.%06.6d\n",
	     end.tv_sec, end.tv_usec,
	     (end.tv_usec > start.tv_usec)
	     ? end.tv_sec - start.tv_sec
	     : end.tv_sec - start.tv_sec - 1,
	     (end.tv_usec > start.tv_usec)
	     ? end.tv_usec - start.tv_usec
	     : end.tv_usec + 1000000 - start.tv_usec );
    }
}


/************************************************************************
 *
 *  computeRootMenuSize  --  figure out how big the root menu is.
 *
 ************************************************************************/
void computeRootMenuSize(me, size)
     MenuJet me;
     XjSize *size;
{
  computeMenuSize(me, me->menu.rootMenu);

  size->width  = me->menu.rootMenu->pane_width;
  size->height = me->menu.rootMenu->pane_height;

  if (me->menu.screenWidth == 1 &&
      me->menu.rootMenu->orientation == HORIZONTAL)
    size->width = MAX(DisplayWidth(me->core.display,
				   DefaultScreen(me->core.display)),
		      me->menu.rootMenu->pane_width);

#ifdef notdefined
  me->core.width = me->menu.rootMenu->pane_width;
  me->core.height = me->menu.rootMenu->pane_height;

  if (me->menu.screenWidth == 1 &&
      me->menu.rootMenu->orientation == HORIZONTAL)
    me->core.width = MAX(DisplayWidth(me->core.display,
				      DefaultScreen(me->core.display)),
			 me->menu.rootMenu->pane_width);
#endif
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
#ifdef DEBUGPARSING
  fprintf(stdout, "%s ", *string);
  fflush(stdout);
#endif
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

  struct timeval start, end;

  if (DEBUG)
    {
      gettimeofday(&start, NULL);
      printf("countMenuEntries: - %d.%d + \n", start.tv_sec, start.tv_usec);
    }

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

  if (DEBUG)
    {
      gettimeofday(&end, NULL);
      printf("countMenuEntries: %d.%d = %d.%06.6d\n", end.tv_sec, end.tv_usec,
	     (end.tv_usec > start.tv_usec)
	     ? end.tv_sec - start.tv_sec
	     : end.tv_sec - start.tv_sec - 1,
	     (end.tv_usec > start.tv_usec)
	     ? end.tv_usec - start.tv_usec
	     : end.tv_usec + 1000000 - start.tv_usec );
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
  char errtext[100];

  beginning = ptr = *string;	/* remember beginning for better diagnostics */

  /*
   * Zeroes flags for us, too.
   */
  bzero(info, sizeof(Item));

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
      sprintf(errtext,
	      "menu line -\n %.70s\n- %s%s",
	      beginning,
	      "does not begin with menu, item, title, separator, ",
	      "or help; ignoring");
      XjWarning(errtext);
      goto youlose;
    }

  while (isspace(*ptr)) ptr++;

  /*
   * Parse label field
   */
  if (0 == (end = index(ptr, ':')))
    {
      sprintf(errtext,
	      "menu line -\n %.70s\n- %s%s",
	      beginning,
	      "does not contain ':' terminated label; ignoring");
      XjWarning(errtext);
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

	      if (strncasecmp(ptr + 1, SGI, strlen(SGI)) == 0)
		{
		  info->u.i.machtype |= (SGINUM << ((*ptr == '+') ? 0 : 8));
		  ptr += strlen(SGI) + 1;
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

	  if (DEBUG)
	    {
	      sprintf(errtext, "'%s' contains unknown flag; ignored",
		      XrmQuarkToString(info->name));
	      XjWarning(errtext);
	    }
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
	  if (0 == (end = index(ptr, '"')))
	    {
	      if (info->type == HelpITEM)
		{
		  sprintf(errtext,
			  "'%s' help does not have closing quote; ignoring",
			  XrmQuarkToString(info->name));
		  XjWarning(errtext);
		}
	      else
		{
		  sprintf(errtext,
       "'%s' menu definition title does not have closing quote; ignoring",
			  XrmQuarkToString(info->name));
		  XjWarning(errtext);
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
		  sprintf(errtext,
			  "'%s' help entry references nonexistent item",
			  XrmQuarkToString(info->name));
		  XjWarning(errtext);
		  goto youlose;
		}
	      if (lookup->type == MenuITEM)
		{
		  sprintf(errtext,
			  "'%s' help entry cannot be added to a menu",
			  XrmQuarkToString(info->name));
		  XjWarning(errtext);
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
		      sprintf(errtext,
			      "'%s' - more than %d child types; truncated",
			      XrmQuarkToString(info->name), MAXCHILDREN);
		      XjWarning(errtext);
		    }
		}
	      if (info->u.m.children[0] != (XrmQuark) NULL)
		info->flags |= childrenFLAG;
	    }
	  else /* type != MenuITEM */
	    {
	      sprintf(errtext, "'%s' non-menu specifies children",
		      XrmQuarkToString(info->name));
	      XjWarning(errtext);
	    }

	  if (0 == (end = index(ptr, '}')))
	    {
	      sprintf(errtext,
		  "%s: child specifier does not have '}'; trying to be smart",
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
		      sprintf(errtext, 
			      "'%s' missing '[' in menu definition; ignoring",
			      XrmQuarkToString(info->name));
		      XjWarning(errtext);
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
			  sprintf(errtext,
				  "'%s' more than %d parent types; truncated",
				  XrmQuarkToString(info->name), MAXCHILDREN);
			  XjWarning(errtext);
			  if (0 == (end = index(ptr, ']')))
			    {
			      XjWarning(
			      "and mismatched brackets, too! ignoring");
			      goto youlose;
			    }
			  done = 1;

			  if (0 != (end = index(ptr, ';')))
			    ptr = end + 1;
			  else
			    {
			      sprintf(errtext,
				      "'%s' missing semicolon; ignoring",
				      XrmQuarkToString(info->name));
			      XjWarning(errtext);
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
			  sprintf(errtext, "'%s' missing ']'; ignoring",
				  XrmQuarkToString(info->name));
			  XjWarning(errtext);
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
	      if (NULL ==
		  (info->u.i.activateProc = XjConvertStringToCallback(&ptr)))
		{
		  sprintf(errtext,
			  "'%s' - couldn't grok callback; ignoring entry",
			  XrmQuarkToString(info->name));
		  XjWarning(errtext);
		  goto youlose;
		}

	      *(ptr-1) = '\0';
	      info->flags |= activateFLAG;
	    }
	  else
	    {
	      sprintf(errtext, "'%s' - garbage at end of line; ignoring entry",
		      XrmQuarkToString(info->name));
	      XjWarning(errtext);
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
      char errtext[100];

      sprintf(errtext, "'%s' - type %s can't override type %s",
	      XrmQuarkToString(info->name),
	      &"item\000menu"[info->type*5],
	      &"item\000menu"[j->type*5]);
      XjWarning(errtext);
      return False;
    }

  /*
   * Register the item's name and zero its structure if not an override
   */
  if (j == NULL)
    {
      (void)hash_store(me->menu.Names, info->name, i);
      bzero(i, sizeof(Item)); /* if new, init to zeroes */
      i->u.i.machtype =
	VAXNUM | RTNUM | DECMIPSNUM | PS2NUM | RSAIXNUM	| SUN4NUM | SGINUM;
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
    i->u.i.activateProc = info->u.i.activateProc;

  if (info->flags & orientationFLAG)
    i->u.m.orientation = info->u.m.orientation;

  if (info->flags & verifyFLAG)
    i->u.i.verify = info->u.i.verify;

  i->u.i.machtype &= ~((info->u.i.machtype >> 8) & 255);
  i->u.i.machtype |= (info->u.i.machtype & 255);

  if (info->flags & parentsFLAG)
    {
      bcopy(info->parents, i->parents, MAXPARENTS * sizeof(XrmQuark));
      bcopy(info->weight, i->weight, MAXPARENTS * sizeof(int));
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

      bcopy(info->u.m.children,
	    i->u.m.children,
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
	      bzero(t, sizeof(TypeDef));
	      (void)hash_store(me->menu.Types, i->u.m.children[n], t);
	      t->type = i->u.m.children[n];
	      t->menus[0] = i;
	    }
	  else
	    {
	      for (m = 0; m < MAXMENUSPERTYPE && t->menus[m] != 0; m++);
	      if (m == MAXMENUSPERTYPE)
		{
		  char errtext[100];

		  sprintf(errtext, "'%s' not typed as %s due to overflow",
			  XrmQuarkToString(i->name),
			  XrmQuarkToString(t->type));
		  XjWarning(errtext);
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



#ifdef notdefined
/************************************************************************
 *
 *  printTable  --  prints out the menu table - for debugging...
 *
 ************************************************************************/
static void printTable(t)
     Item *t;
{
  int i, done;

  while (t != NULL)
    {
      fprintf(stdout, "%d %s %d %d %d %s\n",
	      t->type,
	      XrmQuarkToString(t->name),
	      t->flags,
	      (t->type == MenuITEM) ? t->u.m.orientation : t->u.i.machtype,
	      (t->type == ItemITEM) ? t->u.i.verify : 0,
	      t->title);

      if (t->type == MenuITEM)
	{
	  for (i = 0; i < MAXCHILDREN && t->u.m.children[i] != 0; i++)
	    {
	      fprintf(stdout, "%s", XrmQuarkToString(t->u.m.children[i]));
	      if ((i < MAXCHILDREN - 1) && t->u.m.children[i+1] != 0)
		fprintf(stdout, " ");
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
		  fprintf(stdout, " %d", t->weight[i]);
		  if ((i < MAXPARENTS - 1) && t->parents[i+1] != 0)
		    fprintf(stdout, " ");
		  else
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


      if (t->type == ItemITEM)
	{
	  printf("%s\n", "activateStringHere");
	  printf("%s\n",t->u.i.help);
	  printf("!\n");
	}


      t = t->next;
    }
}
#endif


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

  struct timeval start, end;

  if (DEBUG)
    {
      gettimeofday(&start, NULL);
      printf("createTablesFromString: - %d.%d + \n", start.tv_sec, start.tv_usec);
    }

  ptr = string;
  num = countMenuEntries(me, ptr);
  location = (Item *)XjMalloc(num * sizeof(Item));

  while (*ptr != '\0')
    {
      if (parseMenuEntry(me, &ptr, &info))
	{
	  if (addMenuEntry(me, &info, location))
	    location++;
	  counter++;
	}
    }

  if (DEBUG)
    {
      gettimeofday(&end, NULL);
      printf("createTablesFromString: %d.%d = %d.%06.6d\n", end.tv_sec, end.tv_usec,
	     (end.tv_usec > start.tv_usec)
	     ? end.tv_sec - start.tv_sec
	     : end.tv_sec - start.tv_sec - 1,
	     (end.tv_usec > start.tv_usec)
	     ? end.tv_usec - start.tv_usec
	     : end.tv_usec + 1000000 - start.tv_usec );
    }
}



/************************************************************************
 *
 *  createItem  --  stick an item into the menu struct...
 *
 ************************************************************************/
static Boolean createItem(me, what)
     MenuJet me;
     Item *what;
{
  Menu *menuParent = NULL, *item, *here, **hereptr;
  Item *parent;
  TypeDef *list;
  int created = 0, qnum = 0, pnum;

  while (1) /* this loops through the list of possible parents */
    {
      /* skip past any breaks... */
      if (what->parents[qnum] == 0)
	{
	  if (what->weight[qnum] == 0) /* end of possibilities... we're done */
	    break;
	  qnum++; /* soft break... possibly create more */
	  if (qnum >= MAXPARENTS)
	    break;
	}

      /* see if there are any possible parents of this type */
      list = (TypeDef *)hash_lookup(me->menu.Types, what->parents[qnum]);
      if (list == NULL &&
	  (0 != strcmp("NULL", XrmQuarkToString(what->parents[qnum]))))
	qnum++; /* type is not NULL and is otherwise undefined */
      else
	{ /* there are possible parents */
	  if (list != NULL)
	    {
	      pnum = 0;
	      while (1) /* on exit from this loop, menuParent points to */
		{       /* parent, NULL if none */
		  if ((parent = list->menus[pnum]) == 0)
		    break; /* couldn't create *any* menus of this type */

		  if ((menuParent = parent->u.m.m) != NULL)
		    break;
		  else
		    { /* this menu doesn't exist... try to create it */
		      if (createItem(me, parent))
			{ /* yay! we created our parent! */
			  menuParent = parent->u.m.m;
			  break;
			}
		    }
		  pnum++;
		  if (pnum >= MAXMENUSPERTYPE)
		    break;
		}
	    }

	  if (menuParent == NULL && list != NULL)
	    qnum++; /* no possible parent of this type */
	  else /* menuParent == NULL && list == NULL */
	    {
	      /*
	       * We win! Create this item!
	       */
	      created++;
	      /* fprintf(stdout, "Created %s\n", what->title); */
	      item = (Menu *)XjMalloc(sizeof(Menu));
	      if (what->type == MenuITEM)
		what->u.m.m = item;
	      bzero(item, sizeof(Menu));

	      item->parent = menuParent;
	      if (menuParent == NULL)
		me->menu.rootMenu = item;
	      else
		{
		  here = menuParent->child;
		  hereptr = &(menuParent->child);

		  while (here != NULL &&
			 ((what->weight[qnum] > here->weight) ||
			  ((what->weight[qnum] == here->weight) &&
			   strcasecmp(what->title, here->title) > 0)))
		    {
		      hereptr = &(here->sibling);
		      here = here->sibling;
		    }

		  item->sibling = here;
		  *hereptr = item;
		}
	      item->state = CLOSED;
	      item->paneType = NORMAL;
	      item->title = what->title;
	      item->title_width = what->title_width;
	      item->weight = what->weight[qnum];

	      if (what->type == MenuITEM)
		{
		  item->orientation = what->u.m.orientation;
		  item->child = NULL;
		  item->machtype = ~0;
		}
	      else
		{
		  item->activateProc = what->u.i.activateProc;
		  item->machtype = what->u.i.machtype;
		  item->verify = what->u.i.verify;

		  if (NULL != what->u.i.help)
		    {
		      item->paneType = HELP;
		      item->child = (Menu *)XjMalloc(sizeof(Menu));
		      bzero(item->child, sizeof(Menu));
		      item->child->title = what->u.i.help;
		      item->child->title_width = what->u.i.help_width;
		      item->child->title_height = what->u.i.help_height;
		      item->child->parent = item;
		      item->child->state = CLOSED;
		      item->child->activateProc = item->activateProc;
		    }
		}
	      if (what->type == MenuITEM)
		return True; /* can currently create only one instance of
				a menu... just break here? */
	      while (qnum < MAXPARENTS && what->parents[qnum] != 0)
		qnum++; /* skip past other same-set candidates */
	    }
	  if (qnum >= MAXPARENTS)
	    break;
	}
    }

  if (created != 0)
    return True;
  else
    return False;
}



/************************************************************************
 *
 *  createMenuFromTables  --  build the menu struct.
 *
 ************************************************************************/
static void createMenuFromTables(me)
     MenuJet me;
{
  Item *i;

  struct timeval start, end;

  if (DEBUG)
    {
      gettimeofday(&start, NULL);
      printf("createMenuFromTables: - %d.%d + \n", start.tv_sec, start.tv_usec);
    }

  for (i = me->menu.firstItem; i != NULL; i = i->next)
    (void)createItem(me, i);

  if (DEBUG)
    {
      gettimeofday(&end, NULL);
      printf("createMenuFromTables: %d.%d = %d.%06.6d\n", end.tv_sec, end.tv_usec,
	     (end.tv_usec > start.tv_usec)
	     ? end.tv_sec - start.tv_sec
	     : end.tv_sec - start.tv_sec - 1,
	     (end.tv_usec > start.tv_usec)
	     ? end.tv_usec - start.tv_usec
	     : end.tv_usec + 1000000 - start.tv_usec );
    }
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
    char errtext[100];

    if (-1 == (fd = open(filename, O_RDONLY, 0)))
      {
	if (errno == 0 || errno > sys_nerr)
	  sprintf(errtext, "loading file - error %d opening '%s'",
		  errno, filename);
	else
	  sprintf(errtext, "opening '%s': %s", filename, sys_errlist[errno]);

	XjWarning(errtext);
	return NULL;
      }

    if (-1 == fstat(fd, &buf)) /* could only return EIO */
      {
	if (errno == 0 || errno > sys_nerr)
	  sprintf(errtext, "loading file - error %d in fstat for '%s'",
		  errno, filename);
	else
	  sprintf(errtext, "fstat '%s': %s", filename, sys_errlist[errno]);

	XjWarning(errtext);
	close(fd);
	return NULL;
      }

    size = (int)buf.st_size;

    info = (char *)XjMalloc(size + 1);	/* one extra for the NULL */

    if (-1 == read(fd, info, size))
      {
	if (errno == 0 || errno > sys_nerr)
	  sprintf(errtext, "loading file - error %d reading '%s'",
		  errno, filename);
	else
	  sprintf(errtext, "reading '%s': %s", filename, sys_errlist[errno]);

	XjWarning(errtext);
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
 *  destroyChildren  --  mass murder...
 *
 ************************************************************************/
static void destroyChildren(reap)
     Menu *reap;
{
  Menu *anal;

  while (reap != NULL)
    {
      if (reap->child != NULL)
	destroyChildren(reap->child);
      anal = reap;
      reap = reap->sibling;
      XjFree(anal);
    }
}
    

/************************************************************************
 *
 *  destroyMenu  --  destroy yourself!  and your little dog too!
 *
 ************************************************************************/
static void destroyMenu(me)
     MenuJet me;
{
  destroyChildren(me->menu.rootMenu);
  XDeleteContext(me->core.display, me->menu.rootMenu->menuPane, context);
  XjFree(me->menu.rootMenu);
  me->menu.rootMenu = NULL;
}



/************************************************************************
 *
 *  loadNewMenus  --  load some new menus into the menu struct.
 *
 ************************************************************************/
/*
 * A lot of this procedure is duplicated from other places -
 * these things ought to be done in common routines. Prime opportunity
 * to introduce bugs by changing the other stuff and forgetting to
 * do it here.
 */
int loadNewMenus(me, file)
     MenuJet me;
     char *file;
{
  XjSize size;
  char *newItems;
  Item *i;

  newItems = loadFile(file);

  if (newItems != NULL && newItems[0] != '\0')
    {
      closeMenuAndAncestorsToLevel(me, me->menu.deepestOpened,
				   me->menu.rootMenu, False);

      me->menu.deepestOpened = me->menu.rootMenu;

      if (me->menu.rude == True)
	{
	  if (me->menu.grabbed == True)
	    XUngrabPointer(me->core.display, CurrentTime);
	  me->menu.grabbed = False;
	}

      if (me->menu.buttonDown == 1)
	me->menu.buttonDown = 0;

      createTablesFromString(me, newItems);
      destroyMenu(me);
      for (i = me->menu.firstMenu; i != NULL; i = i->next)
	i->u.m.m = NULL;

      createMenuFromTables(me);

      me->menu.rootMenu->state = OPENED;

      me->menu.rootMenu->menuPane = me->core.window;
      me->menu.rootMenu->pane_x = 0;
      me->menu.rootMenu->pane_y = 0;

      me->menu.same = False;
      me->menu.inside = False;
      me->menu.buttonDown = 0;

      if (XCNOMEM == XSaveContext(me->core.display,
				  me->menu.rootMenu->menuPane,
				  context, (caddr_t)me->menu.rootMenu))
	XjFatalError("out of memory in XSaveContext");

      me->menu.rootMenu->paneType = NORMAL;

      computeAllMenuSizes(me, me->menu.rootMenu);
      computeRootMenuSize(me, &size);
      me->core.width  = size.width;
      me->core.height = size.height;

#ifdef notdefined
      size.width = me->core.width; /* this is an official hack */
      size.height = me->core.height;
#endif
      XjResize(XjParent(me), &size);

      return 0;
    }
  else
    return 1;
}


/************************************************************************
 *
 *  createTablesFromCompiledFile  --  long enough name or what?
 *
 ************************************************************************/
static int
createTablesFromCompiledFile(me, file, fontname)
     MenuJet me;
     char *file, *fontname;
{
  char *comp_file;		/* "compiled" file name */
  struct stat buf;
  time_t mod_time;
  int i, num;
  Item *loc;
  char *str, *end;
  int samefont = False;

  int qnum;
  int tmp, mach_or_orntn, verify;
  int j;
  char *ptr;
  TypeDef *t;


  struct timeval start, t_end;

  if (DEBUG)
    {
      gettimeofday(&start, NULL);
      printf("createTablesFromCompiledFile: - %d.%d + \n", start.tv_sec, start.tv_usec);
    }

  if (file[0] == '\0'  ||  file == NULL)
    return False;

  if (-1 == stat(file, &buf))
    return False;
  mod_time = buf.st_mtime;

  if ((comp_file = XjMalloc((unsigned)strlen(file) + 5)) == NULL)
    return False;

  (void) strcpy(comp_file, file);
  (void) strcat(comp_file, ".cmp");

  if (-1 == stat(comp_file, &buf))
    {
      XjFree(comp_file);
      return False;
    }

  if (buf.st_mtime < mod_time)
    {
      XjFree(comp_file);
      return False;
    }

  str = loadFile(comp_file);
  XjFree(comp_file);

  if (str == NULL)
    return False;

  num = atoi(str);
  str += 5;

  tmp = atoi(str);
  str += 4;
  *(end = str+tmp) = '\0';
  if (!strcasecmp(str, fontname))
    samefont = True;
  str = ++end;

  loc = (Item *)XjMalloc(num * sizeof(Item));
  bzero(loc, num * sizeof(Item));

  for (i=0; i < num; i++)
    {
      loc->type = atoi(str);
      str += 2;
      tmp = atoi(str);
      str += 5;
      *(end = str+tmp) = '\0';
      /*      end = index(str, ' ');       *end = '\0';       */
      loc->name = XrmStringToQuark(str);
      str = ++end;
      loc->flags = atoi(str);
      str += 6;
      mach_or_orntn = atoi(str);
      str += 6;
      verify = atoi(str);
      str += 2;
      
      tmp = atoi(str);
      str += 5;
      *(end = str+tmp) = '\0';
      /* end = index(str, '\n');      *end = '\0';       */
      loc->title = str;
      str = ++end;

      if (samefont)
	loc->title_width = atoi(str);
      str += 6;

      switch(loc->type)
	{
	  /*
	   *  If an item, parse this-a-way...
	   */
	case ItemITEM:
	  loc->next = me->menu.firstItem;
	  me->menu.firstItem = loc;
	  loc->u.i.machtype = mach_or_orntn;
	  loc->u.i.verify = verify;
	  break;

	  /*
	   *  ...else, if a menu, parse that-a-way...
	   */
	case MenuITEM:
	  loc->next = me->menu.firstMenu;
	  me->menu.firstMenu = loc;
	  loc->u.m.orientation = mach_or_orntn;

	  j = 0;
	  do
	    {
	      tmp = atoi(str);
	      str += 5;
	      *(end = str+tmp) = '\0';
	      /* end = index(str, ' ');      *end = '\0';       */
	      loc->u.m.children[j++] = XrmStringToQuark(str);
	      str = end+1;
	    }
	  while ( *str != '\n' );
	  str++;
/*
	  ret = sscanf(str, "%s %s %s %s %s",
		       strs[0], strs[1], strs[2], strs[3], strs[4]);
	  for (j=0; j < ret; j++)
	    loc->u.m.children[j] = XrmStringToQuark(strs[j]);
*/

	  break;
	}

      /*
       *  Both menus and items have parents, so share this
       *  line-parsing...
       */

      qnum = 0;
      while (*str != '!')
	{
	  do
	    {
	      end = index(str, ' ');
	      *end = '\0';
	      loc->parents[qnum] = XrmStringToQuark(str);
	      str = ++end;
	      loc->weight[qnum++] = atoi(str);
	      str += 5;
	    }
	  while ( *str != '\n' );
	  str++;

	  if (qnum != MAXPARENTS)
	    {
	      loc->parents[qnum] = (XrmQuark) NULL;
	      loc->weight[qnum++] = 1;
	    }	  
/*
	  ret = sscanf(str, "%s %d %s %d %s %d %s %d %s %d %s %d %s %d %s %d %s %d %s %d",
		       strs[0], &wts[0], strs[1], &wts[1], strs[2], &wts[2],
		       strs[3], &wts[3], strs[4], &wts[4], strs[5], &wts[5],
		       strs[6], &wts[6], strs[7], &wts[7], strs[8], &wts[8],
		       strs[9], &wts[9]);
	  for (j=0; j < ret/2; j++)
	    {
	      loc->parents[qnum] = XrmStringToQuark(strs[j]);
	      loc->weight[qnum++] = wts[j];
	    }

	  str = ++end;
	  end = index(str, '\n');
	  *end = '\0';
*/

	}
      if (qnum != MAXPARENTS)
	{
	  qnum--;
	  loc->parents[qnum] = (XrmQuark) NULL;
	  loc->weight[qnum] = 0;
	}

      str += 2;

      switch(loc->type)
	{
	  /*
	   *  If an item, parse this-a-way...
	   */
	case ItemITEM:
	  tmp = atoi(str);
	  str += 5;
	  *(end = str+tmp) = '\0';
	  /*  end = index(str, '\n');	  *end = '\0'; */
	  ptr = str;

	  if (strncmp("(null)", ptr, 6))
	    {
	      if (NULL ==
		  (loc->u.i.activateProc = XjConvertStringToCallback(&ptr)))
		{
		  char errtext[100];
		  sprintf(errtext,
			  "'%s' - couldn't grok callback; ignoring entry",
			  XrmQuarkToString(loc->name));
		  XjWarning(errtext);
		  loc->flags &= ~activateFLAG; /* is this right??? */
		}
	    }

	  str = ++end;
	  tmp = atoi(str);
	  str += 5;
	  *(end = str+tmp) = '\0';
	  /* end = index(str, '!');	  while ( *(end-1) != '\n' ) */
	  /* end = index(end+1, '!');	  *end = '\0';  */
	  if (strncmp("(null)", str, 6))
	    {
	      loc->u.i.help = str;
	    }

	  str = end+3;

	  if (samefont)
	    {
	      loc->u.i.help_width = atoi(str);
	      str += 6;
	      loc->u.i.help_height = atoi(str);
	      str += 6;
	    }
	  else
	    str += 12;

	  break;

	  /*
	   *  ...else, if a menu, parse that-a-way...
	   */
	case MenuITEM:
	  /*
	   * Add the new ones...
	   */
	  for (j = 0; loc->u.m.children[j] != 0 && j < MAXCHILDREN; j++)
	    {
	      t = (TypeDef *)hash_lookup(me->menu.Types, loc->u.m.children[j]);
	      if (t == NULL)
		{
		  t = (TypeDef *)XjMalloc(sizeof(TypeDef));
		  bzero(t, sizeof(TypeDef));
		  (void)hash_store(me->menu.Types, loc->u.m.children[j], t);
		  t->type = loc->u.m.children[j];
		  t->menus[0] = loc;
		}
	      else
		{
		  int m;

		  for (m = 0; m < MAXMENUSPERTYPE && t->menus[m] != 0; m++);
		  if (m == MAXMENUSPERTYPE)
		    {
		      char errtext[100];
		      
		      sprintf(errtext, "'%s' not typed as %s due to overflow",
			      XrmQuarkToString(loc->name),
			      XrmQuarkToString(t->type));
		      XjWarning(errtext);
		    }
		  else
		    t->menus[m] = loc;
		}
	    }
	  break;
	}

      (void)hash_store(me->menu.Names, loc->name, loc);
      loc++;
    }

  if (DEBUG)
    {
      gettimeofday(&t_end, NULL);
      printf("createTablesFromCompiledFile: %d.%d = %d.%06.6d\n",
	     t_end.tv_sec, t_end.tv_usec,
	     (t_end.tv_usec > start.tv_usec)
	     ? t_end.tv_sec - start.tv_sec
	     : t_end.tv_sec - start.tv_sec - 1,
	     (t_end.tv_usec > start.tv_usec)
	     ? t_end.tv_usec - start.tv_usec
	     : t_end.tv_usec + 1000000 - start.tv_usec );
    }

  return True;
}



/************************************************************************
 *
 *  initialize  --  the mother of all routines...
 *
 ************************************************************************/
static void initialize(me)
     MenuJet me;
{
  struct timeval start, end;
  unsigned long foo;
  char *fontname;
  XjSize size;
  Bool font_prop;
  if (DEBUG)
    {
      gettimeofday(&start, NULL);
      printf("initialize: - %d.%d + \n", start.tv_sec, start.tv_usec);
    }

  font_prop = XGetFontProperty(me->menu.font, XA_FONT, &foo);
  if (font_prop)
    fontname = XGetAtomName(me->core.display, foo);

  me->menu.Names = create_hash(HASHSIZE);
  me->menu.Types = create_hash(HASHSIZE);
  me->menu.firstItem = NULL;
  me->menu.firstMenu = NULL;

  /*
   * load up default menus...
   */
  if (!createTablesFromCompiledFile(me, me->menu.file,
				    font_prop ? fontname : "" ))
    {
      if (me->menu.file[0] != '\0')
	me->menu.items = loadFile(me->menu.file);

      if (me->menu.items == NULL  &&  me->menu.fallback[0] != '\0')
	{
	  Jet root=(Jet)me;

	  XjWarning("trying fallback file");
	  while (XjParent(root))
	    root=XjParent(root);

	  XjUserWarning(root, NULL, False,
			"Unable to load menu file, trying fallback file.",
			"Try restarting dash later to get up-to-date menus.");
	  me->menu.items = loadFile(me->menu.fallback);
	}

      if (me->menu.items != NULL &&
	  me->menu.items[0] != '\0') /* a bit of a lose, but... */
	createTablesFromString(me, me->menu.items);
      else
	createTablesFromString(me, NOMENU);
    }
  if (font_prop)
    XjFree(fontname);

  /*
   * load up user-defined stuff...
   */
  if (me->menu.userFile[0] != '\0')
    me->menu.userItems = loadFile(me->menu.userFile);

  if (me->menu.userItems != NULL &&
      me->menu.userItems[0] != '\0') /* a bit of a lose, but... */
    createTablesFromString(me, me->menu.userItems);

  /*
   * create menus from everything...
   */
  createMenuFromTables(me);

#ifdef notdefined
  if (print)
    {
      struct timeval start, end;

      gettimeofday(&start, NULL);
      printf("initialize(printTable): - %d.%d + \n", start.tv_sec, start.tv_usec);

      printTable(me->menu.firstMenu);
      printTable(me->menu.firstItem);
      print=0;

      gettimeofday(&end, NULL);
      printf("initialize(printTable): %d.%d = %d.%06.6d\n", end.tv_sec, end.tv_usec,
	     (end.tv_usec > start.tv_usec)
	     ? end.tv_sec - start.tv_sec
	     : end.tv_sec - start.tv_sec - 1,
	     (end.tv_usec > start.tv_usec)
	     ? end.tv_usec - start.tv_usec
	     : end.tv_usec + 1000000 - start.tv_usec );
    }
#endif

  /*
   * Here all of the menu tree has been created and filled in
   * with titles, callbacks, etc. Now we go through and compute the
   * necessary sizes of all of the menu panes.
   */

  me->menu.rootMenu->paneType = NORMAL;

  computeAllMenuSizes(me, me->menu.rootMenu);

  computeRootMenuSize(me, &size);
  (XjParent(me))->core.width  = me->core.width  = size.width;
  (XjParent(me))->core.height = me->core.height = size.height;

  me->menu.grabbed = False;
  me->menu.scrollx = 0;
  if (DEBUG)
    {
      gettimeofday(&end, NULL);
      printf("initialize: %d.%d = %d.%06.6d\n", end.tv_sec, end.tv_usec,
	     (end.tv_usec > start.tv_usec)
	     ? end.tv_sec - start.tv_sec
	     : end.tv_sec - start.tv_sec - 1,
	     (end.tv_usec > start.tv_usec)
	     ? end.tv_usec - start.tv_usec
	     : end.tv_usec + 1000000 - start.tv_usec );
    }
}



/************************************************************************
 *
 *  realize  --  think about it...
 *
 ************************************************************************/
static void realize(me)
     MenuJet me;
{
  unsigned long valuemask;
  XGCValues values;
  int temp;

  /*
   * Initialize GC's
   */

  if (me->menu.reverseVideo)
    {
      temp = me->menu.foreground;
      me->menu.foreground = me->menu.background;
      me->menu.background = temp;
    }

  values.function = GXcopy;
  values.line_width = 0;
  values.foreground = me->menu.foreground;
  values.background = me->menu.background;
  values.font = me->menu.font->fid;
  values.graphics_exposures = False;
  valuemask = ( GCForeground | GCBackground | GCFunction | GCFont
	       | GCLineWidth | GCGraphicsExposures );

  me->menu.gc = XjCreateGC(me->core.display,
			   me->core.window,
			   valuemask,
			   &values);

  values.line_width = 0;
  values.line_style = LineOnOffDash;
  values.dashes = (char)1;
  valuemask |= GCLineStyle | GCDashList;

  me->menu.dash_gc = XjCreateGC(me->core.display,
				me->core.window,
				valuemask,
				&values);

  me->menu.dot_gc = XjCreateGC(me->core.display,
			       me->core.window,
			       valuemask,
			       &values);

  XSetDashes(me->core.display, me->menu.dash_gc, 1, "\001\002", 2);
  XSetDashes(me->core.display, me->menu.dot_gc, 1, "\001\002", 2);

  valuemask &= ~(GCLineStyle | GCDashList);

  me->menu.dim_gc = XjCreateGC(me->core.display,
			       me->core.window,
			       valuemask,
			       &values);

  if (me->menu.grey != NULL)
    { /* Turn into a toolkit routine... */
      Pixmap p;

      p = XjCreatePixmap(me->core.display,
			 me->core.window,
			 me->menu.grey->width,
			 me->menu.grey->height,
			 DefaultDepth(me->core.display, /* wrong... sigh. */
				      DefaultScreen(me->core.display)));

      XCopyPlane(me->core.display,
		 me->menu.grey->pixmap,
		 p,
		 me->menu.gc,
		 0, 0,
		 me->menu.grey->width,
		 me->menu.grey->height,
		 0, 0, 1);

      valuemask = GCFillStyle | GCTile;
      values.fill_style = FillTiled;
      values.tile = p;

      XChangeGC(me->core.display,
		me->menu.dim_gc,
		valuemask,
		&values);
    }

  values.line_width = 0;
  values.foreground = me->menu.background;
  valuemask = ( GCForeground | GCBackground | GCFunction | GCFont
	       | GCLineWidth | GCGraphicsExposures );
  me->menu.background_gc = XjCreateGC(me->core.display,
				      me->core.window,
				      valuemask,
				      &values);

  values.function = GXxor;
  values.foreground = (me->menu.foreground ^
		       me->menu.background);

  me->menu.invert_gc = XjCreateGC(me->core.display,
				  me->core.window,
				  valuemask,
				  &values);
  /*
   * Do some context initialization
   */

  if (!gotContextType)
    {
      context = XUniqueContext();
      gotContextType ++;
    }

  /* rootMenu is always opened */
  me->menu.deepestOpened = me->menu.rootMenu;
  me->menu.rootMenu->state = OPENED;

  me->menu.rootMenu->menuPane = me->core.window;
  me->menu.rootMenu->pane_x = 0;
  me->menu.rootMenu->pane_y = 0;

  me->menu.same = False;
  me->menu.inside = False;
  me->menu.buttonDown = 0;

  XjSelectInput(me->core.display, me->core.window,
	       ExposureMask | ButtonPressMask | ButtonReleaseMask |
	       ButtonMotionMask | EnterWindowMask /* | LeaveWindowMask */
		/*| StructureNotifyMask */);

  /*
   * Send the events for this window to us
   */

  XjRegisterWindow(me->core.window, me);

  /*
   * We also want to remember what part of the tree
   * this menu belongs to
   */
  if (XCNOMEM == XSaveContext(me->core.display, me->menu.rootMenu->menuPane,
			      context, (caddr_t)me->menu.rootMenu))
    XjFatalError("out of memory in XSaveContext");

  /*
   * futz around with "rightJet"...
   */
  if (me->menu.rightJet)
    {
      me->menu.right_jet = XjFindJet(me->menu.rightJet, XjParent(me));
      if (me->menu.right_jet)
	{
	  XjSize size;
	  int x = me->menu.rootMenu->pane_width
	    + me->menu.rootMenu->pane_x;

	  size.width  = me->core.width - x;
	  size.height = me->core.height;
	  XjMove(me->menu.right_jet, x, me->menu.rootMenu->pane_y);
	  XjResize(me->menu.right_jet, &size);
	}
    }
}




static int (*def_handler)();
static int menu_error=0;
static int handler(display, error)
     Display *display;
     XErrorEvent *error;
{
  if (error->error_code == BadWindow  ||  error->error_code == BadAtom)
    {
      menu_error=1;
      return 0;
    }

  def_handler(display, error);
  return 0;			/* it'll never get this far anyway... */
}

/************************************************************************
 *
 *  findMenu  --  figure out where the user clicked...
 *
 ************************************************************************/
void findMenu(me, who, what, start, window, rx, ry)
     MenuJet me;
     Menu **who;	/* what menu we found the mouse to be in */
     int *what;		/* what state that menu ought to be in */
     Menu *start;	/* where to start scanning the menu tree */
     Window window;	/* window mouse location reported relative to */
     int rx, ry;	/* where the mouse is */
{
  int x, y;
  Menu *level, *ptr;
  Window scratch;

  menu_error=0;
  def_handler = XSetErrorHandler(handler);
  if (window != me->core.window)
    (void)XTranslateCoordinates(me->core.display,
				window,
				me->core.window,
				rx, ry,
				&rx, &ry, &scratch);
  (void) XSetErrorHandler(def_handler);
  if (menu_error)
    {
      *who=NULL;
      menu_error=0;
      return;
    }
  menu_error=0;

#ifdef DEBUGEVENTS
      fprintf(stderr, "find: %d, %d\n",rx, ry);
#endif

  for (level = start; level != NULL; level = level->parent)
    {
      x = rx - level->pane_x;
      y = ry - level->pane_y;

      if (level->state == OPENED)
	{
	  switch(level->orientation)
	    {
	    case HORIZONTAL:
	      for (ptr = level->child; ptr != NULL; ptr = ptr->sibling)
		if (x >= ptr->label_x && y >= BORDER &&
		    x < ptr->label_x + ptr->label_width &&
		    y < level->pane_height - BORDER - SHADOW)
		  {
		    *who = ptr;
		    if (ptr->paneType == HELP ||
			ptr->child == NULL)
		      *what = SELECTED;
		    else
		      *what = OPENED;
		    return;
		  }
	      break;

	    case VERTICAL:
	      for (ptr = level->child; ptr != NULL; ptr = ptr->sibling)
		if (x >= BORDER && y >= ptr->label_y &&
		    x < level->pane_width - BORDER - SHADOW &&
		    y < ptr->label_y + ptr->label_height)
		  {
		    *who = ptr;

		    *what = CLOSED;

/*		    if (ptr->paneType == HELP &&
			me->menu.showHelp == 0)
		      *what = SELECTED;
		    else */
		      if (x >= level->pane_open_x)
			{
			  if (ptr->child != NULL &&
			      (ptr->paneType != HELP ||
			       (ptr->paneType == HELP &&
				me->menu.showHelp != 0)))
			    *what = OPENED;
			}
		      else
			*what = SELECTED;
/*
		    if (*what == SELECTED &&
			ptr->activateProc == NULL)
		      *what = CLOSED; */

		    return;
		  }
	      break;
	    }

	  if (x >= 0 && x < level->pane_width &&
	      y >= 0 && y < level->pane_height)
	    {
	      *who = SAME;
	      return;
	    }
	}
    }

  *who = NULL;
  return;
}



/************************************************************************
 *
 *  highlightMenu  --  highlight the current item.
 *
 ************************************************************************/
static void highlightMenu(me, menu, state, gc)
     MenuJet me;
     Menu *menu;
     int state;
     GC gc;
{
#ifdef DEBUGSETTINGS
  fprintf(stderr, "Invert %s\n", menu->title);
#endif

  if (menu->parent->paneType == HELP)
    return;

  switch(menu->parent->orientation)
    {
    case HORIZONTAL:
      XDrawRectangle(me->core.display, menu->parent->menuPane,
		     gc,
		     menu->label_x + me->menu.hMenuPadding / 4,
		     BORDER + me->menu.vMenuPadding / 4,
		     menu->label_width - me->menu.hMenuPadding / 2 - 1,
		     menu->parent->pane_height -
		       2 * BORDER - SHADOW - 1 - me->menu.vMenuPadding / 2);
      break;

    case VERTICAL: /* make sure pane_open_x gets inited */
      if (state == OPENED)
	XDrawRectangle(me->core.display, menu->parent->menuPane,
		       gc,
		       menu->parent->pane_open_x + me->menu.hMenuPadding / 4,
		       menu->label_y + me->menu.vMenuPadding / 4,
		       menu->parent->pane_width - BORDER - SHADOW -
		    menu->parent->pane_open_x - 1 - me->menu.hMenuPadding / 2,
		       menu->label_height - 1 - me->menu.vMenuPadding / 2);
      else
	if (state == SELECTED)
	XDrawRectangle(me->core.display, menu->parent->menuPane,
		       gc,
		       BORDER + me->menu.hMenuPadding / 4,
		       menu->label_y + me->menu.vMenuPadding / 4,
          menu->parent->pane_open_x - BORDER - 1 - me->menu.hMenuPadding / 2,
		       menu->label_height - 1 - me->menu.vMenuPadding / 2);
      break;
    }
}



/************************************************************************
 *
 *  offset  --  move the menu bar...  there be beasties here...
 *
 ************************************************************************/
static void offset(me, menu, delta)
     MenuJet me;
     Menu *menu;
     int delta;
{
  if (menu->parent != NULL)
    offset(me, menu->parent, delta);
  else
    me->menu.scrollx += delta;

/*   Not quite right...  close...
  if (menu->menuPane == me->core.window)
    {
      me_moving = 1;
      XMoveWindow(me->core.display, menu->menuPane,
		  me->core.x + delta, me->core.y);
    }
  else
  */
  menu->x += delta;
  me_moving = 1;
  XMoveWindow(me->core.display, menu->menuPane, menu->x, menu->y);
}


/************************************************************************
 *
 *  woffset  --  offset and Warp!  (more beasties...)
 *
 ************************************************************************/
static void woffset(me, menu, delta, warp)
     MenuJet me;
     Menu *menu;
     int delta;
     Boolean warp;
{
  offset(me, menu, delta);

  if (warp)
    XWarpPointer(me->core.display, None, None,
		 0, 0, 0, 0,
		 delta, 0);
}



/************************************************************************
 *
 *  openMenu  --  user clicked on another menu...  open up!
 *
 ************************************************************************/
/*
 * Add window caching code?
 */
static void openMenu(bar, menu)
     MenuJet bar;
     Menu *menu;
{
  Menu *checkMenu;
  int x, y, delta;
  Window parentwindow, scratch;
  int screen;
  WindowJet wj;

#ifdef DEBUGSETTINGS
  fprintf(stderr, "Open %s\n", menu->title);
#endif

  if (menu->child && !menu->menuPane /* twm problems... */)
    {
      XSetWindowAttributes attributes;
      int attribsMask;

      attributes.override_redirect = True;
      attribsMask = CWOverrideRedirect;

      attributes.background_pixel = bar->menu.background;
      attribsMask |= CWBackPixel;

      screen = DefaultScreen(bar->core.display);

      /*
       * Create the menu pane
       */
      parentwindow = RootWindow(bar->core.display, screen);

      /*
       * This is a hack to figure out where to put the window -
       * it's definitely the wrong way to go, but I get anxious
       * sometimes. I'll fix it later. Honest.
       * (He's lying.)
       */
      XTranslateCoordinates(bar->core.display,
			    menu->parent->menuPane,
			    parentwindow,
			    0, 0,
			    &x, &y,
			    &scratch);

      switch(menu->parent->orientation)
	{
	case HORIZONTAL:
	  x += menu->label_x;
	  y += menu->parent->pane_height/* - BORDER*/;

	  menu->pane_x = menu->parent->pane_x + menu->label_x;
	  menu->pane_y = menu->parent->pane_y + menu->parent->pane_height;
	  break;

	case VERTICAL:
	  x += menu->parent->pane_width/* - BORDER */;
	  y += menu->label_y;

	  menu->pane_x = menu->parent->pane_x + menu->parent->pane_width;
	  menu->pane_y = menu->parent->pane_y + menu->label_y;
	  break;
	}

      delta = 0;
      if (bar->menu.moveMenus)
	{
	  for (checkMenu = menu->parent->child;
	       checkMenu != NULL;
	       checkMenu = checkMenu->sibling)
	    delta = MAX(delta, checkMenu->pane_width);

	  if ((delta =
	       (DisplayWidth(bar->core.display,
			     DefaultScreen(bar->core.display))
		- (x + delta))) < 0)
	    {
	      woffset(bar, menu->parent, delta, True);
	      bar->menu.responsibleParent = menu->parent;
	    }
	}
      menu->x = x + (delta < 0 ? delta : 0);
      menu->y = y;

      menu->menuPane =
	XCreateWindow(bar->core.display,
		      parentwindow,
		      menu->x, menu->y,
		      menu->pane_width,
		      menu->pane_height,
		      0,
		      DefaultDepth(bar->core.display,
				   DefaultScreen(bar->core.display)),
		      InputOutput,
		      CopyFromParent,
		      attribsMask,
		      &attributes);

      wj = (WindowJet) XjParent(bar);
      if (wj->window.cursor != (Cursor) NULL)
	XDefineCursor(bar->core.display, menu->menuPane, wj->window.cursor);

      XSelectInput(bar->core.display, menu->menuPane,
		   ExposureMask | StructureNotifyMask | ButtonPressMask |
		   ButtonReleaseMask | ButtonMotionMask);

      XMapWindow(bar->core.display, menu->menuPane);

      /*
       * Send the events for this window to us
       */

      XjRegisterWindow(menu->menuPane, bar);

      /*
       * We also want to remember what part of the tree
       * this menu belongs to
       */

      if (XCNOMEM == XSaveContext(bar->core.display,
				  menu->menuPane,
				  context, (caddr_t)menu))
	XjFatalError("out of memory in XSaveContext");
    }
}



/************************************************************************
 *
 *  closeMenu  --  user closed a menu...
 *
 ************************************************************************/
static void closeMenu(bar, menu, warp)
     MenuJet bar;
     Menu *menu;
     Boolean warp;
{
  int freespace;
#ifdef DEBUGSETTINGS
  fprintf(stderr, "Close %s\n", menu->title);
#endif

  if (menu->menuPane != (Window) NULL)
    {
      XDestroyWindow(bar->core.display, menu->menuPane);
      menu->menuPane = (Window) NULL;

      if (bar->menu.scrollx != 0 && menu->parent != NULL &&
	  bar->menu.responsibleParent == menu)
	{
	  menu = menu->parent;
	  bar->menu.responsibleParent = menu;
	  freespace = DisplayWidth(bar->core.display,
				   DefaultScreen(bar->core.display)) -
		      (menu->x + menu->pane_width);
	  woffset(bar, menu, MIN(freespace, -bar->menu.scrollx), warp);
	}
    }
}



/************************************************************************
 *
 *  setMenu  --  I have no idea...
 *
 ************************************************************************/
static void setMenu(me, menu, newState, warp)
     MenuJet me;
     Menu *menu;
     int newState;
     Boolean warp;
{
  switch(newState)
    {
    case CLOSED:
      switch(menu->state)
	{
	case CLOSED:
	  break;
	case OPENED:
	  closeMenu(me, menu, warp);
	  highlightMenu(me, menu, OPENED, me->menu.background_gc);
	  break;
	case SELECTED:
	  highlightMenu(me, menu, SELECTED, me->menu.background_gc);
	  break;
	}
      menu->state = CLOSED;
      break;

    case SELECTED:
      switch(menu->state)
	{
	case CLOSED:
	  highlightMenu(me, menu, SELECTED,
			(menu->activateProc == NULL ||
		(MACHNUM != UNKNOWNNUM && (MACHNUM & menu->machtype) == 0)) ?
			me->menu.dot_gc : me->menu.gc);
	  break;
	case OPENED:
	  closeMenu(me, menu, warp);
	  highlightMenu(me, menu, OPENED, me->menu.background_gc);
	  highlightMenu(me, menu, SELECTED,
			(menu->activateProc == NULL ||
		(MACHNUM != UNKNOWNNUM && (MACHNUM & menu->machtype) == 0)) ?
			me->menu.dot_gc : me->menu.gc);
/*	  highlightMenu(me, menu, SELECTED, menu->activateProc == NULL ?
			me->menu.dot_gc : me->menu.gc); */
	  break;
	case SELECTED:
	  break;
	}
      menu->state = SELECTED;
      break;

    case OPENED:
      switch(menu->state)
	{
	case CLOSED:
	  highlightMenu(me, menu, OPENED, me->menu.dot_gc);
	  openMenu(me, menu);
	  break;
	case OPENED:
	  break;
	case SELECTED:
	  highlightMenu(me, menu, SELECTED, me->menu.background_gc);
	  highlightMenu(me, menu, OPENED, me->menu.dot_gc);
	  openMenu(me, menu);
	  break;
	}
      menu->state = OPENED;
      break;
    }
}



/************************************************************************
 *
 *  closeMenuAndAncestorsToLevel  --  close menu and ancestors up to
 *	level specified...
 *
 ************************************************************************/
static void closeMenuAndAncestorsToLevel(bar, start, end, warp)
     MenuJet bar;
     Menu *start, *end;
     Boolean warp;
{
  Menu *ptr;

  for (ptr = start; ptr != end && ptr != end->parent; ptr = ptr->parent)
    setMenu(bar, ptr, CLOSED, warp);
  XFlush(bar->core.display);
}


/************************************************************************
 *
 *  drawBackground  --  draw background of menu window...
 *
 ************************************************************************/
static void drawBackground(me, menu)
     MenuJet me;
     Menu *menu;
{
  int width = menu->pane_width;
  int height = menu->pane_height;

  if (me->core.window == menu->menuPane)
    width = me->core.width;
/*
  XFillRectangle(me->core.display, menu->menuPane,
		 me->menu.background_gc,
		 BORDER, BORDER,
		 width - 2 * BORDER - SHADOW,
		 height - 2 * BORDER - SHADOW);
*/
  XDrawRectangle(me->core.display, menu->menuPane,
		 me->menu.gc,
		 0, 0,
		 width - SHADOW - 1,
		 height - SHADOW - 1);

  XDrawLine(me->core.display, menu->menuPane,
	    me->menu.gc,
	    BORDER, height - 1,
	    width - 1, height - 1);

  XDrawLine(me->core.display, menu->menuPane,
	    me->menu.gc,
	    width - 1, BORDER,
	    width - 1, height - 1);
}



/************************************************************************
 *
 *  drawPane  --  draw the items in a menu pane.
 *
 ************************************************************************/
static void drawPane(me, eventMenu)
     MenuJet me;
     Menu *eventMenu;
{
  Menu *m;

  drawBackground(me, eventMenu);

  if (eventMenu->paneType == HELP)
    drawHelp(me, eventMenu);
  else
    {
      if (eventMenu->orientation == VERTICAL)
	XDrawLine(me->core.display,
		  eventMenu->menuPane,
		  me->menu.dash_gc,
		  eventMenu->pane_open_x,
		  BORDER,
		  eventMenu->pane_open_x,
		  eventMenu->pane_height - BORDER);

      for (m = eventMenu->child; m != NULL; m = m->sibling)
	{
	  if (m->activateProc != NULL)
	    XDrawString(me->core.display, eventMenu->menuPane,
			(MACHNUM != UNKNOWNNUM && (MACHNUM & m->machtype) == 0) ?
			me->menu.dim_gc : me->menu.gc,
			m->label_x + me->menu.hMenuPadding / 2 - 10,
			m->label_y + me->menu.vMenuPadding / 2 +
			me->menu.font->ascent,
			"*", 1);

	  XDrawString(me->core.display, eventMenu->menuPane,
		      /* (m->activateProc != NULL ||
			 (m->paneType != HELP && m->child != NULL)) ? */
		      (MACHNUM != UNKNOWNNUM && (MACHNUM & m->machtype) == 0) ?
		      me->menu.dim_gc : me->menu.gc,
		      m->label_x + me->menu.hMenuPadding / 2,
		      m->label_y + me->menu.vMenuPadding / 2 +
		      me->menu.font->ascent,
		      m->title, strlen(m->title));

	  switch(m->state)
	    {
	    case OPENED:
	      highlightMenu(me, m, OPENED, me->menu.dot_gc);
	      break;

	    case SELECTED:
	      highlightMenu(me, m, SELECTED,
			    (m->activateProc == NULL ||
	       (MACHNUM != UNKNOWNNUM && (MACHNUM & m->machtype) == 0)) ?
			    me->menu.dot_gc : me->menu.gc);
	      break;

	    case CLOSED:
	      break;
	    }

	  if (eventMenu->orientation == VERTICAL)
	    {
	      if (m->paneType == HELP)
		{
		  if (me->menu.showHelp)
		    XCopyPlane(me->core.display,
			       me->menu.helpPixmap->pixmap,
			       eventMenu->menuPane,
			       me->menu.gc,
			       0, 0,
			       me->menu.helpPixmap->width,
			       me->menu.helpPixmap->height,
			       eventMenu->pane_open_x +
			       me->menu.hMenuPadding / 2,
			       m->label_y +
			       me->menu.vMenuPadding / 2,
			       1);
		}
	      else
		if (m->child != NULL)
		  XCopyPlane(me->core.display,
			     me->menu.submenuPixmap->pixmap,
			     eventMenu->menuPane,
			     me->menu.gc,
			     0, 0,
			     me->menu.submenuPixmap->width,
			     me->menu.submenuPixmap->height,
			     eventMenu->pane_open_x +
			     me->menu.hMenuPadding / 2,
			     m->label_y +
			     me->menu.vMenuPadding / 2,
			     1);
	    }
	}
    }
}



/************************************************************************
 *
 *  event_handler  --  deal with all the x events that get thrown at us.
 *
 ************************************************************************/
static Boolean event_handler(me, event)
     MenuJet me;
     XEvent *event;
{
  MenuInfo info;
  Menu *eventMenu;
  Menu *m;
  int state, newState;

  if (XCNOENT == XFindContext(me->core.display, event->xany.window,
			      context, (caddr_t *)&eventMenu))
    return False;

  switch(event->type)
    {
    case GraphicsExpose:
    case Expose:
      if (DEBUG)
      fprintf(stderr, "Exposure\n");

      if (event->xexpose.count != 0)
	break;
      if (eventMenu->menuPane != (Window) NULL)
	{
	  drawPane(me, eventMenu);
	}

      /* hack to support clock in menu bar */
      if (me->menu.right_jet)
	XjExpose(me->menu.right_jet, event);
#ifdef notdefined	  
      if (me->core.sibling != NULL &&
	  me->core.sibling->core.window == event->xany.window)
	XjExpose(me->core.sibling, event);
#endif
      break;

    case ButtonRelease:
#ifdef DEBUGEVENTS
      fprintf(stderr, "ButtonRelease to %d (%d) %d\n", 
	      event->xbutton.button, me->menu.buttonDown - 1,
	      event->xbutton.state);
#endif

      findMenu(me, &m, &newState, me->menu.deepestOpened,
	       event->xbutton.window,
	       event->xbutton.x, event->xbutton.y);

      if (me->menu.buttonDown == 1 &&
	  (me->menu.deepestOpened->state == SELECTED ||
	   m == NULL))
	{
	  state = me->menu.deepestOpened->state;
	  closeMenuAndAncestorsToLevel(me, me->menu.deepestOpened,
				       me->menu.rootMenu, False);

	  if (me->menu.inside == True || me->menu.same == True)
	    {
	      info.menubar = me;
	      /* there's potentially a bug here, but I don't want
		 to think about it right now. */
	      if ((me->menu.deepestOpened->parent != NULL &&
		  me->menu.deepestOpened->parent->paneType == HELP) ||
		  state != SELECTED)
		info.menu = me->menu.deepestOpened->parent; /* NOP */
	      else
		{
		  info.null = NULL;
		  info.menu = me->menu.deepestOpened;
		  if (MACHNUM == UNKNOWNNUM ||
		      (MACHNUM & me->menu.deepestOpened->machtype) != 0 &&
		      me->menu.deepestOpened->activateProc != NULL)
		    if (me->menu.verifyProc != NULL &&
			me->menu.deepestOpened->verify)
		      XjCallCallbacks(&info,
				      me->menu.verifyProc, NULL);
		    else
		      XjCallCallbacks(&info,
				      me->menu.deepestOpened->activateProc,
				      event);
		}
	    }

	  me->menu.deepestOpened = me->menu.rootMenu;

	  me->menu.inside = False;
	  me->menu.buttonDown = 0;

	  if (me->menu.rude == True)
	    {
	      if (me->menu.grabbed == True)
		XUngrabPointer(me->core.display, CurrentTime);
	      me->menu.grabbed = False;
	    }
	}
      else
	if (me->menu.buttonDown == 1)
	  me->menu.buttonDown = 0;

      break;

    case ButtonPress:
      if (me->core.window == event->xany.window)
	{
	  if (WindowVisibility(XjParent(me)) != VisibilityUnobscured)
	    XRaiseWindow(me->core.display, me->core.window);
	  else
	    {
	      Jet rj = me->menu.right_jet;
	      if (rj
		  && ((event->xbutton.x > rj->core.x)
		      && (event->xbutton.x < rj->core.x + rj->core.width))
		  && ((event->xbutton.y > rj->core.y)
		      && (event->xbutton.y < rj->core.y + rj->core.height))
		  && rj->core.classRec->core_class.event)
		{
		  if (rj->core.classRec->core_class.event(rj, event))
		    break;
		}
	    }
	}

#ifdef DEBUGEVENTS
      fprintf(stderr, "ButtonPress to %d (%d) %d\n",
	      event->xbutton.button, me->menu.buttonDown + 1,
	      event->xbutton.state);
#endif
      if (me->menu.buttonDown == 0)
	{
	  if (me->menu.rude == True &&
	      me->menu.grabbed == False)
	    {
	      XGrabPointer(me->core.display,
			   me->core.window,
			   False,
			   ButtonPressMask |
			   ButtonReleaseMask |
			   ButtonMotionMask |
			   EnterWindowMask
			   /* | LeaveWindowMask */ ,
			   GrabModeAsync,
			   GrabModeAsync,
			   None,
			   None,
			   CurrentTime);
	      me->menu.grabbed = True;
	    }
	  me->menu.buttonDown = 1;
	}

/*      break; */

    case MotionNotify:
/*
#ifdef DEBUGERVENTS
      fprintf(stderr, "MotionNotify: %d, %d; %d\n",
	      event->xmotion.x, event->xmotion.y, event->xmotion.state);
#endif
*/
      if (me->menu.buttonDown == 0)
	break;

      findMenu(me, &m, &newState, me->menu.deepestOpened,
	       event->xmotion.window,
	       event->xmotion.x, event->xmotion.y);

      if (m == SAME)
	{
	  me->menu.same = True;
	  break;
	}

      me->menu.same = False;

      /* We're outside of all menus */
      if (m == NULL)
	{
	  if (me->menu.inside == False)
	    break; /* just moving around outside menus */

	  /* Here we've just moved outside of all menus */
	  /* if (m == NULL && me->menu.inside == True) */
	  setMenu(me, me->menu.deepestOpened, CLOSED, True);
	  me->menu.deepestOpened =
	    me->menu.deepestOpened->parent;
	  me->menu.inside = False;
	  break;
	}

      /* m != NULL ... */
      if (me->menu.inside == False) /* just moved into something */
	{
	  if (m == me->menu.deepestOpened) /* merge? */
	    {
	      if (m->state != newState)
		setMenu(me, m, newState, True);

	      me->menu.inside = True; /* this is an unlikely case */
	      break;
	    }

	  /* so we just moved into something that's not what's
	     already opened */
	  me->menu.inside = True;
	}

      if (m == me->menu.deepestOpened) /* merge? */
	{
	  if (m->state != newState)
	    setMenu(me, m, newState, True);
	  
	  break;
	}

      if (m->parent == me->menu.deepestOpened) /* opened a deeper child */
	{
	  me->menu.deepestOpened = m;
	  setMenu(me, me->menu.deepestOpened, newState, True);
	  break;
	}

      /*
       * Here we have moved into a sibling or parent of the deepest
       * child. We must close everything from the level of that
       * parent/sibling on down.
       */
      closeMenuAndAncestorsToLevel(me, me->menu.deepestOpened, m, True);

      if (m->menuPane == (Window) NULL)
	setMenu(me, m, newState, True);

      me->menu.deepestOpened = m;

      break;

    case CirculateNotify:
      if (DEBUG)
	printf("Menu - circ: x=%d, y=%d\n",
	       event->xconfigure.x, event->xconfigure.y);
      break;

    case CreateNotify:
      if (DEBUG)
	printf("Menu - create: x=%d, y=%d\n",
	       event->xconfigure.x, event->xconfigure.y);
      break;

    case ConfigureNotify:
      if (me->core.window == event->xany.window)
	{
	  if (DEBUG)
	    printf("Menu - config: x=%d, y=%d\n",
		   event->xconfigure.x, event->xconfigure.y);
	  me->menu.rootMenu->x = event->xconfigure.x;
	  me->menu.rootMenu->y = event->xconfigure.y;
	}
      break;

    case DestroyNotify:
      XDeleteContext(me->core.display, event->xany.window, context);
      XjUnregisterWindow(event->xany.window, me);
      break;

    case GravityNotify:
    case MapNotify:
    case ReparentNotify:
    case UnmapNotify:
      break;

    case EnterNotify:
      if (me->menu.autoRaise &&
	  me->core.window == event->xany.window &&
	  WindowVisibility(XjParent(me)) != VisibilityUnobscured)
	XRaiseWindow(me->core.display, me->core.window);
      break;

    case LeaveNotify:
      if (me->menu.buttonDown == 0)
	{
	  Cursor cursor = XjCreateFontCursor(me->core.display,
					     XC_ul_angle);
	  XDefineCursor(me->core.display, me->core.window, cursor);
	}

    default:
      return False;
    }
  return True;
}



/************************************************************************
 *
 *  tab  --  space out...
 *
 ************************************************************************/
static void tab(n)
     int n;
{
  while (n--)
    fprintf(stdout, "    ");
}


/************************************************************************
 *
 *  printmenu  --  print out menu struct in a human-readable form.
 *
 ************************************************************************/
static void printmenu(first, tabbing)
     Menu *first;
     int tabbing;
{
  Menu *ptr;
  char *start, *end;

  for (ptr = first; ptr != NULL; ptr = ptr->sibling)
    {
      tab(tabbing);

      if (ptr->activateProc != NULL)
	fprintf(stdout, "*");

      if (!strncmp("   ", ptr->title, 3))
	fprintf(stdout, "%s", ptr->title + 3);
      else if (!strncmp("* ", ptr->title, 2))
	fprintf(stdout, "%s", ptr->title + 2);
      else
	fprintf(stdout, "%s", ptr->title);

      if (ptr->child != NULL)
	fprintf(stdout, ":\n");
      else
	fprintf(stdout, "\n");

      if (ptr->paneType == HELP &&
	  ptr->child != NULL)
	{
	  start = ptr->child->title;

	  while (*start != '\0')
	    {
	      end = index(start, '\n');
	      if (end == NULL)
		end = start + strlen(start);
	      tab(tabbing + 1);
	      fprintf(stdout, "%.*s\n", end - start, start);
	      start = end;
	      if (*start != '\0')
		start++;
	    }
	}
      else
	if (ptr->child != NULL)
	  printmenu(ptr->child, tabbing + 1);
    }
}


/************************************************************************
 *
 *  PrintMenu  --  call the printmenu proc above...  this proc is
 *	suitable for use in a procedure table, the other is not...
 *
 ************************************************************************/
void PrintMenu(me)
     MenuJet me;
{
  int tabbing;
  Menu *ptr;

  tabbing = 0;
  ptr = me->menu.rootMenu;

  printmenu(ptr, tabbing);
}
