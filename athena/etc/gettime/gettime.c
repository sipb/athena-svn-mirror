/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/etc/gettime/gettime.c,v $
 *	$Author: builder $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/etc/gettime/gettime.c,v 1.1 1985-05-27 16:17:18 builder Exp $
 */

#ifndef lint
static char *rcsid_gettime_c = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/gettime/gettime.c,v 1.1 1985-05-27 16:17:18 builder Exp $";
#endif	lint

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <setjmp.h>
char buffer[512];
char *ctime();
struct timeval tv;
struct timezone tz;
jmp_buf top_level;
int hiccup();
main(argc, argv)
	int argc;
	char *argv[];
{
	struct sockaddr_in sin;
	struct servent *sp;
	struct hostent *host;
	int setflg = 0;
	register int i;
	register int s;
	long hosttime;
	register int *nettime;
	char hostname[64];
	int attempts = 0;
	strcpy (hostname, "dcn1");
	for (i = 1;i < argc;i++) {
		if (*argv[i] == '-') {
			if (argv[i][1] == 's') setflg++;
			else {
				fprintf (stderr, "%s: Invalid argument %s\n",
				 argv[0], argv[i]);
				 exit (11);
			}
		}
		else strcpy (hostname, argv[i]);
	}
	sp = getservbyname("time", "udp");
	if (sp == 0) {
		fprintf (stderr, "%s: time/udp: unknown service.\n",
			argv[0]);
		exit (1);
	}
	host = gethostbyname(hostname);
	if (host == NULL) {
		fprintf (stderr, "%s: The timeserver host %s is unknown\n",
			 argv[0], hostname);
		exit (2);
	}
	sin.sin_family = host->h_addrtype;
	bcopy (host->h_addr, (caddr_t)&sin.sin_addr,
			host->h_length);
	sin.sin_port = sp->s_port;
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror ("gettime: socket");
		exit (3);
	}
	if (connect (s, (caddr_t)&sin, sizeof (sin)) < 0) {
		perror ("gettime: connect");
		exit (4);
	}
	setjmp(top_level);
	if (attempts++ > 5) {
		close (s);
		fprintf (stderr, "Failed to get time from %s\n",
			 hostname);
		exit (10);
	}
	alarm(0);
	signal(SIGALRM, hiccup);
	alarm(5);
	send (s, buffer, 40, 0); /* Send an empty packet */
	if (gettimeofday (&tv, &tz) < 0) {
		perror ("gettime: gettimeofday");
		exit (5);
	}
	recv (s, buffer, 512, 0);  /* Wait for the reply */
	nettime = (int *)buffer;
	hosttime = (long) ntohl (*nettime) - 2208988800;
	fprintf (stdout, "%s", ctime(&hosttime));
	(&tv)->tv_sec = hosttime;
	if (setflg) {
		if (settimeofday (&tv, &tz) < 0) {
			perror ("gettime: settimeofday");
			exit (6);
		}
	}
	close (s);
	exit (0);
}
hiccup() {
	longjmp (top_level);
}
