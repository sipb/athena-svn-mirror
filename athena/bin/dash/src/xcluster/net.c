#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <hesiod.h>

int net(progname, num, names)
     char *progname;
     int num;
     char **names;
{
  static char *host;		/* name of host running cview daemon. */
  static char **serv;		/* cview service hesiod info. */
  static struct hostent *hp;	/* hostent struct for cview server. */
  static struct servent *sp;	/* servent struct for cview service. */
  static struct sockaddr_in sin; /* Socket address. */
  static int init = 0;		/* Have we been here before? */
  int s;			/* Socket to connect to cview daemon. */
  int j;			/* Counter. */
  char buf[BUFSIZ];		/* Temporary buffer. */

  if (!init)
    {
#ifdef TEST
      host = "fries.mit.edu";
#else
      serv = hes_resolve("cview","sloc"); 

      if (serv == NULL)
	host = "doghouse.mit.edu";	/* fall back if hesiod is broken... */
      else
	host = *serv;
#endif

      hp = gethostbyname(host);

      if (hp == NULL) 
	{
	  fprintf(stderr, "%s: Unable to resolve hostname '%s'.\n",
		  progname, host);
	  return(-1);
	}
  
      sp = getservbyname("cview", "tcp");
      if (sp == 0) 
	{
	  fprintf(stderr, "%s: Unknown service 'tcp/cview'.\n", progname);
	  return(-1);
	}

      memset(&sin, 0, sizeof (sin));
#ifdef POSIX
      memmove((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
#else
      bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
#endif
      sin.sin_family = hp->h_addrtype;
      sin.sin_port = sp->s_port;

      init = 1;
    }
  
  s = socket(hp->h_addrtype, SOCK_STREAM, 0);
  if (s < 0) 
    {
      perror("socket"); 
      return(-1);
    }

  if (connect(s, (char *)&sin, sizeof (sin)) < 0) 
    {
      perror("connect");
      close(s);
      return(-1);
    }

  for (j=0; j<num; j++)
    {
      sprintf(buf, "%s ", *names);
      if (write(s, buf, strlen(buf)) == -1)
	{
	  perror("write");
	  close(s);
	  return(-1);
	}
      names++;
    }

  if (write(s, "\n", 1) == -1)
    {
      perror("write");
      close(s);
      return(-1);
    }

  return(s);
}
