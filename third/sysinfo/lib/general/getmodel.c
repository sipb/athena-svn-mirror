/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
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
extern int GetModel(Query)
     MCSIquery_t	      *Query;
{
    extern PSI_t	        GetModelPSI[];
    char		       *Str;

    if (Query->Op == MCSIOP_CREATE) {
	if (Str = PSIquery(GetModelPSI)) {
	    Query->Out = (Opaque_t) strdup(Str);
	    Query->OutSize = strlen(Str);
	    return 0;
	}
    } else if (Query->Op == MCSIOP_DESTROY) {
	if (Query->Out && Query->OutSize)
	    (void) free(Query->Out);
	return 0;
    }

    return -1;
}

