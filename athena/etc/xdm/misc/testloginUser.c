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
  char **args, **newenv, *tty;

  ret = nanny_loginUser(&newenv, &args, &tty);
  if (ret)
    {
      fprintf(stderr, "loginUser failed.\n");
      exit(1);
    }

  printf("Environment:\n");
  putstrings(newenv);
  printf("\nArgs:\n");
  putstrings(args);
  printf("\ntty: %s\n", tty);

  exit(0);
}
