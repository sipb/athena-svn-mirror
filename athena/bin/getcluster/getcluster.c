#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <hesiod.h>

#ifndef INADDR_NONE
#define INADDR_NONE ((unsigned long) -1)
#endif

/* Delay attaching new packs by up to four hours. */
#define UPDATE_INTERVAL (3600 * 4)

extern int optind;

static void usage(void);
static void shellenv(char **hp, char *ws_version, int bourneshell);
static void output_var(const char *var, const char *val, int bourneshell);
static void upper(char *v);
static int vercmp(const char *v1, const char *v2);

/* Make a hesiod cluster query for the machine you are on and produce a
 * set of environment variable assignments for the C shell or the Bourne
 * shell, depending on the '-b' flag
 *
 * If any stdio errors, truncate standard output to 0 and return an exit
 * status.
 */

int main(int argc, char **argv)
{
  char buf[256], **hp, **hpp;
  int debug = 0, bourneshell = 0, ch;
  void *hescontext;

  while ((ch = getopt(argc, argv, "bd")) != -1)
    {
      switch (ch)
	{
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

  if (debug)
    {
      /* Get clusterinfo records from standard input. */
      hpp = (char **) malloc(100 * sizeof(char *));
      hp = hpp;
      while (fgets(buf, sizeof(buf), stdin) != NULL)
	{
	  *hpp = malloc(strlen(buf) + 1);
	  strcpy(*hpp, buf);
	  (*hpp)[strlen(buf) - 1] = 0;
	  hpp++;
	}
      *hpp = NULL;
    }
  else
    {
      /* Get clusterinfo records from Hesiod. */
      if (hesiod_init(&hescontext) != 0)
	{
	  perror("hesiod_init");
	  return 0;
	}
      hp = hesiod_resolve(hescontext, argv[0], "cluster");
      if (hp == NULL && errno == ENOENT)
	{
	  fprintf(stderr, "No Hesiod information available for %s\n", argv[0]);
	  return 2;
	}
      else if (hp == NULL)
	{
	  perror("hesiod_resolve");
	  return 1;
	}
    }

  shellenv(hp, argv[1], bourneshell);
  if (!debug)
    {
      hesiod_free_list(hescontext, hp);
      hesiod_end(hescontext);
    }
  return (ferror(stdout)) ? 1 : 0;
}

static void usage()
{
  fprintf(stderr, "Usage: getcluster [-b] [-d] hostname version\n");
  exit(1);
}

/* Cluster records will come in entries of the form:
 *
 *	variable value [version [flags]]
 *
 * There may be multiple records for the same variable, but not
 * multiple entries with the same variable and version.  There may be
 * at most one entry for a given variable that does not list a
 * version.
 *
 * Discard records if they have a version greater than the current
 * workstation version and any of the following is true:
 *	- They have 't' listed in the flags.
 *	- AUTOUPDATE is false.
 *	- The environment variable UPDATE_TIME doesn't exist or
 *	  specifies a time later than the current time.
 * Set NEW_TESTING_RELEASE if any records are discarded for the first
 * reason, NEW_PRODUCTION_RELEASE if any records are discarded for
 * the second reason, and UPDATE_TIME if any entries specify a version
 * greater than the current workstation version and are not discarded
 * for the first two reasons.
 *
 * After discarding records, output the variable definition with the
 * highest version number.  If there aren't any records left with
 * version numbers and there's one with no version number, output
 * that one. */
static void shellenv(char **hp, char *ws_version, int bourneshell)
{
  int *seen, count, i, j, output_time = 0, autoupdate = 0;
  char var[80], val[80], vers[80], flags[80], compvar[80], compval[80];
  char compvers[80], defaultval[80], new_production[80], new_testing[80];
  char timebuf[32], *envp;
  time_t update_time = -1, now;
  unsigned long ip = INADDR_NONE;

  time(&now);

  /* Gather information from the environment.  UPDATE_TIME comes from
   * the clusterinfo file we wrote out last time; AUTOUPDATE and ADDR
   * come from rc.conf.  If neither file has been sourced by the
   * caller, we just use defaults. */
  envp = getenv("UPDATE_TIME");
  if (envp)
    update_time = atoi(envp);
  envp = getenv("AUTOUPDATE");
  if (envp && strcmp(envp, "true") == 0)
    autoupdate = 1;
  envp = getenv("ADDR");
  if (envp)
    ip = inet_addr(envp);

  count = 0;
  while (hp[count])
    count++;

  seen = (int *) calloc(count, sizeof(int));
  if (seen == NULL)
    exit(1);

  strcpy(new_production, "0.0");
  strcpy(new_testing, "0.0");

  /* The outer loop is for the purpose of "considering each variable."
   * We skip entries for variables which had a previous entry. */
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

	  /* If there's no version, keep this as the default value in
	   * case we don't come up with anything else to print.  If
	   * there is a version, discard it if it doesn't match the
	   * current workstation version and (a) we're not autoupdate,
	   * or (b) it's a testing version.  If we do consider the
	   * entry, and its version is greater than the current best
	   * version we have, update the current best version and its
	   * value. */
	  if (!*compvers)
	    {
	      strcpy(defaultval, compval);
	    }
	  else if (((autoupdate && !strchr(flags, 't')) ||
		    (vercmp(compvers, ws_version) == 0)))
	    {
	      if (vercmp(compvers, ws_version) > 0)
		{
		  /* We want to take this value, but not necessarily
		   * right away.  Accept the record only if we have an
		   * update time which has already passed.  Make
		   * a note that we should output the time
		   * whether or not we discard the entry, since the
		   * workstation is out of date either way. */
		  output_time = 1;
		  if (update_time == -1 || now < update_time)
		    continue;
		}

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
  if (output_time)
    {
      /* If we have no time from the environment, make up one
       * between now and UPDATE_INTERVAL seconds in the future. */
      if (update_time == -1)
	{
	  srand(ntohl(ip));
	  update_time = now + rand() % UPDATE_INTERVAL;
	}
      sprintf(timebuf, "%lu", update_time);
      output_var("UPDATE_TIME", timebuf, bourneshell);
    }
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
  return (major1 != major2) ? (major1 - major2) : (minor1 - minor2);
}
