/*
 *  Chris VanHaren
 *  7/13/89
 */

#include "xolc.h"

static MRMHierarchy	Hierarchy;     /* MRM database hierarchy id */
static MRMCode		class;

extern Widget	/* Widget ID's */
  widget_motdbox,
  widget_motdframe,
  toplevel,
  main_form;

static Display *display;
static Window root;
static int screen;

static char *vec[] = {"olc.uid"};   /* There is 1 uid file to fetch.  */

extern void
  olc_ask(),
  olc_stock(),
  olc_help(),
  olc_quit(),
  olc_send(),
  olc_done(),
  olc_cancel(),
  olc_save_log(),
  olc_motd(),
  olc_update(),
  widget_create()
;

static MRMCount reg_num = 11;
static MRMRegisterArg  reg_vec[] = {
  {"olc_ask", (caddr_t) olc_ask},
  {"olc_stock", (caddr_t) olc_stock},
  {"olc_help", (caddr_t) olc_help},
  {"olc_quit", (caddr_t) olc_quit},
  {"olc_send", (caddr_t) olc_send},
  {"olc_done", (caddr_t) olc_done},
  {"olc_cancel", (caddr_t) olc_cancel},
  {"olc_save_log", (caddr_t) olc_save_log},
  {"olc_motd", (caddr_t) olc_motd},
  {"olc_update", (caddr_t) olc_update},
  {"widget_create", (caddr_t) widget_create},
};

PERSON User;                            /* Structure describing user. */
char DaemonHost[LINE_LENGTH];           /* Name of the daemon's machine. */
char *program;
char REALM[40];
char INSTANCE[40];

char *LOCAL_REALM = "ATHENA.MIT.EDU";
char *LOCAL_REALMS[] =
{
  "MIT.EDU",
  "DU.MIT.EDU",
  "SIPB.MIT.EDU",
  "PIKA.MIT.EDU",
  "CARLA.MIT.EDU",
  "ZBT.MIT.EDU",
  "",
};

int OLC=1;

/*
 *   Main Program
 */

int main(argc, argv)
     int argc;
     char *argv[];
{  
  Arg arglist[1];
  struct passwd *getpwuid();   /* Get a password entry based on the user ID.*/
  struct passwd *pwent;        /* Password entry. */
  char hostname[LINE_LENGTH];  /* Name of local machine. */
  struct hostent *host;
  int uid;                     /* User ID number. */
  char *tty;                   /* Terminal path name. */
  char *prompt;

#ifdef HESIOD
  char **hp;                   /* return value of Hesiod resolver */
#endif

  program = rindex(*argv,'/');
  if(program == (char *) NULL)
     program = *argv;
  if(*program == '/')
     ++program;

#ifdef HESIOD
  if ((hp = hes_resolve(OLC_SERVICE,OLC_SERV_NAME)) == NULL)
    {
      fprintf(stderr,
              "Unable to get name of OLC server host from the Hesiod nameserver.\n");
      fprintf(stderr,
              "This means that you cannot use OLC at this time. Any problems \n");
      fprintf(stderr,
              "you may be experiencing with your workstation may be the result of this\n");
      fprintf(stderr,
              "problem. \n");

      exit(ERROR);
    }
  else
    (void) strcpy(DaemonHost, *hp);

#endif HESIOD


  uid = getuid();
  pwent = getpwuid(uid);
  (void) strcpy(User.username, pwent->pw_name);
  (void) strcpy(User.realname, pwent->pw_gecos);
  if (index(User.realname, ',') != 0)
    *index(User.realname, ',') = '\0';

  gethostname(hostname, LINE_LENGTH);
  host = gethostbyname(hostname);
  (void) strcpy(User.machine, (host ? host->h_name : hostname));
  User.uid = uid;

  expand_hostname(DaemonHost,INSTANCE,REALM);

#ifdef TEST
printf("main: starting for %s/%s (%s)  on %s\n",User.realname, User.username,
       User.uid, User.machine);
#endif TEST

  signal(SIGPIPE, SIG_IGN);
  

  XmInitializeMRM();

  toplevel = XtInitialize("olc", "OLC", NULL, 0, &argc, argv);
  XtSetArg (arglist[0], XtNallowShellResize, TRUE);
  XtSetValues (toplevel, arglist, 1);
  display = XtDisplay(toplevel);
  screen = DefaultScreen(display);
  root = RootWindow(display, screen);
  
  if (XmOpenHierarchy (1, vec, NULL, &Hierarchy) != MRMSuccess)
    printf ("Can't open hierarchy...\n");
  
  if (XmRegisterMRMNames (reg_vec, reg_num) != MRMSuccess)
    printf ("Can't register names...\n");

  if (XmFetchWidget (Hierarchy, "main", toplevel, &main_form,
		     &class) != MRMSuccess)
    printf ("Can't fetch interface...\n");

  olc_init();

  XtManageChild(main_form);
  XtRealizeWidget(toplevel);
  if (! XtIsManaged(widget_motdframe) )
    XtManageChild(widget_motdframe);
  XtMainLoop();

  return(0);

}

olc_init()
{
  int fd;			/* File descriptor for socket. */
  RESPONSE response;		/* Response code from daemon. */
  REQUEST Request;
  ERRCODE errcode=0;		/* Error code to return. */
  int n,first=0;
  char file[NAME_LENGTH];
  char topic[TOPIC_SIZE];
  
  fill_request(&Request);
  Request.request_type = OLC_STARTUP;
  
  fd = open_connection_to_daemon();
  send_request(fd, &Request);
  read_response(fd, &response);
  
#ifdef TEST
  printf("do_olcinit: requester %s, response: %d\n",
         Request.requester.username, response);
#endif TEST
  
  switch(response)
    {
    case USER_NOT_FOUND:
      printf("new user question\n");
#ifdef TEST
      printf("do_olc_init: %s not found\n", Request.requester.username);
#endif TEST
      break;

    case CONNECTED:
    case SUCCESS:
      read_int_from_fd(fd, &n);
      printf("\nWelcome back to OLC. \n\n");
      /*      t_personal_status(&Request); */
      break;
   case PERMISSION_DENIED:
      printf("You are not allowed to use OLC.\n");
      exit(1);
    default:
      if(handle_response(response, &Request) == ERROR)
        exit(ERROR);
    }

  make_temp_name(file);
  x_get_motd(&Request,OLC,file);
  unlink(file);

  (void) close(fd);
  fflush(stdout);
}
