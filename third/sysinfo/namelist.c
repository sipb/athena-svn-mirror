/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: namelist.c,v 1.1.1.1 1996-10-07 20:16:51 ghudson Exp $";
#endif

/*
 * Name list routines.
 */

#include "defs.h"

extern namelist_t *NameListFind(List, Name)
    namelist_t		       *List;
    char		       *Name;
{
    register namelist_t	       *Ptr;

    for (Ptr = List; Ptr; Ptr = Ptr->nl_next) 
	if (EQ(Ptr->nl_name, Name))
	    return(Ptr);

    return((namelist_t *) NULL);
}

extern void NameListAdd(List, Name)
    namelist_t		      **List;
    char		       *Name;
{
    register namelist_t	       *Ptr;
    
    Ptr = (namelist_t *) xmalloc(sizeof(namelist_t));
    Ptr->nl_name = strdup(Name);
    Ptr->nl_next = (namelist_t *) NULL;

    if (List && *List)
	Ptr->nl_next = *List;

    *List = Ptr;
}

extern void NameListFree(List)
    namelist_t		       *List;
{
    register namelist_t	       *Ptr;
    register namelist_t	       *LastPtr;

    for (Ptr = List; Ptr; ) {
	if (Ptr->nl_name)
	    (void) free(Ptr->nl_name);
	LastPtr = Ptr;
	Ptr = Ptr->nl_next;
	(void) free(LastPtr);
    }
}
