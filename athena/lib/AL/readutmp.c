#include <sys/types.h>
#ifndef SYSV
#include <utmp.h>
#else
#include <utmpx.h>
#endif

#include <stdio.h>

char *types[] = { "NULL", "LVL", "BOOT", "OTIME", "NTIME", "INIT",
		    "LOGIN", "USER", "DEAD", "ACC" };

main(argc, argv)
     int argc;
     char **argv;
{
  struct utmpx *utmpx;
  struct utmp *utmp;

#ifdef UTMPX_FILE
  setutxent();
#endif
  setutent();

  printf("%8s  %4s  %11s  %5s  %5s  %3s  %s\n\n",
	 "user", "id", "line", "pid", "type", "len", "host");

  while((int)(utmp = getutent())
#ifdef UTMPX_FILE
	| (int)(utmpx = getutxent())
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
  endutent();

#ifdef UTMPX_FILE
  endutxent();
#endif
}
