/* $Id: libjwgc_debug.h,v 1.1.1.1 2006-03-10 15:32:45 ghudson Exp $ */

#ifndef _LIBJWGC_DEBUG_H_
#define _LIBJWGC_DEBUG_H_ 1

#include "libjwgc_types.h"


/* --------------------------------------------------------- */
/* JDebug.c                                                  */
/* Debugging support                                         */
/* --------------------------------------------------------- */
extern	int errno;

typedef enum dZones {
	dJWG,
	dParser,
	dJAB,
	dOutput,
	dEval,
	dPoll,
	dExecution,
	dVars,
	dMatch,
	dXML,
	dGPG,
	dNumZones
} dZone;

void	dclear();
void	dflagon(dZone zone);
void	dflagoff(dZone zone);
void	dprintf(dZone zone, const char *msgfmt, ...);
void	dprinttypes();
void	dparseflags(char *flags);
char	*dzoneitos(dZone zone);
dZone	dzonestoi(char *zone);


#endif /* _LIBJWGC_DEBUG_H_ */
