/* This programs turns on/off delayed I/O on a filesystem.
 * 
 * Usage:   fastfs path fast|slow|status
 *
 * e.g.:    fastfs / fast
 *
 * Adapted from code found on Usenet, to speed up Solaris installs.
 * (Normally it is not safe to run with delayed I/O enabled).
 */

#include <stdio.h> 
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/filio.h>
#include <fcntl.h>
#include <errno.h>

char *prog;

static void usage();


int main(int argc, char **argv)
{
  char *path, *cmd;
  int fd;
  int flag;
    
  prog = argv[0];

  if (argc != 3)
    usage();
    
  path = argv[1];
  cmd = argv[2];

  fd = open(path, O_RDONLY, 0);
  if (fd == -1)
    {
      fprintf(stderr, "%s: Unable to open %s: %s\n",
	      prog, path, strerror(errno));
      exit(2);
    }
    
  if (strcmp(cmd, "status") == 0)
    {
      /* Get the current state. */
      if (ioctl(fd, _FIOGDIO, &flag) != 0)
	{
	  fprintf(stderr, "%s: Cannot get status of %s: %s\n",
		  prog, path, strerror(errno));
	  close(fd);
	  exit(3);
	}
    }
  else
    {
      /* Set the state. */
      if (strcmp(cmd, "fast") == 0)
	flag = 1;
      else if (strcmp(cmd, "slow") == 0)
	flag = 0;
      else
	{
	  close(fd);
	  usage();
	}
      if (ioctl(fd, _FIOSDIO, &flag) != 0)
	{
	  fprintf(stderr, "%s: Cannot set %s to %s: %s\n",
		  prog, path, cmd, strerror(errno));
	  close(fd);
	  exit(3);
	}
    }
  close(fd);
  printf("%s is %s\n", path, flag ? "fast" : "slow");
  return 0;
}

static void usage()
{
  fprintf(stderr, "Usage: %s path {fast|slow|status}\n", prog);
  exit(1);
}
