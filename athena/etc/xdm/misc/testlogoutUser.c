#include <stdio.h>
#include "nannylib.h"

int main(int argc, char **argv, char **env)
{
  int ret;

  ret = nanny_logoutUser();
  if (ret)
    {
      fprintf(stderr, "logoutUser failed.\n");
      exit(1);
    }

  exit(0);
}
