/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: define.c,v 1.1.1.1 1996-10-07 20:16:49 ghudson Exp $";
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
    DL_KERNEL, DL_SYSCONF,
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
	if (Debug) Error("Invalid list name `%s'.", ListName);
	return((Define_t *) NULL);
    }

    for (Ptr = List; Ptr; Ptr = Ptr->Next) {
	if (KeyStr && Ptr->KeyStr)
	    if (FLAGS_ON(Opts, DO_REGEX)) {
		strtolower(KeyStr);
		strtolower(Ptr->KeyStr);
		if (REMatch(KeyStr, Ptr->KeyStr) > 0)
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
