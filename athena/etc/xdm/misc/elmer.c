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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DELAY 5
#define ROOT (uid_t)0
#define N_ROOT "root"

char *session = "/etc/athena/login/Xsession";

char *defargs[2] = { "1", "" };

main(int argc, char **argv, char **envp)
{
  char **env, **args, *tty;
  char **ptr, *name;
  char *prelogin;
  char athconsole[64];
  int c, nannyKnows;
  pid_t uid;
  int fd;

  if (argv[0])
    {
      name = strrchr(argv[0], '/');
      if (++name == NULL)
	name = argv[0];
    }
  else
    name = "elmer";

  /* xdm needed AFS tokens to stat the user's home directory.  We don't want
   * them any more; we're going to make a pag and get new ones. */
  ktc_ForgetAllTokens();

  uid = getuid();
  nannyKnows = !nanny_loginUser(&env, &args, &tty);

  /*
   * If xlogin failed for some reason and we fell back to xdm,
   * enable root logins to be possible via xdm. In this case,
   * nanny hasn't yet been informed that anyone is getting logged
   * in, so we try to do it. But we're not too fancy.
   *
   * Of course, this is only one reason the loginUser call may
   * be failing. If it's failing for other reasons, this code
   * will still help us get through a root login.
   */
  if (!nannyKnows && uid == ROOT)
    {
      if (!nanny_setupUser(N_ROOT, 0, envp, defargs))
	nannyKnows = !nanny_loginUser(&env, &args, &tty);


      /* If we're here because xdm took over, it killed the console.
	 Therefore, we make sure it's running. */
      nanny_setXConsolePref(1);

      /*
       * If for some strange reason our efforts failed anyway,
       * still try to get the tty information properly.
       */
      if (!nannyKnows)
	{
	  env = NULL;
	  args = defargs;

	  if (nanny_getTty(athconsole, sizeof(athconsole)))
	    tty = "/dev/console";
	  else
	    tty = athconsole;
	}
    }

  if (nannyKnows || uid == ROOT)
    {
      /* Redirect stdout and stderr to the console window.
	 Don't do any output to stdout or stderr before this point,
	 or weird things may happen. */
      if ((fd = open(tty, O_WRONLY)) == -1)
	fd = open("/dev/console", O_WRONLY);
      if (fd != -1)
	{
	  dup2(fd, STDOUT_FILENO);
	  dup2(fd, STDERR_FILENO);
	  if (fd != STDOUT_FILENO && fd != STDERR_FILENO)
	    close(fd);
	}

      if (env)
	for (ptr = env; *ptr != NULL; ptr++)
	  putenv(*ptr);

      setpag();

      c = fork();
      if (c == 0)
	{
	  prelogin = getenv("PRELOGIN");
	  if (prelogin && !strcmp(prelogin, "true"))
	    execv("/bin/sh", args);
	  else
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

      if (nannyKnows)
	nanny_logoutUser();
    }
  else
    {
      fprintf(stderr, "%s: loginUser request failed\n", name);
      sleep(DELAY);
    }
}
