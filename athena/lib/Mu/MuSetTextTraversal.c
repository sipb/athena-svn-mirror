/*
 * Copyright 1989 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * M.I.T. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability
 * of this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * MotifUtils:   Utilities for use with Motif and UIL
 * $Id: MuSetTextTraversal.c,v 1.2 1999-01-22 23:16:41 ghudson Exp $
 */

#include "Mu.h"

typedef struct link {
  Widget w;
  Widget subtree;
  struct link *next_in_all;
  struct link *next_in_list;
  struct link *previous_in_list;
} MuTraversalList;

static MuTraversalList *TraversalList = NULL;

void MuSetTextTraversal(twidgets, tnum, tsubtree)
     Widget twidgets[];
     int tnum;
     Widget tsubtree;
{
  int i;
  MuTraversalList *list;
  static XtTranslations translations = NULL;
  extern char *malloc();

  static void upfocus(), downfocus();
  char *strans =
"<Key>Up: activate() _Muupfocus()\n\
<Key>Return: activate() _Mudownfocus()\n\
<Key>Down: activate() _Mudownfocus()\n";

  static XtActionsRec actionTable[] = {
    {"_Muupfocus", (XtActionProc) upfocus},
    {"_Mudownfocus", (XtActionProc) downfocus},
  };

  if (translations == NULL) {
    XtAddActions(actionTable, XtNumber(actionTable));
    translations = XtParseTranslationTable(strans);
  }

  list = (MuTraversalList *)malloc(sizeof(MuTraversalList) * tnum);
  for(i=0; i<tnum; i++) {
      XtOverrideTranslations(twidgets[i], translations);
      list[i].w = twidgets[i];
      list[i].subtree = tsubtree;
      list[i].next_in_all = (i != tnum-1)?&list[i+1]:NULL;
      list[i].next_in_list = (i != tnum-1)?&list[i+1]:&list[0];
      list[i].previous_in_list = (i!=0)?&list[i-1]:&list[tnum-1];
  }

  list[tnum-1].next_in_all = TraversalList;
  TraversalList = list;
}

static MuTraversalList *find_entry(widget)
Widget widget;
{
    MuTraversalList *entry;
    
    entry = TraversalList;
    while ((entry != NULL) && (entry->w != widget))
	entry = entry->next_in_all;
    return(entry);
}

static MuTraversalList *get_prev_entry(entry)
MuTraversalList *entry;
{
    MuTraversalList *prev_entry;
    
    /* get previous entry */
    prev_entry = entry->previous_in_list;
    
    /* if insensitive, keep looking, but don't go into an infinite loop */
    while (!XtIsSensitive(prev_entry->w) && (prev_entry != entry))
	prev_entry = prev_entry->previous_in_list;
    return prev_entry;
}

static MuTraversalList *get_next_entry(entry)
MuTraversalList *entry;
{
    MuTraversalList *next_entry;
    
    /* get next entry */
    next_entry = entry->next_in_list;
    
    /* if insensitive, keep looking, but don't go into an infinite loop */
    while (!XtIsSensitive(next_entry->w) && (next_entry != entry))
	next_entry = next_entry->next_in_list;
    return next_entry;
}

static void upfocus(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
/*ARGSUSED*/
{
    MuTraversalList *entry;
    
    entry = find_entry(w);
    entry = get_prev_entry(entry);
    XtSetKeyboardFocus(entry->subtree, entry->w);
}

static void downfocus(w, event, params, num_params)
Widget w;
XEvent *event;
String *params;
Cardinal *num_params;
/*ARGSUSED*/
{
    MuTraversalList *entry;
    
    entry = find_entry(w);
    entry = get_next_entry(entry);
    XtSetKeyboardFocus(entry->subtree, entry->w);
}
 
