/* 
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/lib/rk_util.c,v 1.2 1990-07-03 14:58:39 qjb Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/lib/rk_util.c,v $
 * $Author: qjb $
 *
 * This routine contains internal routines for general use by the rkinit
 * library and server.  
 *
 * See the comment at the top of rk_lib.c for a description of the naming
 * conventions used within the rkinit library.
 */

#if !defined(lint) && !defined(SABER)
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/lib/rk_util.c,v 1.2 1990-07-03 14:58:39 qjb Exp $";
#endif lint || SABER

#include <stdio.h>
#include <sys/types.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>

#ifdef DEBUG
#include <syslog.h>
#endif

#include <rkinit.h>
#include <rkinit_private.h>
#include <rkinit_err.h>

#define RKINIT_TIMEOUTVAL 60

static char errbuf[BUFSIZ];
static jmp_buf timeout_env = NULL;

#ifdef DEBUG
static int _rkinit_server_ = FALSE;

void rki_dmsg(string)
  char *string;
{
    if (_rkinit_server_)
	syslog(LOG_NOTICE, string);
    else
	printf("%s\n", string);
}	

void rki_i_am_server()
{
    _rkinit_server_ = TRUE;
}
#else /* DEBUG */
void rki_dmsg(string)
  char *string;
{
    return;
}

#endif /* DEBUG */

char *rki_mt_to_string(mt)
  int mt;
{
    char *string = 0;

    switch(mt) {
      case MT_STATUS:
	string = "Status message";
	break;
      case MT_CVERSION:
	string = "Client version";
	break;
      case MT_SVERSION:
	string = "Server version";
	break;
      case MT_RKINIT_INFO:
	string = "Rkinit information";
	break;
      case MT_SKDC:
	string = "Server kdc packet";
	break;
      case MT_CKDC:
	string = "Client kdc packet";
	break;
      case MT_AUTH:
	string = "Authenticator";
	break;
      case MT_DROP:
	string = "Drop server";
	break;
      default:
	string = "Unknown message type";
	break;
    }

    return(string);
}      
	      
int rki_choose_version(version)
  int *version;
{
    int s_lversion;		/* lowest version number server supports */
    int s_hversion;		/* highest version number server supports */
    int status = RKINIT_SUCCESS;
    
    if ((status = 
	 rki_rpc_exchange_version_info(RKINIT_LVERSION, RKINIT_HVERSION, 
				       &s_lversion, 
				       &s_hversion)) != RKINIT_SUCCESS)
	return(status);
    
    *version = min(RKINIT_HVERSION, s_hversion);
    if (*version < max(RKINIT_LVERSION, s_lversion)) {
	sprintf(errbuf, 
		"Can't run version %d client against version %d server.",
		RKINIT_HVERSION, s_hversion);
	rkinit_errmsg(errbuf);
	status = RKINIT_VERSION;
    }

    return(status);
}

int rki_send_rkinit_info(version, info)
  int version;
  rkinit_info *info;
{
    int status = 0;

    if ((status = rki_rpc_send_rkinit_info(info)) != RKINIT_SUCCESS)
	return(status);

    return(rki_rpc_get_status());
}

static int rki_timeout() 
{
    sprintf(errbuf, "%d seconds exceeded.", RKINIT_TIMEOUTVAL);
    rkinit_errmsg(errbuf);
    longjmp(timeout_env, RKINIT_TIMEOUT);
    return(0);
}

static void set_timer(secs)
  int secs;
{
    struct itimerval timer;	/* Time structure for timeout */

    /* Set up an itimer structure to send an alarm signal after TIMEOUT
       seconds. */
    timer.it_interval.tv_sec = secs;
    timer.it_interval.tv_usec = 0;
    timer.it_value = timer.it_interval;
    
    (void) setitimer (ITIMER_REAL, &timer, (struct itimerval *)0);
}
    

int (*rki_setup_timer(env))()
  jmp_buf env;
{
    bcopy((char *)env, (char *)timeout_env, sizeof(jmp_buf));
    set_timer(RKINIT_TIMEOUTVAL);
    return((int (*)()) signal(SIGALRM, rki_timeout));
}

void rki_restore_timer(old_alrm)
  int (*old_alrm)();
{
    set_timer(0);
    (void) signal(SIGALRM, old_alrm);
}
