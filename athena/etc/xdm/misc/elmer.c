/*
 * elmer.c
 *
 * This code glues together various components of the Athena login
 * system that have been blown to bits by the SGI login stuff.
 *
 *   Get Xsession args, env, and tty from nanny.
 *
 *   Union the env we got from nanny over what xdm passed us.
 *
 *   Perform a setpag.
 *
 *   Redirect output through tty.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define DELAY 5

char *session = "/etc/athena/login/Xsession";

main(int argc, char **argv)
{
  char **env, **args, *tty;
  char **ptr, *name;
  int c;

  if (argv[0])
    {
      name = strrchr(argv[0], '/');
      if (++name == NULL)
	name = argv[0];
    }
  else
    name = "elmer";

  if (!nanny_loginUser(&env, &args, &tty))
    {
      /* Redirect stdout and stderr to the console window. */
      if (NULL == freopen(tty, "w", stdout))
	(void)freopen("/dev/console", "w", stdout);

      if (NULL == freopen(tty, "w", stderr))
	(void)freopen("/dev/console", "w", stderr);

      for (ptr = env; *ptr != NULL; ptr++)
	putenv(*ptr);

      setpag();

      c = fork();
      if (c == 0)
	{
	  execl(session, "sh", args[0], args[1], NULL);
	  fprintf(stderr, "%s: Xsession exec failed\n", name);
	  sleep(DELAY);
	  exit(1);
	}

      if (c == -1)
	{
	  fprintf(stderr, "%s: fork for Xsession failed\n", name);
	  sleep(DELAY);
	}
      else
	waitpid(c, NULL, 0);

      nanny_logoutUser();
    }
  else
    {
      fprintf(stderr, "%s: loginUser request failed\n", name);
      sleep(DELAY);
    }
}
