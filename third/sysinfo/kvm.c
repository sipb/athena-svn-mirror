/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */
#ifndef lint
static char *RCSid = "$Id: kvm.c,v 1.1.1.1 1996-10-07 20:16:50 ghudson Exp $";
#endif

/*
 * Frontend functions for kvm_*() functions
 *
 * It is assumed we HAVE_NLIST if we HAVE_KVM.
 */

#include <stdio.h>
#include "defs.h"

#if	defined(HAVE_KVM)

#include "defs.h"
#include <fcntl.h>

/*
 * Perform a kvm_close().  Really just here to be compatible.
 */
extern void KVMclose(kd)
    kvm_t 		       *kd;
{
    if (kd)
	(void) kvm_close(kd);
}

/*
 * Perform a kvm_open().
 */
extern kvm_t *KVMopen()
{
    kvm_t 		       *kd = NULL;
    extern char 	       *ProgramName;

    if ((kd = kvm_open((char *)NULL, (char *)NULL, (char *)NULL, O_RDONLY,
		       ProgramName)) == NULL) {
	if (Debug) Error("kvm_open failed: %s.", SYSERR);
	return((kvm_t *) NULL);
    }

    return(kd);
}

#if	defined(HAVE_NLIST) || defined(HAVE_KNLIST)

static nlist_t NListTab[] = {
    { 0 },
    { 0 },
};

/*
 * Perform an nlist on "kd" looking for "Symbol" or "NameList".
 * Return the a pointer to the found nlist structure.
 * On failure, return NULL and KVMclos(kd).
 */
extern nlist_t *KVMnlist(kd, Symbol, NameList, NumNameList)
    kvm_t		       *kd;
    char		       *Symbol;
    nlist_t		       *NameList;
    int				NumNameList;
    /*ARGSUSED*/
{
    nlist_t		       *NLPtr;
    nlist_t		       *TabPtr;
    char		       *cp;
    int				Status;
    int				NumEle;

    if (NameList) {
	NLPtr = NameList;
	TabPtr = NameList;
	NumEle = NumNameList;
    } else {
	TabPtr = NListTab;
	NLPtr = &NListTab[0];
	NumEle = 1;
	memset((void *) NLPtr, 0, sizeof(nlist_t));
	GetNlNamePtr(NLPtr) = Symbol;
    }

#if	defined(COFF) || defined(ELF)
    for (NLPtr = TabPtr; GetNlNamePtr(NLPtr); ++NLPtr) {
	cp = GetNlNamePtr(NLPtr);
	/*
	 * Skip over initial '_'
	 */
	if (*cp == '_')
	    GetNlNamePtr(NLPtr) = cp + 1;
    }
#endif

#if	defined(HAVE_KNLIST)
    Status = knlist(TabPtr, NumEle, sizeof(nlist_t));
#else	/* !HAVE_KNLIST */
    Status = kvm_nlist(kd, TabPtr);
#endif	/* HAVE_KNLIST */
    if (Status == -1) {
	cp = GetNlNamePtr(NLPtr);
	if (Debug) Error("kvm_nlist name \"%s\" failed (status = %d): %s.", 
			 (cp) ? cp : "(unknown)", Status, SYSERR);
	KVMclose(kd);
	return((nlist_t *) NULL);
    }

    return(&TabPtr[0]);
}
#endif	/* HAVE_NLIST || HAVE_KNLIST */

/*
 * Perform a kvm_read().
 *
 * If DataType==KDT_STRING, read 1 byte at a time until '\0' or
 * NumBytes is reached. This is necessary in order to avoid
 * reading invalid memory pages from the kernel which can lead
 * to system crashes.
 *
 * If DataType==KDT_DATA then we read NumBytes of data into
 * Buf all at once.
 */
extern int KVMget(kd, Addr, Buf, NumBytes, DataType)
    kvm_t 		       *kd;
    OFF_T_TYPE	 		Addr;
    char 		       *Buf;
    size_t 		        NumBytes;
    int				DataType;
{
    char		       *Ptr;

    if (!kd)
	return(-1);

    switch (DataType) {
    case KDT_STRING:
	Ptr = Buf;
	do {
	    if (kvm_read(kd, Addr++, Ptr, 1) != 1) {
		if (Debug) Error("kvm_read failed prematurely: %s.", SYSERR);
		return(-1);
	    }
	} while (Ptr < &Buf[NumBytes-1] && *Ptr++);
	*Ptr = C_NULL;
	break;

    case KDT_DATA:
	if (kvm_read(kd, Addr, Buf, NumBytes) != NumBytes) {
	    if (Debug) Error("kvm_read failed (amount=%d): %s.", 
			     NumBytes, SYSERR);
	    return(-1);
	}
	break;

    default:
	if (Debug) Error("Unknown Kernel Data Type: %d.", DataType);
	return(-1);
    }

    return(0);
}

/*
 * Check to see if PtrNL is valid.
 */
extern int _CheckNlist(PtrNL)
    nlist_t		       *PtrNL;
{
    char		       *cp;

    if (!PtrNL)
	return(-1);

    /*
     * Should use n_type, but that's not set
     * correctly on some OS's.
     */
    if (!PtrNL->n_value) {
	cp = GetNlNamePtr(PtrNL);
	if (Debug) Error("Kernel symbol \"%s\" not found.", 
			 (cp) ? cp : "(unknown)");
	return(-1);
    }

    return(0);
}

#endif /* HAVE_KVM */
