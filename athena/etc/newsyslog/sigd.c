/*
 *  sigd - print names of signals we get
 *         [it's not a real deamon-- so sue me. =)]
 */

#include <signal.h>
#include <malloc.h>
#include "signames.h"

int    restart_sig;
int    exit_sig;

FILE **logfiles = NULL;
char **names = NULL;

FILE  *def_lf[2];

void sig_handler(int signum)
{
   char *name = signal_name(signum);
   char **n;
   FILE **f;
   int exiting = (signum == exit_sig);

   signal(signum, sig_handler);

   /* using printf in signal handlers is bad, but I really don't want to deal
      with raw write()'s right now, and this is only a test anyway. */

   for (f = logfiles; f && *f; f++) {
      if (name)
	 fprintf(*f, "%s   got a SIG%s\n",
		 exiting? "[exiting]    " : "[pre-restart]",
		 name);
      else
	 fprintf(*f, "%s   got signal %d\n", 
		 exiting? "[exiting]    " : "[pre-restart]",
		 signum);
   }

   if (exiting)
      exit(0);

   if (names) {
      /* restart */
      for (f = logfiles, n = names; n && *n; f++, n++) {
	 if (**n)
	    *f = freopen( *n, "a", *f );
	 if (! *f)  abort();
      }
   }
	 
   for (f = logfiles; f && *f; f++) {
      if (name)
	 fprintf(*f, "[post-restart]  got a SIG%s\n", name);
      else
	 fprintf(*f, "[post-restart]  got signal %d\n", signum);
   }

   for (f = logfiles; f && *f; f++)
      fflush(*f);
}

void open_files (char **names)
{
   int count, i;
   char **n;

   /* assume logfiles==NULL */

   for (n = names, count = 0; n && *n; n++, count++) ;
   logfiles = (FILE **) malloc((count+1)*sizeof(FILE*));

   for (i=0; i<count; i++) {
      logfiles[i] = fopen( names[i], "a" );
      if (! logfiles[i])  abort();
   }
   logfiles[count] = NULL;
}

void main(int argc, char **argv)
{
   if (argc < 3) exit(100);

   if ((restart_sig = signal_number( argv[1] )) <= 0)
      exit(101);

   if ((exit_sig = signal_number( argv[2] )) <= 0)
      exit(102);

   if (argc == 3) {
      names = NULL;
      def_lf[0] = stdout;
      def_lf[1] = NULL;
      logfiles = def_lf;
   } else {
      names = (argv + 3);
      open_files( names );
   }

   signal(restart_sig, sig_handler);
   signal(exit_sig, sig_handler);

   while(1) {
      sleep(3600);
   }
}
