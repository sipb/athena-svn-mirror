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
 *	$Id: statistics.c,v 1.11 1991-11-06 15:46:27 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/statistics.c,v 1.11 1991-11-06 15:46:27 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <olcd.h>
#include <fcntl.h>
#include <sys/time.h>

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
dump_request_stats(file)
     char *file;
{
    FILE *fp;
    int i;
    long current_time;
    char time_buf[25];

    fp = fopen(file,"a");
    if (!fp)
	return;

    time(&current_time);
    format_time(time_buf,localtime(&start_time));
    fprintf(fp, "Daemon started at:  %s,\n",time_buf);
    format_time(time_buf,localtime(&current_time));
    fprintf(fp, "  stats dumped at:  %s.\n",time_buf);
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
dump_question_stats(file)
     char *file;
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

void
write_ask_stats(username,topic,machine,ask_by)
     char *username;
     char *topic;
     char *machine;
     char *ask_by;
{
  int fd;
  char buf[BUFSIZ];
  char ask_time[26];
  time_t now;

  if ((fd = open(ASK_STATS_FILE,O_APPEND|O_WRONLY,0600)) < 0) {
    if (errno != ENOENT) {
      sprintf(buf,"Error opening ask stats file %s: %%m", ASK_STATS_FILE);
      log_error(buf);
    }
    return;
  }

  now = time(0);
  strcpy(ask_time,ctime(&now));
  ask_time[24] = '\0';
  sprintf(buf,"%s %s %s %s %s\n",ask_time, username, topic, machine,
	  ask_by);
  write(fd,buf,strlen(buf));
  close(fd);
}

void
write_res_stats(q)
     QUESTION *q;
{
  int fd,i,res_uid;
  char buf[BUFSIZ];
  char ask_time[26],res_time[26];
  time_t now;

  if ((fd = open(RES_STATS_FILE,O_APPEND|O_WRONLY,0600)) < 0) {
    if (errno != ENOENT) {
      sprintf(buf,"Error opening res stats file %s: %%m", RES_STATS_FILE);
      log_error(buf);
    }
    return;
  }

  now = time(0);
  strcpy(res_time,ctime(&now));
  res_time[24] = '\0';
  strcpy(ask_time,ctime(&q->owner->timestamp));
  ask_time[24] = '\0';
  sprintf(buf,"%s %s %s %s %d %d %d %d %d\n", ask_time, res_time,
	  q->owner->user->username, q->owner->user->machine,
	  q->stats.n_crepl, q->stats.n_cmail, q->stats.n_urepl,
	  q->stats.time_to_fr, q->nseen);

  write(fd,buf,strlen(buf));

  for(i=0;i<q->nseen;i++) {
    sprintf(buf,"%d ",q->seen[i]);
    write(fd,buf,strlen(buf));
  }
  write(fd,"\n",1);
  if (q->owner->connected == NULL)
    res_uid = -1;
  else
    res_uid = q->owner->connected->user->uid;

  sprintf(buf,"%d\n",res_uid);
  write(fd,buf,strlen(buf));
  close(fd);
}
