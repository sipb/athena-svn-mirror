/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.3 $";
#endif

/*
 * Memory related functions.
 */

#include "defs.h"

/*
 * Divide and Round Up
 */
extern Large_t DivRndUp(Num, Div)
    Large_t	 		Num;
    Large_t	 		Div;
{
    Large_t			i;

    i = Num / Div;

    return((Num % Div) ? i+1 : i);
}

/*
 * Get string of the amount of physical memory
 */
extern char *GetMemoryStr(Amount)
    Large_t			Amount;
{
    static char			Buff[64];

    if (Amount > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%s", GetSizeStr(Amount, MBYTES));
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
    Large_t			Bytes;
    Large_t	 		Amount = 0;
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
	    Bytes = (Large_t)GETPAGESIZE() * (Large_t)physmem;
	    SImsg(SIM_DBG, "Physmem: Bytes = %.0f physmem = %d pagesize = %d", 
		  (float) Bytes, physmem, GETPAGESIZE());
	    Amount = DivRndUp(Bytes, (Large_t)MBYTES);
	}
    }

    if (kd)
	KVMclose(kd);

    return(GetMemoryStr(Amount));
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
