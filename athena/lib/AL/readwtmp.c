#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifndef SYSV
#include <utmp.h>
#else
#include <utmpx.h>
#endif

#include <stdio.h>

int wtfd = -1;
struct utmp gwtmp;

int setwtent(void)
{
  if (wtfd == -1)
    wtfd = open(WTMP_FILE, O_RDONLY, 0);

  if (wtfd != -1)
    lseek(wtfd, 0, SEEK_END);

  return 0;
}

struct utmp *getwtent(void)
{
  if (wtfd == -1)
    setwtent();

  if (wtfd == -1)
    return NULL;

  if (-1 == lseek(wtfd, -sizeof(gwtmp), SEEK_CUR))
    return NULL;
  read(wtfd, &gwtmp, sizeof(gwtmp));
  lseek(wtfd, -sizeof(gwtmp), SEEK_CUR);

  return &gwtmp;
}

int endwtent(void)
{
  if (wtfd != -1)
    close(wtfd);

  return 0;
}

#ifdef WTMPX_FILE

int wtxfd = -1;
struct utmpx gwtxmp;

int setwtxent(void)
{
  if (wtxfd == -1)
    wtxfd = open(WTMPX_FILE, O_RDONLY, 0);

  if (wtxfd != -1)
    lseek(wtxfd, 0, SEEK_END);

  return 0;
}

struct utmpx *getwtxent(void)
{
  if (wtxfd == -1)
    setwtxent();

  if (wtxfd == -1)
    return NULL;

  if (-1 == lseek(wtxfd, -sizeof(gwtxmp), SEEK_CUR))
    return NULL;
  read(wtxfd, &gwtxmp, sizeof(gwtxmp));
  lseek(wtxfd, -sizeof(gwtxmp), SEEK_CUR);

  return &gwtxmp;
}

int endwtxent(void)
{
  if (wtxfd != -1)
    close(wtxfd);

  return 0;
}

#endif

char *types[] = { "NULL", "LVL", "BOOT", "OTIME", "NTIME", "INIT",
		    "LOGIN", "USER", "DEAD", "ACC" };

main(argc, argv)
     int argc;
     char **argv;
{
  struct utmpx *utmpx;
  struct utmp *utmp;

#ifdef UTMPX_FILE
  setwtxent();
#endif
  setwtent();

  printf("%8s  %4s  %11s  %5s  %5s  %3s  %s\n\n",
	 "user", "id", "line", "pid", "type", "len", "host");

  while((int)(utmp = getwtent())
#ifdef UTMPX_FILE
	| (int)(utmpx = getwtxent())
#endif
	)
    {
#ifdef UTMPX_FILE
      if (utmpx)
	printf("%8s  %4.4s  %11s  %5d  %5s  %3i  %s\n",
	       utmpx->ut_user,
	       utmpx->ut_id,
	       utmpx->ut_line,
	       utmpx->ut_pid,
	       types[utmpx->ut_type],
	       utmpx->ut_syslen,
	       utmpx->ut_host);
#endif
      if (utmp)
	printf("%8s  %4.4s  %11s  %5d  %5s       %s\n",
	       utmp->ut_user,
	       utmp->ut_id,
	       utmp->ut_line,
	       utmp->ut_pid,
	       types[utmp->ut_type],
#ifdef _AIX
	       utmp->ut_host
#else
	       ""
#endif
	       );
    }
  endwtent();

#ifdef UTMPX_FILE
  endwtxent();
#endif
}
