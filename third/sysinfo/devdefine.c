/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: devdefine.c,v 1.1.1.1 1996-10-07 20:16:49 ghudson Exp $";
#endif

/*
 * Device Definetion functions
 */
#include "defs.h"

/*
 * Master list of definetions
 */
DevDefine_t		       *DevDefinetions = NULL;

/*
 * Add a definetion to the master list.
 */
extern void DevDefAdd(DevDefine)
    DevDefine_t		       *DevDefine;
{
    static DevDefine_t	       *Last = NULL;

    /*
     * Add Define
     */
    if (!DevDefinetions)
	Last = DevDefinetions = DevDefine;
    else {
	Last->Next = DevDefine;
	Last = DevDefine;
    }
}

/*
 * Get device data tab entry having a name of "Name" and/or Type + Ident.
 */
extern DevDefine_t *DevDefGet(Name, Type, Ident)
    char 		       *Name;
    int				Type;
    int				Ident;
{
    register DevDefine_t       *Ptr;
    register char	      **cpp;
    register int 		i;
    int				NameMatch = FALSE;
    int				IdentMatch = FALSE;

    if (!Name && !Ident)
	return((DevDefine_t *) NULL);

    for (Ptr = DevDefinetions; Ptr; Ptr = Ptr->Next) {
	if (Name && Ptr->Name) {
	    if (FLAGS_ON(Ptr->Flags, DDT_LENCMP)) {
		if (EQN(Name, Ptr->Name, strlen(Ptr->Name)))
		    NameMatch = TRUE;
	    } else {
		if (EQ(Name, Ptr->Name)) 
		    NameMatch = TRUE;
	    }
	}
	if (Name && !NameMatch) {
	    /* Check name aliases */
	    for (cpp = Ptr->Aliases; !NameMatch && cpp && *cpp; ++cpp)
		if (EQ(Name, *cpp))
		    NameMatch = TRUE;
	}

	if (Type && Ident)
	    if (Ptr->Type == Type && Ptr->Ident == Ident)
		IdentMatch = TRUE;
	/*
	 * If we're suppose to match everything and it does...
	 */
	if ((Name && Type && Ident) && (NameMatch && IdentMatch))
	    return(Ptr);
	/* Just Name checking and it does match */
	else if (Name && NameMatch)
	    return(Ptr);
	/* Just Ident checking and it does match */
	else if (Type && Ident && IdentMatch)
	    return(Ptr);
    }

    return((DevDefine_t *) NULL);
}
