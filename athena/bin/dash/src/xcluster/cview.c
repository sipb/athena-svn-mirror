#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include "net.h"

/* 
 * MAIN PROGRAM
 */

void main(argc, argv)
     int argc;
     char **argv;
{
  FILE *f;			/* stream for reading data from cviewd. */
  int s;			/* socket returned from "net". */
  char buf[1024];		/* Temporary buffer. */
  char *progname;		/* Name of program from command-line. */

  progname = strrchr(argv[0], '/');
  if(progname == (char *) NULL)
     progname = argv[0];	/* Find out what our name is today. */
  if(*progname == '/')		/* Useful for being able to print error */
     ++progname;		/* messages and the like. */

  argc--;			/* decrement number of args to pass */
  argv++;			/* and increment counter into command-line */

  s = net(progname, argc, argv);

  if (s > 0)
    {
      f = fdopen(s, "r");

      while(fgets(buf, 1024, f) != NULL)
	printf("%s", buf);

      (void) fclose(f);
    }

  exit(0);
}
