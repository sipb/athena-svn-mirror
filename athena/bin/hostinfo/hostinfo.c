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
 * Modified:    08/20/90	Tom Coppet
 *		Added HINFO queries and list all addrs.
 *			
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/hostinfo/hostinfo.c,v 1.5 1991-07-01 14:59:14 epeisach Exp $";
#endif

#include <stdio.h>			/* Standard IO */
#include <sys/types.h>			/* System type defs. */
#include <netdb.h>			/* Networking defs. */
#include <sys/socket.h>			/* More network defs. */
#include <strings.h>			/* String fct. declarations. */
#include <ctype.h>			/* Character type macros. */
#include <netinet/in.h>			/* Internet defs. */

typedef int bool;

#define TRUE		1
#define FALSE		0
#define LINE_LENGTH	128
#define ERROR		-1
#define CPNULL		((char *) NULL)

static char *usage[] = {
  "Usage: %s <host-name-or-address>",
  "   -h: output only hostname",
  "   -a: output only internet address",
  "   -n: punt hinfo query",
  "   Both -h and -a flags must not be set",
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

  char hostname[LINE_LENGTH];	           /* Name of desired host. */
  struct hostent *host_entry = NULL;	   /* Host entry of it. */
  struct hostent hostbuf;
  struct in_addr internet_address;
  char **alias;				   /* Ptr. to aliases. */
  char **addr;
  char *hinfo = NULL;
  int  i;
  bool h_flag = FALSE;
  bool a_flag = FALSE;
  bool n_flag = FALSE;
  int flagc;

  myname = (myname = rindex(argv[0], '/')) ? myname + 1 : argv[0];

  while ((flagc = getopt(argc, argv, "nha")) != EOF)
    switch ((char) flagc) {
    case 'h':
      if (a_flag) usage_error();
      else h_flag = TRUE;
      break;
    case 'a':
      if (h_flag) usage_error();
      else a_flag = TRUE;
      break;
    case 'n':
      n_flag = TRUE;
      break;
    case '?':
    default:
      usage_error();
      break;
    }
  if (argc -1 != optind) usage_error();
  (void) strcpy(hostname, argv[optind]);
  if (*hostname >= '0' && *hostname <= '9')
    {
      unsigned long host_address = inet_addr(hostname);
      if (host_address == -1) goto oops;
      host_entry = gethostbyaddr((char *) &host_address, 4, AF_INET);
      if(host_entry != (struct hostent *) NULL) {
	  bcopy(host_entry, &hostbuf, sizeof(hostbuf));
	  host_entry = &hostbuf;	    
	  if(!n_flag)
	    hinfo = (char *) gethinfobyaddr((char *) &host_address, 4, AF_INET);
      }
    }
  else
    {
    oops:
      host_entry = gethostbyname(hostname);
      if(host_entry != (struct hostent *) NULL) {
	  bcopy(host_entry, &hostbuf, sizeof(hostbuf));
	  host_entry = &hostbuf;	      
 	  if(!n_flag)
	    hinfo = (char *) gethinfobyname(hostname);
      }
    }
  if (host_entry == NULL)
    {
	switch (h_errno) {
#ifdef ultrix
	/* it can return NULL, h_errno == 0 in some cases when
	   there is no name */
	case 0:
#endif
	case HOST_NOT_FOUND:
	    printf("No such host '%s'.\n",hostname);
	    exit(ERROR);
	    break;
	default:
	case TRY_AGAIN:
	case NO_RECOVERY:
	    printf("Cannot resolve name '%s' due to network difficulties.\n",
		   hostname);
	    exit(ERROR);
	    break;
	case NO_ADDRESS:
	    /* should look up MX record? */
	    /* this error return appears if there is some entry for the
	       requested name, but no A record (e.g. only an NS record) */
	    printf("No address for '%s'.\n", hostname);
	    exit(ERROR);
	    break;
	}
    }

  if (h_flag) printf("%s\n", host_entry->h_name);
  else  {
      if(!a_flag) {
      	printf("Desired host:\t%s\n", hostname);
      	printf("Official name:\t%s\n", host_entry->h_name);
      	for (alias = host_entry->h_aliases; *alias; alias++)
		printf("Alias:\t\t%s\n", *alias);
      }
      for (addr = host_entry->h_addr_list; *addr; addr++) {
	  bcopy(*addr, &internet_address, host_entry->h_length);
          if(a_flag)
		printf("%s\n", inet_ntoa(internet_address));
	  else
	  	printf("Host address:\t%s\n", inet_ntoa(internet_address));      
      }
      
      if(hinfo && !a_flag) {
	  i = (int) hinfo[0];
	  hinfo[i+1] = '/';
	  printf("Host info:\t%s\n", hinfo+1);
      }
   }
} /* main */

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
