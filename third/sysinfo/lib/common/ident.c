/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Ident functions
 */

#include "defs.h"

/*
 * Create a new Ident
 */
extern Ident_t *IdentCreate(Old)
     Ident_t		       *Old;
{
    Ident_t		       *New;

    New = (Ident_t *) xcalloc(1, sizeof(Ident_t));

    if (Old) {
	New->Type 	= Old->Type;
	New->Length	= Old->Length;
	New->Identifier = (u_char *) xmalloc(Old->Length);
	(void) memcpy(New->Identifier, Old->Identifier, Old->Length);
    }

    return New;
}

/*
 * Check to see if Ident1 matches Ident2.
 */
extern int IdentMatch(Ident1, Ident2)
     Ident_t		       *Ident1;
     Ident_t		       *Ident2;
{
    if (!Ident1 || !Ident2)
	return FALSE;

    if (Ident1->Length != Ident2->Length)
	return FALSE;

    if (memcmp(Ident1->Identifier, Ident2->Identifier, Ident1->Length) == 0)
	return TRUE;
    else
	return FALSE;
}

/*
 * Return an ASCII printable version of the Identifier
 */
extern char *IdentString(Ident)
     Ident_t		       *Ident;
{
    static char			Buff[1024];
    register int		i;
    register int		b;

    if (!Ident || Ident->Length <= 0)
	return (char *) NULL;

    if (Ident->Type == IDT_ASCII)
	return (char *) Ident->Identifier;

    (void) memset(Buff, CNULL, sizeof(Buff));
    for (i = 0, b = 0; i < Ident->Length && b < sizeof(Buff)-2; ++i) {
	(void) sprintf(&Buff[b], "%02x", (u_char) Ident->Identifier[i]);
	b += 2;
    }

    return Buff;
}
