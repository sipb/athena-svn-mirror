/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains functions for doing motd timeouts
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1988,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/motd.c,v $
 *	$Id: motd.c,v 1.2 1990-12-12 15:18:56 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/motd.c,v 1.2 1990-12-12 15:18:56 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

#include <olcd.h>

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/file.h>

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

static int gethm P((char *cp , long *hp , long *mp ));

#undef P

static long expire_time = -1;   /* In seconds past Jan. 1, 1970 */

void
  check_motd_timeout()
{
  struct timeval now;
  int fd;
  char buf[128];

  if (expire_time == 0)
    return;

  if (expire_time == -1) {
    fd = open(MOTD_TIMEOUT_FILE,O_RDONLY,0);
    if (fd < 0) {
      log_error("check_motd_timeout: Couldn't open motd timeout file");
      expire_time = 0;
      return;
    }
    read(fd,buf,128);
    close(fd);
    expire_time = atol(buf);
    strcpy(buf,"MOTD timeout set to: ");
    if (expire_time == 0)
      strcat(buf,"infinite.");
    else
      strcat(buf,ctime(&expire_time));
    log_status(buf);
  }

  gettimeofday(&now,0);
  if (now.tv_sec > expire_time) {
    fd = open(MOTD_FILE,O_TRUNC|O_CREAT, 0644);
    if (fd < 0)
      log_error("check_motd_timeout: Couldn't create/truncate motd file");
    close(fd);
    log_status("MOTD expired");
    expire_time = 0;
    fd = open(MOTD_TIMEOUT_FILE,O_WRONLY|O_CREAT|O_TRUNC,0644);
    if (fd < 0) {
      log_error("check_motd_timeout: Couldn't open motd timeout file");
      expire_time = 0;
      return;
    }
    write(fd,"0\n",2);
    close(fd);
  }
}

void
  set_motd_timeout(requester)
KNUCKLE *requester;
{
  FILE *f;		/* motd file */
  FILE *new_motd;		/* altered motd file fd */
  char line[BUF_SIZE];	/* buffer to read in to parse possible timeout */
  char *p;
  struct timeval tv;
  struct tm *now;
  long minutes, hours, diff;
  char msgbuf[BUF_SIZE];

  expire_time = 0;
  f = fopen(MOTD_FILE,"r+");
  if (f == NULL) {
    log_error("set_motd_timeout: Couldn't create/truncate motd file");
    return;
  }
  fgets(line,BUF_SIZE,f);
  if(strncasecmp(line,"timeout:",8) != 0) {
    fclose(f);
    return;
  }
  line[BUF_SIZE-1] = '\0';

/* parse the timeout time */
  gettimeofday(&tv,0);
  now = localtime(&tv.tv_sec);

  p = index(line,'\n');
  if (p != NULL) *p = '\0';
  p = &line[8];  /* start right after the colon */

  while (*p == ' ')
    p++;

  if (*p == '+') {
    p++;
    if (!gethm(p, &hours, &minutes))
      goto done;
    if (minutes < 0 || minutes > 59)
      goto done;
    expire_time = tv.tv_sec + 60*((60*hours) + minutes);
  }
  else {
    if (!gethm(p, &hours, &minutes))
      goto done;
    if (hours > 12)
      hours -= 12;
    if (hours == 12)
      hours = 0;
    if (hours < 0 || hours > 12 || minutes < 0 || minutes > 59)
      goto done;
    if (now->tm_hour > 12)
      now->tm_hour -= 12;      /* do am/pm bit */
    diff = 60*(hours - now->tm_hour) + minutes - now->tm_min;
    while (diff < 0)
      diff += 12 * 60;
    expire_time = tv.tv_sec + 60*diff;
  }

 done:
  strcpy(msgbuf,"MOTD set to time out at: ");
  if (expire_time == 0)
    strcat(msgbuf,"Never");
  else
    strcat(msgbuf,ctime(&expire_time));
  write_message_to_user(requester,msgbuf,0);

  /* strip out the timeout line and write it to a new file */
  new_motd = fopen("/tmp/new_motd","w+");
  if (new_motd == NULL)
    log_error("change_motd_timeout: opening /tmp/new_motd: %%m");
  else {
    while (fgets(line,BUF_SIZE,f) != NULL)
      fputs(line,new_motd);
    fclose(new_motd);
  }
  fclose(f);

  if (rename("/tmp/new_motd",MOTD_FILE) != 0)
    log_error("change_motd_timeout: rename: %%m");

  /* write new timeout to timeout file */
  new_motd = fopen(MOTD_TIMEOUT_FILE,"w+");
  if (new_motd == NULL) {
    log_error("change_motd_timeout: couldn't create/open timeout file: %%m");
    return;
  }
  fprintf(new_motd,"%ld\n",expire_time);
  fclose(new_motd);
}

static int
  gethm(cp, hp, mp)
char *cp;
long *hp, *mp;
{
  char c;
  long tod;
  
  tod = 0;
  while ((c = *cp++) != '\0') {
    if (!isdigit(c))
      continue;
    tod = tod * 10 + (c - '0');
  }
  *hp = tod / 100;
  *mp = tod % 100;
  return(1);
}
