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
 *	$Id: motd.c,v 1.4 1991-01-01 14:00:21 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/motd.c,v 1.4 1991-01-01 14:00:21 lwvanels Exp $";
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

static void write_motd_times P((void));
static int gethm P((char *cp , long *hp , long *mp ));
static long parse_time P((char *buf ));

#undef P

static long expire_time = -1;   /* In seconds past Jan. 1, 1970 */
static long in_time = -1;

void
  check_motd_timeout()
{
  struct timeval now;
  int fd;
  char buf[128];
  char *p;

  if ((expire_time == 0) && (in_time == 0))
    return;

  if (expire_time == -1) {
    fd = open(MOTD_TIMEOUT_FILE,O_RDONLY,0);
    if (fd < 0) {
      log_error("check_motd_timeout: Couldn't open motd timeout file");
      expire_time = 0;
      in_time = 0;
      write_motd_times();
      return;
    }
    read(fd,buf,128);
    close(fd);
    expire_time = atol(buf);

    p = index(buf,'\n');
    if (p == NULL) {
      log_error("check_motd_timeout: improperly formatted timeout file");
      in_time = 0;
      return;
    }
    p = p+1;
    in_time = atol(p);

    strcpy(buf,"MOTD timeout set to: ");
    if (expire_time == 0)
      strcat(buf,"infinite.");
    else
      strcat(buf,ctime(&expire_time));
    log_status(buf);

    strcpy(buf,"  timein set to    : ");
    if (in_time == 0)
      strcat(buf,"Never.");
    else
      strcat(buf,ctime(&in_time));
    log_status(buf);
    
  }

  gettimeofday(&now,0);

  if ((now.tv_sec > in_time) && (in_time != 0)) {
    in_time = 0;
    write_motd_times();
    if (rename(MOTD_HOLD_FILE,MOTD_FILE) < 0) {
      log_error("check_motd_timeout:rename %m");
      return;
    }
    log_status("MOTD automatically invoked");
  }    

  if ((now.tv_sec > expire_time) && (expire_time != 0)) {
    fd = open(MOTD_FILE,O_TRUNC|O_CREAT, 0644);
    if (fd < 0)
      log_error("check_motd_timeout: Couldn't create/truncate motd file");
    close(fd);
    log_status("MOTD expired");
    expire_time = 0;
    write_motd_times();
    fd = open(MOTD_TIMEOUT_FILE,O_WRONLY|O_CREAT|O_TRUNC,0644);
    if (fd < 0) {
      log_error("check_motd_timeout: Couldn't open motd timeout file");
      return;
    }
    sprintf(buf,"0\n%ld\n",in_time);
    write(fd,buf,strlen(buf)+1);
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
  char msgbuf[BUF_SIZE];
  char *time;
  int which;

  expire_time = 0;
  in_time = 0;
  f = fopen(MOTD_FILE,"r+");
  if (f == NULL) {
    log_error("set_motd_timeout: Couldn't create/truncate motd file");
    return;
  }
  new_motd = fopen("/tmp/new_motd","w+");
  /* strip out the timeout line and write it to a new file */
  if (new_motd == NULL) { 
    log_error("change_motd_timeout: opening /tmp/new_motd: %m");
    return;
  }

/* Search for timeout and timein strings */
  while (fgets(line,BUF_SIZE,f) != NULL) {
    line[BUF_SIZE-1] = '\0';
    if (strncasecmp(line,"timeout:",8) == 0) {
      which = 1;
      time = &line[8];
    }
    else if (strncasecmp(line,"timein:",7) == 0) {
      which = 2;
      time = &line[7];
    }
    else {
      fputs(line,new_motd);
      continue;
    }

    if (which == 1)
      expire_time = parse_time(time);
    else
      in_time = parse_time(time);

  }
  fclose(f);
  fclose(new_motd);

  strcpy(msgbuf,"MOTD set to time out at: ");
  if (expire_time == 0)
    strcat(msgbuf,"Never");
  else
    strcat(msgbuf,ctime(&expire_time));
  strcat(msgbuf,"\n            start at   : ");
  if (in_time == 0)
    strcat(msgbuf,"Immediately");
  else
    strcat(msgbuf,ctime(&in_time));

  write_message_to_user(requester,msgbuf,0);
  
  if (in_time == 0) {
    if (rename("/tmp/new_motd",MOTD_FILE) != 0)
      log_error("change_motd_timeout: rename: %m");
  }
  else {
    expire_time = 1;
    check_motd_timeout();
    if (rename("/tmp/new_motd",MOTD_HOLD_FILE) != 0)
      log_error("change_motd_timeout: rename: %m");
  }  
  write_motd_times();
}

static void
  write_motd_times()
{
  FILE *motd_times;

  /* write new timeout to timeout file */
  motd_times = fopen(MOTD_TIMEOUT_FILE,"w+");
  if (motd_times == NULL) {
    log_error("write_motd_times: couldn't create/open timeout file: %m");
    return;
  }
  fprintf(motd_times,"%ld\n%ld\n",expire_time,in_time);
  fclose(motd_times);
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

static long 
  parse_time(buf)
char *buf;
{
  struct timeval tv;
  struct tm *now;
  long minutes, hours, diff;
  long when;
  char *p;

  when = -1;
  gettimeofday(&tv,0);
  now = localtime(&tv.tv_sec);
  
  p = index(buf,'\n');
  if (p != NULL) *p = '\0';

  while (*buf == ' ')
    buf++;

  if (*buf == '+') {
    buf++;
    if (!gethm(buf, &hours, &minutes))
      return(when);
    if (minutes < 0 || minutes > 59)
      return(when);
    when = tv.tv_sec + 60*((60*hours) + minutes);
  }
  else {
    if (!gethm(buf, &hours, &minutes))
      return(when);
    if (hours > 12)
      hours -= 12;
    if (hours == 12)
      hours = 0;
    if (hours < 0 || hours > 12 || minutes < 0 || minutes > 59)
      return(when);
    if (now->tm_hour > 12)
      now->tm_hour -= 12;      /* do am/pm bit */
    diff = 60*(hours - now->tm_hour) + minutes - now->tm_min;
    while (diff < 0)
      diff += 12 * 60;
    when = tv.tv_sec + 60*diff;
  }
  return(when);
}
