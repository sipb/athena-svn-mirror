#include <stdio.h>
#include <ctype.h>
#include <hesiod.h>

/*
 * Make a hesiod cluster query
 * for the machine you are on
 * and produce a set of environment variable
 * assignments for the C shell or the Bourne shell,
 * depending on the '-b' flag
 *
 * If any stdio errors, truncate standard output to 0
 * and return an exit status.
 */

main(argc, argv)
char *argv[];
{
  register char **hp;
  int bourneshell = 0;
  char myself[80];

  if (argc < 3 || argc > 4)
    {
      fprintf(stderr, "usage: getcluster [-b] hostname version\n");
      exit(-1);
    }
  if (argc == 4 && strcmp(argv[1], "-b") == 0)
    {
      bourneshell++;
      argv++;
    }
	
  hp = hes_resolve(argv[1], "cluster");
  if (hp == NULL)
    {
      fprintf(stderr, "No Hesiod information available for %s\n", argv[1]);
      exit(-1);
    }
  shellenv(hp, bourneshell, argv[2]);
}

shellenv(hp, bourneshell, version)
char **hp;
int bourneshell;
char *version;
{
  char var[80], val[80], vers[80], **hp_save=hp;
  int specific = 0;

  while (specific < 2)
    {
      while(*hp)
	{
	  vers[0] = '\0';
	  sscanf(*hp++, "%s %s %s", var, val, vers);
	  if ((specific && !vers[0]) || (!specific && vers[0])
	      || (vers[0] != '\0' && vercmp(vers, version)))
	    continue;
	  upper(var);
	  if (bourneshell)
	    printf("%s=%s ; export %s\n", var, val, var);
	  else
	    printf("setenv %s %s\n", var, val);
	}

      specific++;
      hp = hp_save;
    }
  if (ferror(stdout))
    {
      ftruncate(fileno(stdout), 0L);
      exit(-1);
    }
}

upper(v)
register char *v;
{
  while(*v)
    {
      *v = toupper(*v);
      v++;
    }
}

/*
 * Compare as many characters in the version as Hesiod provides, and
 * no more. This allows "8.0" to mean "attach these packs if you're
 * running any version of 8.0" and "8.0A" to mean "attach these packs
 * only if you're running 8.0A."
 */
int vercmp(hesvers, ourvers)
     char *hesvers, *ourvers;
{
  return strncmp(hesvers, ourvers, strlen(hesvers));
}
