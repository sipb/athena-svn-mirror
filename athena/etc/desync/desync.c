/*
 * desync - desynchronize cron jobs on networks
 *
 * This program is a tool which sleeps an ip-address dependent period
 * of time in order to skew over the course of an hour cron jobs that
 * would otherwise be synchronized. It should reside on local disk so
 * as not to cause a fileserver load at its own invocation. It avoids
 * the use of the stdio libraries in an effort to keep binary size to
 * a minimum (for DS3100's). Similarly, it gets the host's ip address
 * from rc.conf in order to avoid linking with resolver routines. (ip
 * addresses are used rather than hostnames in order to avoid any
 * systematic biases that may occur due to various cluster naming
 * schemes. That, and that's how it was done before.)
 *
 * There are two reasons this program exists:
 *
 *   1. It's smaller than sleep under Ultrix, and I'm paranoid about
 *	disk space on the 3100's. That is, it's cheaper than bringing
 *	sleep local, which is what the old method used. (Loading sleep
 *	itself is currently causing load spikes.)
 *
 *   2. The current mechanism keeps getting broken on various platforms
 *	because noone pays attention to the fact that it needs to inherit
 *	environment variables from rc.conf. This code will avoid that
 *	breakage in the future.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#ifdef BIG
#include <stdio.h>
#include <errno.h>

char *name;
#endif

#ifndef NULL
#define NULL 0
#endif

#define CONF "/etc/athena/rc.conf"

char *readFile(file)
     char *file;
{
  int cc, d;
  struct stat info;
  char *buf;

  d = open(file, O_RDONLY, 0);
  if (d < 0)
    {
#ifdef BIG
      fprintf(stderr, "%s: Couldn't open %s (errno %d)\n", name, file, errno);
#endif
      return NULL;
    }

  if (fstat(d, &info))
    {
#ifdef BIG
      fprintf(stderr, "%s: Couldn't stat %s (errno %d)\n", name, file, errno);
#endif
      close(d);
      return NULL;
    }

  buf = (char *)malloc(info.st_size + 1);
  if (buf == NULL)
    {
#ifdef BIG
      fprintf(stderr, "%s: Couldn't allocate %d bytes\n", name, info.st_size);
#endif
      close(d);
      return NULL;
    }

  cc = read(d, buf, info.st_size);
  if (cc == -1)
    {
#ifdef BIG
      fprintf(stderr, "%s: Couldn't read %s (errno %d)\n", name, file, errno);
#endif
      free(buf);
      close(d);
      return NULL;
    }

  if (cc != info.st_size)
    {
#ifdef BIG
      fprintf(stderr, "%s: Didn't read expected number of bytes from %s\n",
              name, file);
#endif
      free(buf);
      close(d);
      return NULL;
    }

  close(d);
  buf[info.st_size] = '\0';
  return buf;
}

char *getAddr(name)
     char *name;
{
  int found = 0;
  static char addr[20];
  char *buf;
  int i;

  buf = readFile(name);
  if (buf == NULL)
    return NULL;

  while (!found)
    {
      while (*buf != 'A' && *buf != '\0')
	buf++;
      if (*buf++ == '\0') return NULL;
      if (buf[0] == 'D' && buf[1] == 'D' && buf[2] == 'R')
	{
	  buf += 3;
	  while (isspace(*buf)) buf++;
	  if (*buf == '=')
	    {
	      buf++;
	      found = 1;
	    }
	}
    }

  if (found)
    {
      while (isspace(*buf)) buf++;
      for (i = 0; *buf != ';' && *buf != '\0' && i < sizeof(addr) - 1; i++)
	addr[i] = *buf++;
      addr[i] = '\0';
      return addr;
    }

  return NULL;
}

int main(argc, argv)
     int argc;
     char **argv;
{
  char *addr, *low;
  int num;

#ifdef BIG
  name = argv[0];
#endif

  addr = getAddr(CONF);
  if (addr)
    {
      low = strrchr(addr, '.');
      if (low)
	{
	  num = atoi(low+1);
	  num *= 13;
	  if (argv[1])
	    num += atoi(argv[1]);
	  sleep(num);
	  exit(0);
	}
    }
  exit(1);
}
