#include <stdio.h>
#include "nannylib.h"

void putstrings(char **strs)
{
  while (*strs)
    printf("  %s\n", *strs++);
}

int main(int argc, char **argv, char **env)
{
  int ret;
  char **args, **newenv;

  ret = nanny_loginUser(&newenv, &args);
  if (ret)
    {
      fprintf(stderr, "setupUser failed.\n");
      exit(1);
    }

  printf("Environment:\n");
  putstrings(newenv);
  printf("\nArgs:\n");
  putstrings(args);

  exit(0);
}
