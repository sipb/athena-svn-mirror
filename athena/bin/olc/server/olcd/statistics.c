/*
 * This file is part of the OLC On-Line Consulting System.  It contains
 * functions for dumping statistics about the server and what it has
 * done already.
 *
 *      Chris VanHaren
 *      MIT Project Athena
 *
 *      Copyright (c) 1990 by the Massachusetts Institute of Technology
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/statistics.c,v $
 *      $Author: vanharen $
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/statistics.c,v 1.3 1990-02-13 17:17:00 vanharen Exp $";
#endif

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
  int i, j, k;
  long current_time;
  struct tm *time_info;

  fp = fopen(file,"a");

  time(&current_time);
  fprintf(fp, "Daemon started at:  %s,\n",
	  format_time(localtime(&start_time)));
  fprintf(fp, "  stats dumped at:  %s.\n",
	  format_time(localtime(&current_time)));
  fprintf(fp, "%d requests, processed in %d seconds.\n",
	  request_count, current_time - start_time);

  for (i = 0; Proc_List[i].proc_code != UNKNOWN_REQUEST; i++)
    {
      fprintf(fp, "%s:", Proc_List[i].description);
      j = 25 - strlen(Proc_List[i].description);
      for (k = 0; k < j; k++)
	fprintf(fp, " ");
      fprintf(fp, "%d\n", request_counts[i]);
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
  int i, j, k;

  fp = fopen(file,"a");
  
  fprintf(fp, "Daemon has been up for %d seconds.\n",
	  time(0) - start_time);
  fprintf(fp, "No other statistics generated yet.\n");
  fprintf(fp, "-------------------------\n");
  fclose(fp);
}
