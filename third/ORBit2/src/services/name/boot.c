#include "CosNaming.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <popt.h>
#include "CosNaming_impl.h"

static void
signal_handler(int signo){
  syslog(LOG_ERR,"Receveived signal %d\nshutting down.", signo);
  switch(signo) {
    case SIGSEGV:
	abort();
    default:
	exit(1);
  }
}

static char *opt_corbaloc_key = 0;

static const struct poptOption TheOptions[] = {
    { "key", 0, POPT_ARG_STRING, &opt_corbaloc_key, 0, 
    	"Respond to corbaloc requests with this object key", "string" },
#if 0
    ORBIT_POPT_TABLE,
#endif
    POPT_AUTOHELP
    { 0 }
};

int
main (int argc, char *argv[])
{
  CORBA_ORB orb;
  CORBA_Environment ev;
  CosNaming_NamingContext context;
  const char*		progname = "orbit-name-server";
  
  openlog(progname, LOG_NDELAY | LOG_PID, LOG_DAEMON);
  syslog(LOG_INFO,"starting");
  {
  	sigset_t empty_mask;
  	struct sigaction act;
  	sigemptyset(&empty_mask);
  	act.sa_handler = signal_handler;
  	act.sa_mask    = empty_mask;
  	act.sa_flags   = 0;
  
  	sigaction(SIGINT,  &act, 0);
  	sigaction(SIGHUP,  &act, 0);
  	sigaction(SIGSEGV, &act, 0);
  	sigaction(SIGABRT, &act, 0);
  
  	act.sa_handler = SIG_IGN;
  	sigaction(SIGPIPE, &act, 0);
  }
  
  {
  	poptContext	pcxt;
	int		rc;
        pcxt = poptGetContext(progname, argc, (const char**)argv, 
		TheOptions, 0);
      	if ( (rc=poptGetNextOpt(pcxt)) < -1 ) {
	    g_warning("%s: bad argument %s: %s\n",
	    		progname, 
			poptBadOption(pcxt, POPT_BADOPTION_NOALIAS), 
			poptStrerror(rc));
	    exit(1);
      }
      if ( poptGetArg(pcxt) != 0 ) {
	    g_warning("%s: leftover arguments.\n", progname);
	    exit(1);
      }
      poptFreeContext(pcxt);
  }
  CORBA_exception_init (&ev);
  orb = CORBA_ORB_init (&argc, argv, "orbit-local-orb", &ev);

  {
  	PortableServer_POA root_poa;
      	PortableServer_POAManager pm;
  	root_poa = (PortableServer_POA)
    	  CORBA_ORB_resolve_initial_references (orb, "RootPOA", &ev);
  	context = ORBit_CosNaming_NamingContextExt_create (root_poa, &ev);
      	pm = PortableServer_POA__get_the_POAManager (root_poa, &ev);
      	PortableServer_POAManager_activate (pm, &ev);
      	CORBA_Object_release((CORBA_Object)pm, &ev);
  	CORBA_Object_release((CORBA_Object)root_poa, &ev);
  }

  {
      	CORBA_char *objstr;
      	objstr = CORBA_ORB_object_to_string (orb, context, &ev);
      	g_print ("%s\n", objstr);
      	fflush (stdout);
      	CORBA_free(objstr);
  }
  if ( opt_corbaloc_key ) {
	CORBA_sequence_CORBA_octet	okey;
	okey._length = strlen(opt_corbaloc_key);
	okey._buffer = opt_corbaloc_key;
  	ORBit_ORB_forw_bind(orb, &okey, context, &ev);
  }


  CORBA_ORB_run (orb, &ev);

  /* Don't release until done (dont know why) */
  CORBA_Object_release (context, &ev);

  syslog(LOG_INFO, "exiting");
  return 0;
}
