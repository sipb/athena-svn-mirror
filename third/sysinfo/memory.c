/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: memory.c,v 1.1.1.1 1996-10-07 20:16:48 ghudson Exp $";
#endif


/*
 * Memory related functions.
 */

#include "defs.h"

/*
 * Divide and Round Up
 */
extern int DivRndUp(Num, Div)
    unsigned long 		Num;
    unsigned long 		Div;
{
    int 			i;

    i = Num / Div;

    return((Num % Div) ? i+1 : i);
}

/*
 * Get string of the amount of physical memory
 */
extern char *GetMemoryStr(Amount)
    u_long			Amount;
{
    static char			Buff[BUFSIZ];

    if (Amount > 0) {
	(void) sprintf(Buff, "%s", GetSizeStr(Amount, MBYTES));
	return(Buff);
    }

    return((char *) NULL);
}

#if	defined(PHYSMEM_SYM) && !defined(HAVE_PHYSMEM)
#	define HAVE_PHYSMEM	1
#endif	/* PHYSMEM_SYM */
#if	!defined(PHYSMEM_SYM)
#	define PHYSMEM_SYM	"_physmem"
#endif	/* PHYSMEM_SYM */
#if	!defined(GETPAGESIZE)
#	define GETPAGESIZE() 	getpagesize()
#endif	/* GETPAGESIZE */

/*
 * Common method of determining amount of physical memory in a
 * BSD Unix machine.
 *
 * Get memory by reading the variable "physmem" from the kernel
 * and the system page size.
 */ 
extern char *GetMemoryPhysmemSym()
{
#if	defined(HAVE_PHYSMEM)
    nlist_t		       *nlptr;
    u_long 			Bytes;
    u_long	 		Amount = 0;
    kvm_t		       *kd;
    int				physmem;

    if (kd = KVMopen()) {
	if ((nlptr = KVMnlist(kd, PHYSMEM_SYM, (nlist_t *)NULL, 0)) == NULL)
	    return((char *)NULL);

	if (CheckNlist(nlptr))
	    return((char *)NULL);

	if (KVMget(kd, nlptr->n_value, (char *)&physmem, 
		    sizeof(physmem), KDT_DATA) >= 0) {
	    /*
	     * Could use ctob() instead of "Page Size * Num Pages",
	     * but this is more portable.
	     */
	    Bytes = (unsigned long) (GETPAGESIZE() * physmem);
	    if (Debug)
		printf("Bytes = %d physmem = %d pagesize = %d\n", 
		       Bytes, physmem, GETPAGESIZE());
	    Amount = DivRndUp(Bytes, MBYTES);
	}
    }

    if (kd)
	KVMclose(kd);

    return(GetMemoryStr((u_long)Amount));
#else	/* !HAVE_PHYSMEM */
    return((char *) NULL);
#endif	/* HAVE_PHYSMEM */
}

/*
 * Get amount of memory on machine.
 */
extern char *GetMemory()
{
    extern PSI_t	       GetMemoryPSI[];
    static char		      *Str = NULL;

    if (Str)
	return(Str);

    return(Str = PSIquery(GetMemoryPSI));
}
