#include <stdio.h>
#include <unistd.h>
#include <signal.h>

/* Tool to wait a specified number of seconds, then exit.
   If there is no input, it prints "timeout", otherwise 
   echoes the line of input. */

void to();

main(int argc, char **argv)
{
  char s[1024];

  signal(SIGALRM, to);

  if(argc!=2)
    {
      printf("usage: %s seconds\n", argv[0]);
      exit(1);
    }
  alarm(atoi(argv[1]));
  gets(s);
  puts(s);
  exit(0);
}

void to()
{
  puts("timeout");
  exit(0);
}
