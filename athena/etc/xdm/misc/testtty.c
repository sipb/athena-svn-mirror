#include <stdio.h>
#include "nannylib.h"

int main(int argc, char **argv, char **env)
{
  char tty[20];
  int ret;

  ret = nanny_getTty(tty, sizeof(tty));

  if (ret)
    {
      fprintf(stderr, "getTty failed.\n");
      exit(1);
    }

  fprintf(stdout, "tty = %s\n", tty);
  exit(0);
}
