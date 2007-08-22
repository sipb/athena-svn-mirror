/*
 * Various minor routines...
 *
 * Copyright 1987, 1988, 1989 by MIT
 *
 * For copyright information, see mit-sipb-copyright.h.
 */

#include "mit-sipb-copyright.h"
#include <stdio.h>
#include "ss_internal.h"

/*
 * ss_self_identify -- assigned by default to the "." request
 */
void
ss_self_identify(argc, argv, sci_idx)
	int argc, sci_idx;
	char **argv;
{
     register ss_data *info = ss_info(sci_idx);
     printf("%s version %s\n", info->subsystem_name,
	    info->subsystem_version);
}

/*
 * ss_subsystem_name -- print name of subsystem
 */
void
ss_subsystem_name(argc, argv, sci_idx)
	int argc, sci_idx;
	char **argv;
{
     printf("%s\n", ss_info(sci_idx)->subsystem_name);
}

/*
 * ss_subsystem_version -- print version of subsystem
 */
void
ss_subsystem_version(argc, argv, sci_idx)
	int argc, sci_idx;
	char **argv;
{
     printf("%s\n", ss_info(sci_idx)->subsystem_version);
}

/*
 * ss_unimplemented -- routine not implemented (should be
 * set up as (dont_list,dont_summarize))
 */
void
ss_unimplemented(argc, argv, sci_idx)
	int argc, sci_idx;
	char **argv;
{
     ss_perror(sci_idx, SS_ET_UNIMPLEMENTED, "");
}
