/* This program looks up a given hostname on the nameserver and prints
 * its Internet address, official name, etc.
 *
 * Win Treese
 * MIT Project Athena
 *
 * Copyright (c) 1986 by the Massachusetts Institute of Technology.
 *
 * Created:	08/29/86
 *
 * Modified:	02/03/87	Doug Alan
 * 		Let input also be in form of an internet address.
 *
 * Modified:	02/04/87	Doug Alan
 *		Take flags to control output.
 *			
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/hostinfo/hostinfo.c,v 1.3 1990-10-25 11:59:07 epeisach Exp $";
#endif

#include <stdio.h>			/* Standard IO */
#include <sys/types.h>			/* System type defs. */
#include <netdb.h>			/* Networking defs. */
#include <sys/socket.h>			/* More network defs. */
#include <strings.h>			/* String fct. declarations. */
#include <ctype.h>			/* Character type macros. */
#include <netinet/in.h>			/* Internet defs. */
#include <sys/param.h>			/* for MAXHOSTNAMELEN */

typedef int bool;

#define TRUE		1
#define FALSE		0
#define ERROR		1
#define CPNULL		((char *) NULL)

static char *usage[] = {
  "Usage: %s [-h | -a] hostname|address ...",
  "   -h: output only hostname",
  "   -a: output only Internet address",
  "   Both flags must not be set",
  CPNULL,
};

char *myname;				/* How program was invoked. */

static void usage_error();

main(argc, argv)
int argc;
char *argv[];
{
     extern int getopt();
     extern int optind;
     extern int h_errno;

     struct hostent *host_entry;	/* Host entry of it. */
     bool h_flag = FALSE;
     bool a_flag = FALSE;
     int flagc;
     int status = 0;
  
     myname = (myname = rindex(argv[0], '/')) ? myname + 1 : argv[0];

     while ((flagc = getopt(argc, argv, "ha")) != EOF)
	  switch ((char) flagc) {
	  case 'h':
	       if (a_flag) usage_error();
	       else h_flag = TRUE;
	       break;
	  case 'a':
	       if (h_flag) usage_error();
	       else a_flag = TRUE;
	       break;
	  case '?':
	  default:
	       usage_error();
	       break;
	  }
     if (argc == optind)
	  usage_error();
     
     while (optind < argc) {
	  host_entry = NULL;
	  char *hostname = argv[optind++];
	  
	  if (*hostname >= '0' && *hostname <= '9') {
	       unsigned long host_address = inet_addr(hostname);
	       if (host_address != -1)
		    host_entry = gethostbyaddr((char *) &host_address,
					       4, AF_INET);
	  }
	  if (! host_entry)
	       host_entry = gethostbyname(hostname);

	  status |= print_it(hostname, host_entry, h_flag, a_flag);
	  /* Print a blank line between blocks of verbose output */
	  if (! (h_flag || a_flag || (optind == argc)))
	       printf("\n");
     }
     exit(status);
}


print_it(hostname, entry, h_flag, a_flag)
char *hostname;
struct hostent *entry;
bool h_flag, a_flag;
{
     if (! entry) {
	  switch (h_errno) {
#ifdef ultrix
	       /*
		*  it can return NULL, h_errno == 0 in some cases when
		*  there is no name
		*/
	  case 0:
#endif
	  case HOST_NOT_FOUND:
	       printf("No such host '%s'.\n",hostname);
	       return(ERROR);
	  default:
	  case TRY_AGAIN:
	  case NO_RECOVERY:
	       printf("Cannot resolve name '%s' due to network difficulties.\n",
		      hostname);
	       return(ERROR);
	  case NO_ADDRESS:
	       /* should look up MX record? */
	       /*
		*  this error return appears if there is some entry for the
		*   requested name, but no A record (e.g. only an NS record)
		*/
	       printf("No address for '%s'.\n", hostname);
	       return(ERROR);
	  }
     }
     if (h_flag)
	  printf("%s\n", entry->h_name);
     else if (a_flag)
	  printf("%s\n", inet_ntoa(*((struct in_addr *) entry->h_addr)));
     else {
	  char **alias;
	  char **addr;
       
	  printf("Desired host:\t%s\n", hostname);
	  printf("Official name:\t%s\n", entry->h_name);
	  for (alias = entry->h_aliases; *alias; alias++)
	       printf("Alias:\t\t%s\n", *alias);
	  for (addr = entry->h_addr_list; *addr; addr++)
	       printf("Host address:\t%s\n",
		      inet_ntoa(*((struct in_addr *) *addr)));
     }
     return(0);
}





     
static void
usage_error()
{
  int line = 0;				/* current line */
  while (usage[line] != CPNULL)
    {
      fprintf (stderr, usage[line++], myname);
      putc('\n', stderr);
    }
  exit(ERROR);
} /* usage_error */
