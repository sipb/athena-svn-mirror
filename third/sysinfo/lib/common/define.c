/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Definetion functions
 */
#include "defs.h"

/*
 * Master list of definetions
 */
DefineList_t		       *Definetions = NULL;

/*
 * Valid list of definetions
 * XXX 	It would be nice if only those names that are valid for
 * 	the OS we're run on are defined.
 */
static char *ValidDefineList[] = {
    DL_SYSMODEL, DL_KARCH, DL_CPU, DL_OBP, DL_VPD, 
    DL_NETTYPE, DL_CATEGORY, DL_PART, DL_TAPEINFO,
    DL_KERNEL, DL_SYSCONF, DL_SCSI_DTYPE, DL_CDSPEED,
    NULL
};

/*
 * Make sure Name is valid.
 */
int DefValid(Name)
    char		       *Name;
{
    register char	      **cpp;
    int				Valid = FALSE;

    for (cpp = ValidDefineList; !Valid && cpp && *cpp; ++cpp)
	if (EQ(Name, *cpp))
	    Valid = TRUE;

    return(Valid);
}

/*
 * Add a definetion to the master list.
 */
extern void DefAdd(Define, ListName)
    Define_t		       *Define;
    char		       *ListName;
{
    register DefineList_t      *ListPtr;
    register DefineList_t      *NewList = NULL;
    register Define_t	       *DefPtr;

    /*
     * Find the requested list
     */
    for (ListPtr = Definetions; ListPtr; ListPtr = ListPtr->Next)
	if (EQ(ListPtr->Name, ListName))
	    break;

    /*
     * Create a new list entry if the list doesn't exist already.
     */
    if (!ListPtr) {
	ListPtr = NewList = (DefineList_t *) xcalloc(1, sizeof(DefineList_t));
	NewList->Name = strdup(ListName);
    }

    /*
     * Add NewList to Definetions
     */
    if (!Definetions)
	Definetions = NewList;
    else if (NewList) {
	for (ListPtr = Definetions; ListPtr && ListPtr->Next;
	     ListPtr = ListPtr->Next);
	ListPtr->Next = NewList;
	ListPtr = NewList;
    }

    /*
     * Add Define to the requested list
     */
    if (!ListPtr->Defines)
	ListPtr->Defines = Define;
    else {
	for (DefPtr = ListPtr->Defines; DefPtr && DefPtr->Next; 
	     DefPtr = DefPtr->Next);
	DefPtr->Next = Define;
    }
}

/*
 * Get the Define list for ListName.
 */
extern Define_t *DefGetList(ListName)
    char		       *ListName;
{
    register DefineList_t      *Ptr;

    for (Ptr = Definetions; Ptr; Ptr = Ptr->Next)
	if (EQ(Ptr->Name, ListName))
	    return(Ptr->Defines);

    return((Define_t *) NULL);
}

/*
 * Destroy DefList
 */
extern int DefDestroy(DefList)
     Define_t		       *DefList;
{
    register Define_t	       *dp;
    register Define_t	       *Last;

    if (!DefList)
	return -1;

    for (dp = DefList; dp; ) {
	if (dp->KeyStr)		(void) free(dp->KeyStr);
	if (dp->ValStr1)	(void) free(dp->ValStr1);
	if (dp->ValStr2)	(void) free(dp->ValStr2);
	if (dp->ValStr3)	(void) free(dp->ValStr3);
	if (dp->ValStr4)	(void) free(dp->ValStr4);
	if (dp->ValStr5)	(void) free(dp->ValStr5);

	Last = dp;
	dp = dp->Next;
	(void) free(Last);
    }

    return 0;
}

/*
 * Find definetion with key KeyStr and/or KeyNum in list ListName.
 */
extern Define_t *DefGet(ListName, KeyStr, KeyNum, Opts)
    char		       *ListName;
    char		       *KeyStr;
    long			KeyNum;
    int				Opts;
{
    Define_t		       *List;
    register Define_t	       *Ptr;
    int				StrMatch = FALSE;
    int				NumMatch = FALSE;

    List = DefGetList(ListName);
    if (!List) {
	SImsg(SIM_DBG, "Invalid list name `%s'.", ListName);
	return((Define_t *) NULL);
    }

    for (Ptr = List; Ptr; Ptr = Ptr->Next) {
	if (KeyStr && Ptr->KeyStr)
	    if (FLAGS_ON(Opts, DO_REGEX)) {
		strtolower(KeyStr);
		strtolower(Ptr->KeyStr);
		if (REmatch(KeyStr, Ptr->KeyStr, NULL) > 0)
		    StrMatch = TRUE;
	    } else {
		if (EQ(KeyStr, Ptr->KeyStr))
		    StrMatch = TRUE;
	    }
	/*
	 * If KeyStr is NULL and the entry key is "-" this implies
	 * that we want to match againt none (NULL).
	 */
	if (!StrMatch)
	    if (!KeyStr && Ptr->KeyStr && EQ(Ptr->KeyStr, "-"))
		StrMatch = TRUE;
	if ((KeyNum >= 0) && (Ptr->KeyNum >= 0) && KeyNum == Ptr->KeyNum)
	    NumMatch = TRUE;
	if (KeyStr && (KeyNum >= 0) && StrMatch && NumMatch)
	    return(Ptr);
	if (KeyStr && StrMatch)
	    return(Ptr);
	if ((KeyNum >= 0) && NumMatch)
	    return(Ptr);
    }

    return((Define_t *) NULL);
}
