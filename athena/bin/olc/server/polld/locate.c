/*
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 */

#include <mit-copyright.h>
#include <polld.h>

#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/access.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif


/* locate.c */
#ifdef VOID_SIGRET
static void do_timeout P((int sig ));
#else
static int do_timeout P((int sig ));
#endif
static int find_finger P((PTF *person ));
static int find_zephyr P((PTF *person ));

#undef P

static jmp_buf env;		/* for longjmp in finger timeout */
static int fd;			/* Socket for fingering */
static FILE *f;			/* Associated FILE * */
#ifdef ZEPHYR
static int use_zephyr;		/* If 1, use zephyr */
#endif

#ifdef VOID_SIGRET
static void
#else
static int
#endif
  do_timeout(sig)
    int sig;
{
  if (f != NULL)
    fclose(f);
  longjmp(env,MACHINE_DOWN);
}

int
locate_person(person)
     PTF *person;
{
  int new_status;


#ifdef ZEPHYR
  if (use_zephyr) {
    new_status = find_zephyr(person);
    if ((new_status == LOGGED_OUT) || (new_status == LOC_ERROR))
      new_status = find_finger(person);
  } else {
    new_status = find_finger(person);
  }
#else
  new_status = find_finger(person);
#endif

/* temporary fudge for now- */
  if (new_status == MACHINE_DOWN)
    return(LOC_NO_CHANGE);

  if (new_status == LOC_ERROR)
    return(new_status);
  else if (new_status == person->status)
    return(LOC_NO_CHANGE);
  else {
    person->status = new_status;
    return(LOC_CHANGED);
  }
}


/* Try fingering at their last location */

/* Sees if the person is actually logged in at their current location; */
/* returns LOC_ERROR or their login status */


static int
  find_finger(person)
PTF *person;
{
  char namebuf[BUF_SIZE];
  int new_status;
  static int finger_port = 0;
  struct hostent *host;		/* Host entry for receiver */
  struct sockaddr_in sin;	/* Socket address */
  int len;

  if (finger_port == 0) {
    struct servent *service;
    service = getservbyname("finger","tcp");
    if (!service) {
      syslog(LOG_ERR,"locate_person: Can't find `finger' service");
      return(LOC_ERROR);
    }
    finger_port = service->s_port;
  }
  
  host = c_gethostbyname(person->machine);
  if (host == (struct hostent *) NULL) {
    syslog(LOG_WARNING,"locate_person: Can't resolve name of host `%s'",
	   person->machine);
    return(LOC_ERROR);
  }

  sin.sin_family = host->h_addrtype;
  bcopy(host->h_addr, (char *) &sin.sin_addr, host->h_length);
  sin.sin_port = finger_port;
  if ((fd = socket(host->h_addrtype, SOCK_STREAM, 0)) < 0) {
    syslog(LOG_ERR,"locate_person: Error opening socket: %m");
    return(LOC_ERROR);
  }

  signal(SIGALRM, do_timeout);
  alarm(FINGER_TIMEOUT);
  setjmp(env);

  if (connect(fd, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
    alarm(0);
    signal(SIGALRM, SIG_IGN);
    close(fd);
    return(MACHINE_DOWN);
  } 

  new_status = LOGGED_OUT;
  write(fd,"\r\n",2);
  f = fdopen(fd,"r");
  if (f == NULL) {
    syslog(LOG_ERR,"Error fdopening finger fd to %s: %m", person->machine);
    alarm(0);
    signal(SIGALRM, SIG_IGN);
    close(fd);
    return(LOC_ERROR);
  }

  len = strlen(person->username);
  while (fgets(namebuf,BUF_SIZE,f) != NULL) {
    if (strncmp(namebuf,person->username,len) == 0) {
      new_status = ACTIVE;
      break;
    }
  }
  
  if (fclose(f) == EOF)
    syslog(LOG_ERR,"Error closing finger fd to %s: %m", person->machine);
  signal(SIGALRM, SIG_IGN);
  alarm(0);

  return(new_status);
}


#ifdef ZEPHYR

/* Returns the person's login status, with person updated to */
/* reflect the new information */
/* Zlocates the user, then fingers at each of the returned locations to see */
/* if they're really there. */

static int
find_zephyr(person)
     PTF *person;
{
  char namebuf[BUF_SIZE];
  int retval;
  int numlocs;
  ZLocations_t locations[1];
  int new_status;
  int one,i;

  strcpy(namebuf,person->username);
  strcat(namebuf,"@");
  strcat(namebuf,ZGetRealm());

  if ((retval = ZLocateUser(namebuf,&numlocs,ZAUTH)) != ZERR_NONE) {
    syslog(LOG_WARNING,"zephyr error while locating user %s: %s", namebuf,
	   error_message(retval));
    return(LOC_ERROR);
  }

  if (numlocs == 0)
    return(LOGGED_OUT);
 
  new_status = LOGGED_OUT;
  one = 1;
  
  /* if they are logged in according to zephyr: */
  for (i=0;i<numlocs;i++) {
    if ((retval = ZGetLocations(locations,&one)) != ZERR_NONE) {
      syslog(LOG_WARNING,"zephyr error in ZGetLocations for %s: %s", namebuf,
	     error_message(retval));
      return(LOC_ERROR);
    }
    
    if (strcmp(person->machine,locations[0].host) != 0)
      strncpy(person->machine,locations[0].host,NAME_SIZE);
    new_status = find_finger(person);
    if (new_status == ACTIVE)
      break;
  }
  return(new_status);
}


void
check_zephyr()
{
  /* this lets us turn off zephyr polling and fall back on fingering only by */
  /* creating ZEPHYR_DOWN_FILE; this is useful when the zephyr servers get */
  /* overwhelmed */

  if (access(ZEPHYR_DOWN_FILE,E_ACC) == 0) {
    use_zephyr = 0;
  } else {
    use_zephyr = 1;
  }
}
#endif /* ZEPHYR */

