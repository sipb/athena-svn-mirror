/*
 * Copyright 1987 by MIT Student Information Processing Board
 *
 * For copyright information, see mit-sipb-copyright.h.
 */
#include "mit-sipb-copyright.h"
#include "ss_internal.h"
#include <sys/wait.h>

#ifdef	lint		/* "lint returns a value which is sometimes ignored" */
#define	DONT_USE(x)	x=x;
#else	!lint
#define	DONT_USE(x)	;
#endif	lint

static char twentyfive_spaces[26] = "                         ";
static char NL[2] = "\n";

ss_list_requests(argc, argv, sci_idx)
     int argc;
     char **argv;
     int sci_idx;
{
     register ss_request_entry *entry;
     register char **name;
     register int spacing;
     register ss_request_table **table;
     
     char buffer[BUFSIZ];
     FILE *output;
     int fd;
     union wait waitb;

     DONT_USE(argc);
     DONT_USE(argv);

     fd = ss_pager_create();
     output = fdopen(fd, "w");

     fprintf(output, "Available %s requests:\n\n",
	    ss_info(sci_idx)->subsystem_name);
     
     for (table = ss_info(sci_idx)->rqt_tables; *table != (ss_request_table *)NULL; table++) {
	  for (entry = (*table)->requests; entry->command_names != (char **)NULL; entry++) {
	       spacing = -2; /* no ", " for first one */
	       buffer[0] = '\0';
	       if (entry->flags & SS_OPT_DONT_LIST)
		    continue;
	       for (name = entry->command_names; *name != (char *)NULL; name++) {
		    register int len = strlen(*name);
		    strncat(buffer, *name, len);
		    spacing += len + 2;
		    if (*(name+1) != (char *)NULL)
			 strcat(buffer, ", ");
	       }
	       if (spacing > 23) {
		    strcat(buffer, NL);
		    fputs(buffer, output);
		    spacing = 0;
		    buffer[0] = '\0';
	       }
	       strncat(buffer, twentyfive_spaces, 25-spacing);
	       strcat(buffer, entry->info_string);
	       strcat(buffer, NL);
	       fputs(buffer, output);
	  }
     }
     fclose(output);
     wait(&waitb);
}
