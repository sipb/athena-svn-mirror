/*
 * gettime
 *
 * Retrieve the time from a remote host and print it in ctime(3)
 * format. The time is acquired via the protocol described in
 * RFC868. The name of the host to be queried is specified on the
 * command line, and if the -s option is also specified, and gettime
 * is run as root, the clock on the local workstation will also be
 * set. gettime makes five attempts at getting the time, each with
 * a five second timeout.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

static char rcsid[] = "$Id: gettime.c,v 1.13 1997-12-15 00:34:34 cfields Exp $";

#define UNIX_OFFSET_TO_1900 ((70 * 365UL + 17) * 24 * 60 * 60)
#define TIME_SERVICE "time"
#define MAX_TRIES 5
#define TIMEOUT 5

char *program_name;

void myperror(char *what, int errnum)
{
  fprintf(stderr, "%s: %s: %s\n", program_name, what, strerror(errnum));
  exit(1);
}

int main(int argc, char **argv)
{
  struct servent *service_info;
  struct hostent *host_info = NULL;
  struct sockaddr_in time_address;
  char *time_hostname, buffer[20];
  int tries, time_socket, result;
  fd_set read_set;
  struct timeval timeout, current_time;
  struct timezone current_timezone;
  time_t now;
  int c, setflag = 0, errflag = 0;
  int granularity = 0;

  /* Set up our program name. */
  if (argv[0] != NULL)
    {
      program_name = strrchr(argv[0], '/');
      if (program_name != NULL)
	program_name++;
      else
	program_name = argv[0];
    }
  else
    program_name = "gettime";

  /* Parse arguments. */
  while ((c = getopt(argc, argv, "sg:")) != EOF)
    {
      switch (c)
	{
	case 's':
	  setflag = 1;
	  break;
	case 'g':
	  granularity = atoi(optarg);
	  break;
	case '?':
	  errflag = 1;
	  break;
	}
    }

  if (errflag || optind + 1 != argc)
    {
      fprintf(stderr, "usage: %s [-g granularity] [-s] hostname\n",
	      program_name);
      exit(1);
    }

  time_hostname = argv[optind];

  /* Look up port number for time service. */
  service_info = getservbyname(TIME_SERVICE, "udp");
  if (service_info == NULL)
    {
      fprintf(stderr, "%s: " TIME_SERVICE "/udp: unknown service\n",
	      program_name);
      exit(1);
    }

  /* Resolve hostname (try MAX_TRIES times). We do this in case the
   * nameserver is just starting up (as in, everyone is just rebooting
   * from a long power failure, and we are requesting name resolution
   * before the server is ready).
   */
  for (tries = 0; tries < MAX_TRIES; tries++)
    {
      host_info = gethostbyname(time_hostname);
      if (host_info != NULL || (host_info == NULL && h_errno != TRY_AGAIN))
	break;
    }

  if (host_info == NULL)
    {
      fprintf(stderr, "%s: host %s unknown\n", program_name, time_hostname);
      exit(1);
    }

  if (host_info->h_addrtype != AF_INET)
    {
      fprintf(stderr, "%s: can't handle address type %d\n",
	      program_name, host_info->h_addrtype);
      exit(1);
    }

  /* Grab a socket. */
  time_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (time_socket < 0)
    myperror("socket", errno);

  /* Set up destination address. */
  time_address.sin_family = AF_INET;
  memcpy(&time_address.sin_addr, host_info->h_addr,
	 sizeof(time_address.sin_addr));
  time_address.sin_port = service_info->s_port;

  /* "Connect" the UDP socket to the time server address. */
  if (connect(time_socket, (struct sockaddr *)&time_address,
	      sizeof(time_address)) < 0)
    myperror("connect", errno);

  /* Initialize info for select. */
  FD_ZERO(&read_set);

  /* Attempt to acquire the time MAX_TRIES times at TIMEOUT interval. */
  for (tries = 0; tries < MAX_TRIES; tries++)
    {
      /* Just send an empty packet. */
      send(time_socket, buffer, 0, 0);

      /* Wait a little while for a reply. */
      FD_SET(time_socket, &read_set);
      timeout.tv_sec = TIMEOUT;
      timeout.tv_usec = 0;
      result = select(time_socket + 1, &read_set, NULL, NULL, &timeout);

      if (result == 0)		/* timed out, resend... */
	continue;

      if (result < 0)		/* error from select */
	myperror("select", errno);

      /* There must be data available. */
      result = recv(time_socket, buffer, sizeof(buffer), 0);
      if (result < 0)
	myperror("recv", errno);

      /* We should have received exactly four bytes in the packet. */
      if (result == 4)
	break;

      fprintf(stderr, "%s: received %d bytes, expected 4\n", program_name,
	      result);
      exit(1);
    }

  if (tries == MAX_TRIES)
    {
      fprintf(stderr, "%s: Failed to get time from %s.\n", program_name,
	      time_hostname);
      exit(1);
    }

  /* Convert RFC868 time to Unix time, and print it. */
  now = ((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3])
    - UNIX_OFFSET_TO_1900;
  fprintf(stdout, "%s", ctime(&now));

  /* Set the time if requested. */
  if (setflag)
    {
      gettimeofday(&current_time, &current_timezone);

      if (current_time.tv_sec < now - granularity
	  || current_time.tv_sec >= now + granularity)
	{
	  current_time.tv_sec = now;
	  current_time.tv_usec = 0;
	  if (settimeofday(&current_time, &current_timezone) < 0)
	    myperror("settimeofday", errno);
	}
    }

  exit(0);
}
