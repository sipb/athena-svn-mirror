/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/lib/Menu.h,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include "Jets.h"
#include "hash.h"

extern JetClass menuJetClass;
extern JetClass menuBarJetClass;
extern void PrintMenu();
extern int loadNewMenus();
extern void computeMenuSize();
extern void computeRootMenuSize();
extern void computeAllMenuSizes();

typedef struct {int littlefoo;} MenuClassPart;

typedef struct _MenuClassRec {
  CoreClassPart		core_class;
  MenuClassPart	menu_class;
} MenuClassRec;

extern MenuClassRec menuClassRec;

#define VERTICAL 0
#define HORIZONTAL 1

#define NORMAL 0
#define HELP 1

#define CLOSED 0
#define SELECTED 1
#define OPENED 2

typedef struct _Menu {
  /*
   * Information to be filled in by parent/runtime activities
   */
  struct _Menu *parent;
  struct _Menu *sibling;
  int label_x, label_y;
  int pane_x, pane_y;
  Window menuPane;
  int x, y;
  int state;			/* CLOSED, SELECTED, OPENED */

  /*
   * Self and child-dependent variables - known or computed at
   * initialization time
   */
  int machtype;
  Boolean verify;
  struct _Menu *child;
  int weight;
  char *title;
  XjCallback *activateProc;
  int paneType;			/* one of NORMAL, HELP */
  int orientation;		/* one of HORIZONTAL, VERTICAL */
  int label_width, label_height;
  int pane_width, pane_height;
  int pane_open_x;
} Menu;

#define MAXMENUSPERTYPE 10
#define MAXPARENTS 10
#define MAXCHILDREN 5

typedef struct _MenuU {
  int orientation;
  XrmQuark children[MAXCHILDREN];	/* possible types of children */
  Menu *m;				/* menu entry if this exists */
} MenuU;

typedef struct _ItemU {
  char *help;
  XjCallback *activateProc;
  int machtype;
  Boolean verify;
} ItemU;

#define ItemITEM 0
#define MenuITEM 1
#define HelpITEM 2
#define TitleITEM 2
#define SeparatorITEM 2

#define titleFLAG 1<<0
#define parentsFLAG 1<<1
#define activateFLAG 1<<2
#define orientationFLAG 1<<3
#define childrenFLAG 1<<4
#define verifyFLAG 1<<5

typedef struct _Item {
  int type;
  int flags;
  char *title;
  XrmQuark name;
  XrmQuark parents[MAXPARENTS];		/* possible types of parents */
  int weight[MAXPARENTS];		/* weight per-parent */
  struct _Item *next;
  union _u {
    MenuU m;
    ItemU i;
  } u;
} Item;

typedef struct _TypeDef {
  XrmQuark type;
  Item *menus[MAXMENUSPERTYPE];
} TypeDef;

typedef struct {
  XjCallbackProc activateProc;
  Boolean showHelp;
  int sticky;
  XFontStruct *font;
  GC gc, invert_gc, background_gc, dim_gc, dash_gc, dot_gc;
  int foreground, background;
  Boolean reverseVideo;
  char *items, *file, *fallback;
  char *userItems, *userFile;
  Menu *rootMenu;
  Menu *deepestOpened;
  int buttonDown;
  Boolean inside;
  Boolean same;
  int hMenuPadding;
  int vMenuPadding;
  Boolean screenWidth;
  XjPixmap *helpPixmap,
           *submenuPixmap,
  	   *grey;
  struct hash *Names, *Types;
  Item *firstMenu;
  Item *firstItem;
  Boolean rude;
  Boolean grabbed;
  Boolean autoRaise;
  int scrollx;
  Menu *responsibleParent;
  XjCallback *verifyProc;
  Boolean verify;
} MenuPart;

typedef struct _MenuRec {
  CorePart	core;
  MenuPart	menu;
} MenuRec;

typedef struct _MenuRec *MenuJet;
typedef struct _MenuClassRec *MenuJetClass;

#define XjCStartItems "StartItems" /* backwards compatibility... */
#define XjNstartItems "startItems"
#define XjCItems "Items"
#define XjNitems "items"
#define XjCFile "File"
#define XjNfile "file"
#define XjCFallback "FallbackFile"
#define XjNfallback "fallbackFile"
#define XjCUserItems "UserItems"
#define XjNuserItems "userItems"
#define XjCUserFile "UserFile"
#define XjNuserFile "userFile"
#define XjCMenuPadding "MenuPadding"
#define XjNhMenuPadding "hMenuPadding"
#define XjNvMenuPadding "vMenuPadding"
#define XjCScreenWidth "ScreenWidth"
#define XjNscreenWidth "screenWidth"
#define XjNhelpPixmap "helpPixmap"
#define XjNsubmenuPixmap "submenuPixmap"
#define XjCShowHelp "ShowHelp"
#define XjNshowHelp "showHelp"
#define XjNsticky "sticky"
#define XjCSticky "Sticky"
#define XjCGrey "Grey"
#define XjNgrey "grey"
#define XjCRude "Rude"
#define XjNrude "rude"
#define XjCVerifyProc "VerifyProc"
#define XjNverifyProc "verifyProc"
#define XjCVerify "Verify"
#define XjNverify "verify"
#define XjCAutoRaise "AutoRaise"
#define XjNautoRaise "autoRaise"

typedef struct _MenuInfo
{
  char *null;
  MenuJet menubar;
  Menu *menu;
} MenuInfo;
