/*
 * This file is part of the OLC On-Line Consulting System.  It contains
 * functions for dumping statistics about the server and what it has
 * done already.
 *
 *      Chris VanHaren
 *      MIT Project Athena
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/statistics.c,v $
 *	$Id: statistics.c,v 1.7 1990-07-16 08:31:00 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/statistics.c,v 1.7 1990-07-16 08:31:00 lwvanels Exp $";
#endif

#include <mit-copyright.h>

#include <olc/olc.h>
#include <olcd.h>

/*
 * Function:	dump_request_stats()  dumps info about the number of
 *			requests received, broken down by types.  Also
 *			prints the number of seconds the daemon was up.
 * Arguments:	file:	name of a file to dump to.
 * Returns:	Nothing (void).
 * Notes:
 *		relies on global variables...
 *		start_time:  time (in seconds) server was started.
 *		request_count:  total number of requests.
 *		request_counts[x]:  number of requests of type "x".
 */

void
#ifdef __STDC__
dump_request_stats(const char *file)
#else
dump_request_stats(file)
     char *file;
#endif
{
    FILE *fp;
    int i;
    long current_time;

    fp = fopen(file,"a");
    if (!fp)
	return;

    time(&current_time);
    fprintf(fp, "Daemon started at:  %s,\n",
	    format_time(localtime(&start_time)));
    fprintf(fp, "  stats dumped at:  %s.\n",
	    format_time(localtime(&current_time)));
    fprintf(fp, "%d requests, processed in %d seconds.\n",
	    request_count, current_time - start_time);

    for (i = 0; Proc_List[i].proc_code != UNKNOWN_REQUEST; i++) {
	char *desc = Proc_List[i].description;
	fprintf (fp, "%s:%*s %d\n", desc, 24 - strlen (desc), "",
		 request_counts[i]);
    }

    fprintf(fp, "---------------------------\n");
    fclose(fp);
}



/*
 * Function:	dump_question_stats()  dumps info about questions.  Not
 *			yet implemented.
 * Arguments:	file:	name of a file to dump to.
 * Returns:	Nothing (void).
 * Notes:
 */

void
#ifdef __STDC__
dump_question_stats(const char *file)
#else
dump_question_stats(file)
     char *file;
#endif
{
    FILE *fp;

    fp = fopen(file,"a");
    if (!fp)
	return;

    fprintf(fp, "Daemon has been up for %d seconds.\n",
	    time(0) - start_time);
    fprintf(fp, "No other statistics generated yet.\n");
    fprintf(fp, "-------------------------\n");
    fclose(fp);
}
