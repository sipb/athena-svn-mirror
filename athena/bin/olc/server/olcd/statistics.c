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
 *	$Id: statistics.c,v 1.14 1999-01-22 23:14:33 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: statistics.c,v 1.14 1999-01-22 23:14:33 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>


#include <fcntl.h>
#include <sys/time.h>
#include <olcd.h>
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
