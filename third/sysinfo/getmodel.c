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
 * Get system model name
 */

#include "defs.h"

/*
 * Read MODELFILE to get name of system model.
 */
char *GetModelFile()
{
    static char 		Buf[MAXPATHLEN];
    FILE 		       *fd;
    int 			Len;

    /*
     * Use model file if it exists.
     */
    if ((fd = fopen(MODELFILE, "r")) != NULL) {
	fgets(Buf, sizeof(Buf), fd);
	Len = strlen(Buf);
	if (Buf[Len-1] == '\n') 
	    Buf[Len-1] = C_NULL;
	return(Buf);
    }

    return(NULL);
}

/*
 * Run a command to get system model
 */
char *GetModelCmd()
{
#if	defined(MODEL_COMMAND)
    extern char 	       *ModelCommand[];

    return(RunCmds(ModelCommand, FALSE));
#else
    return((char *) NULL);
#endif	/* MODEL_COMMAND */
}

/*
 * Return the predefined symbol MODEL_NAME
 */
extern char *GetModelDef()
{
#if	defined(MODEL_NAME)
    return(MODEL_NAME);
#else	/* !MODEL_NAME */
    return((char *) NULL);
#endif	/* MODEL_NAME */
}

/*
 * Get system model
 */
extern char *GetModel()
{
    extern PSI_t	       GetModelPSI[];

    return(PSIquery(GetModelPSI));
}

