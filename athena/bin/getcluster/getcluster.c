#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <hesiod.h>

extern int optind;

static void usage(void);
static void shellenv(const char *const *hp, char *ws_version, int autoupdate,
		     int bourneshell);
static void output_var(const char *var, const char *val, int bourneshell);
static void upper(char *v);
static int vercmp(const char *v1, const char *v2);

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

int main(int argc, char **argv)
{
  char **hp, *envp, buf[256], **hpp;
  int debug = 0, bourneshell = 0, autoupdate = 0, ch;

  while ((ch = getopt(argc, argv, "bd")) != -1) {
    switch (ch) {
    case 'd':
      debug = 1;
      break;
    case 'b':
      bourneshell = 1;
      break;
    default:
      usage();
    }
  }
  argc -= optind;
  argv += optind;

  if (argc != 2)
    usage();

  envp = getenv("AUTOUPDATE");
  if (envp && strcmp(envp, "true") == 0)
    autoupdate = 1;
	
  if (debug) {
    hp = (char **) malloc(100 * sizeof(char *));
    hpp = hp;
    while (fgets(buf, sizeof(buf), stdin) != NULL)
      {
	*hpp = malloc(strlen(buf) + 1);
	strcpy(*hpp, buf);
	(*hpp)[strlen(buf) - 1] = 0;
	hpp++;
      }
    *hpp = NULL;
    shellenv(hp, argv[1], autoupdate, bourneshell);
    return(0);
  }

  hp = hes_resolve(argv[0], "cluster");
  if (hp == NULL)
    {
      fprintf(stderr, "No Hesiod information available for %s\n", argv[0]);
      return(1);
    }
  shellenv(hp, argv[1], autoupdate, bourneshell);

  return(ferror(stdout) ? 1 : 0);
}

static void usage()
{
  fprintf(stderr, "Usage: getcluster [-b] [-d] hostname version\n");
  exit(1);
}

/* Cluster information will come in entries of the form:
 *		variable value version flags
 * There may be multiple entries for the same variable, but not multiple
 * entries with the same variable and version.  There may be at most one
 * entry for a given variable that does not list a version.
 *
 * If autoupdate is false, output variable definitions for entries which
 * have a version matching the workstation major and minor version.
 *
 * If autoupdate is true, output a variable definition for each variable,
 * using the entry with the highest version, BUT discarding entries which:
 *		- have 't' listed in the flags string (for "testing"), and
 *		- do not match the current workstation major and minor
 *		  version
 *
 * If we would not otherwise output a definition for a variable, and
 * there is an entry with no version number, output a definition using
 * that entry. */
static void shellenv(const char *const *hp, char *ws_version, int autoupdate,
		     int bourneshell)
{
  int *seen, count, i, j;
  char var[80], val[80], vers[80], flags[80], compvar[80], compval[80];
  char compvers[80], defaultval[80], new_production[80], new_testing[80];

  count = 0;
  while (hp[count])
    count++;

  seen = (int *) calloc(count, sizeof(int));
  if (seen == NULL)
    exit(1);

  strcpy(new_production, "0.0");
  strcpy(new_testing, "0.0");

  /* The outer loop is for the purpose of "considering each variable."  We skip
   * entries for variables which had a previous entry. */
  for (i = 0; i < count; i++)
    {
      if (seen[i])
	continue;
      sscanf(hp[i], "%s", var);

      /* Consider each entry for this variable (including hp[i]). */
      strcpy(vers, "0.0");
      *defaultval = 0;
      for (j = i; j < count; j++)
	{
	  *compvers = *flags = 0;
	  sscanf(hp[j], "%s %s %s %s", compvar, compval, compvers, flags);
	  if (strcmp(compvar, var) != 0)
	    continue;
	  seen[j] = 1;

	  /* If there's no version, keep this as the default value in case we
	   * don't come up with anything else to print.  If there is a version,
	   * discard it if it doesn't match the current workstation version and
	   * (a) we're not autoupdate, or (b) it's a testing version.  If we
	   * do consider the entry, and its version is greater than the current
	   * best version we have, update the current best version and its
	   * value. */
	  if (!*compvers)
	    {
	      strcpy(defaultval, compval);
	    }
	  else if (((autoupdate && !strchr(flags, 't')) ||
		    (vercmp(compvers, ws_version) == 0)))
	    {
	      if (vercmp(compvers, vers) >= 0)
		{
		  strcpy(val, compval);
		  strcpy(vers, compvers);
		}
	    }
	  else
	    {
	      /* Discard this entry, but record most recent testing and
	       * production releases. */
	      if (strchr(flags, 't') && vercmp(compvers, new_testing) > 0)
		strcpy(new_testing, compvers);
	      if (!strchr(flags, 't') && vercmp(compvers, new_production) > 0)
		strcpy(new_production, compvers);
	    }
	}
      if (*vers != '0' || *defaultval)
	{
	  if (*vers == '0')
	    strcpy(val, defaultval);
	  upper(var);
	  output_var(var, val, bourneshell);
	}
    }

  if (vercmp(new_testing, ws_version) > 0)
    output_var("NEW_TESTING_RELEASE", new_testing, bourneshell);
  if (vercmp(new_production, ws_version) > 0)
    output_var("NEW_PRODUCTION_RELEASE", new_production, bourneshell);
  free(seen);
}

static void output_var(const char *var, const char *val, int bourneshell)
{
  if (bourneshell)
    printf("%s=%s ; export %s\n", var, val, var);
  else
    printf("setenv %s %s\n", var, val);
}

static void upper(char *v)
{
  while(*v)
    {
      *v = toupper(*v);
      v++;
    }
}

static int vercmp(const char *v1, const char *v2)
{
  int major1 = 0, minor1 = 0, major2 =0, minor2 = 0;

  sscanf(v1, "%d.%d", &major1, &minor1);
  sscanf(v2, "%d.%d", &major2, &minor2);
  return((major1 != major2) ? (major1 - major2) : (minor1 - minor2));
}
