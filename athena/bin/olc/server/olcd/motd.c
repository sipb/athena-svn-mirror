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
 *	$Id: motd.c,v 1.19 1999-06-28 22:52:41 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: motd.c,v 1.19 1999-06-28 22:52:41 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include "olcd.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <time.h>

#ifdef HAVE_DISCUSS
#include <lumberjack.h>
#endif

static void write_motd_times (void);
static int gethm (char *cp , long *hp , long *mp );
static long parse_time (char *buf );

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
      log_error("check_motd_timeout: Couldn't open motd timeout file: %m");
      expire_time = 0;
      in_time = 0;
      write_motd_times();
      return;
    }
    read(fd,buf,128);
    close(fd);
    expire_time = atol(buf);

    p = strchr(buf,'\n');
    if (p == NULL) {
      log_error("check_motd_timeout: improperly formatted timeout file");
      in_time = 0;
      return;
    }
    p = p+1;
    in_time = atol(p);

    log_status("MOTD timeout set to: %s",
	       (expire_time == 0) ? "infinite." : ctime(&expire_time));
    log_status("     timein set to:  %s",
	       (expire_time == 0) ? "never."    : ctime(&in_time));
  }

  gettimeofday(&now,0);

  if ((now.tv_sec > in_time) && (in_time != 0)) {
    in_time = 0;
    write_motd_times();
    if (rename(MOTD_HOLD_FILE,MOTD_FILE) < 0) {
      log_error("check_motd_timeout: rename: %m");
      return;
    }
    log_status("MOTD automatically invoked");
  }    

  if ((now.tv_sec > expire_time) && (expire_time != 0)) {
    fd = open(MOTD_FILE,O_TRUNC|O_CREAT, 0664);
    if (fd < 0)
      log_error("check_motd_timeout: Couldn't create/truncate motd file: %m");
    close(fd);
    log_status("MOTD expired");
    expire_time = 0;
    write_motd_times();
  }
}

void
  set_motd_timeout(requester)
KNUCKLE *requester;
{
  FILE *f;		/* motd file */
  FILE *new_motd;	/* altered motd file fd */
  char line[BUF_SIZE];	/* buffer to read in to parse possible timeout */
  char msgbuf[BUF_SIZE];
  char tmp_motd_file[BUF_SIZE];
  char *time;
  struct stat statb;
  int fd;

  expire_time = 0;
  in_time = 0;
  f = fopen(MOTD_FILE,"r+");
  if (f == NULL) {
    log_error("set_motd_timeout: Couldn't create/truncate motd file: %m");
    return;
  }
  sprintf(tmp_motd_file,"%s.new",MOTD_FILE);
  new_motd = fopen(tmp_motd_file,"w+");
  /* strip out the timeout line and write it to a new file */
  if (new_motd == NULL) { 
    log_error("set_motd_timeout: opening tmp motd file: %m");
    return;
  }

/* Fix so it has correct permissions and group */
  if (fstat(fileno(f), &statb) == -1) {
    log_error("set_motd_timeout: stat'ing MOTD file: %m");
    return;
  }
  fchown(fileno(new_motd),-1,statb.st_gid);
  fchmod(fileno(new_motd),0664);

/* Search for timeout and timein strings */
  while (fgets(line,BUF_SIZE,f) != NULL) {
    line[BUF_SIZE-1] = '\0';
    if (strncmp(line,"timeout:",8) == 0) {
      time = &line[8];
      expire_time = parse_time(time);
      continue;
    }
    else if (strncmp(line,"timein:",7) == 0) {
      time = &line[7];
      in_time = parse_time(time);
      continue;
    }
    else {
      fputs(line,new_motd);
      continue;
    }
  }
  fclose(f);
  fclose(new_motd);

  strcpy(msgbuf,"MOTD set to time out at: ");
  if (expire_time == 0)
    strcat(msgbuf,"Never\n");
  else
    strcat(msgbuf,ctime(&expire_time));
  strcat(msgbuf,"            start at   : ");
  if (in_time == 0)
    strcat(msgbuf,"Immediately\n");
  else
    strcat(msgbuf,ctime(&in_time));

  write_message_to_user(requester,msgbuf,0);
  
  if (in_time == 0) {
    if (rename(tmp_motd_file,MOTD_FILE) != 0)
      log_error("change_motd_timeout: rename: %m");
  }
  else {
    fd = open(MOTD_FILE,O_TRUNC|O_CREAT, 0664);
    if (fd < 0)
      log_error("set_motd_timeout: Couldn't create/truncate motd file: %m");
    close(fd);
    check_motd_timeout();
    if (rename(tmp_motd_file,MOTD_HOLD_FILE) != 0)
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
  now = localtime((time_t *)&tv.tv_sec);
  
  p = strchr(buf,'\n');
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

/* Log the new MOTD.  If you don't have Discuss, it'll just silently
 * do nothing; in that case, you may want to add some other means of
 * logging the message here... 
 */
void
log_motd(username)
     char *username;
{
#ifdef HAVE_DISCUSS
  char newfile[NAME_SIZE];	/* New file name. */
  char ctrlfile[NAME_SIZE];	/* Control file name. */
  FILE *fp;			/* Stream for control file. */
  int pid;			/* Process ID for fork. */
  char msgbuf[BUF_SIZE];	/* Construct messages to be logged. */
  long time_now;		/* time now, in seconds (a unique number) */

  time_now = NOW;

  (void) sprintf(ctrlfile, "%s/ctrl%d", DONE_DIR, time_now);
  if (access(ctrlfile,F_OK) == 0) {
    /* Whups, processed that last done too fast... */
    time_now++;
    (void) sprintf(ctrlfile, "%s/ctrl%d", DONE_DIR, time_now);
  }
  (void) sprintf(newfile, "%s/log%d", DONE_DIR, time_now);

  sprintf(msgbuf,"/bin/cp %s %s",MOTD_FILE,newfile);
  system(msgbuf);

  fp = fopen(ctrlfile, "w");
  if (fp == NULL)
    {
      log_error("Can't create control file: %m");
      return;
    }

  fprintf(fp, "%s\nNew MOTD: %smotd\n%s\n", newfile, ctime(&time_now),
	  username);
  fclose(fp);
      
  pid = fork();
  if (pid == -1) 
    {
      log_error("Can't fork to dispose of log: %m");
      return;
    }
  else if (pid == 0) 
    {
      log_status("New motd logged");

      execl(LUMBERJACK_LOC, "lumberjack", 0);
      log_error("log_motd: cannot exec %s: %m",LUMBERJACK_LOC);
      _exit(4);
    }
#endif /* HAVE_DISCUSS */
  return;
}
