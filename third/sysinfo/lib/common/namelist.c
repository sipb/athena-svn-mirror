/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
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
