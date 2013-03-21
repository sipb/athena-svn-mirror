#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <hesiod.h>

#ifndef INADDR_NONE
#define INADDR_NONE ((unsigned long) -1)
#endif

/* Delay attaching new packs by up to four hours. */
#define UPDATE_INTERVAL (3600 * 4)

static void usage(void);
static void die(const char *f, ...);
static void shellenv(char **hp, const char *ws_version, int bourneshell, int plaintext);
static void output_var(const char *var, const char *val, int bourneshell, int plaintext);
static void upper(char *v);
static char **readcluster(FILE *f);
static char **merge(char **l1, char **l2);
static int vercmp(const char *v1, const char *v2);
static void *emalloc(size_t size);

/* Make a hesiod cluster query for the machine you are on and produce a
 * set of environment variable assignments for the C shell or the Bourne
 * shell, depending on the '-b' flag.
 * If a localfile or a fallbackfile is present, and the -h (hostname)
 * option is not given, read cluster information from it as well.
 * Variables in localfile override variables obtained from Hesiod, and
 * variables obtained from Hesiod override those in fallbackfile.
 * Under linux, the hesiod lookup is retried for generic cluster info
 * if no fallback cluster info is present.
 * 
 * "Override" means that the presence of any instances of variable "foo"
 * in one source will prevent any instances from a source being overridden.
 * Example 1:
 *   localfile: lpr myprinter
 *   Hesiod:    syslib random-syspack-1 9.0
 *              syslib random-syspack-2 9.1
 *              syslib old-random-syspack
 *   fallback:  syslib normal-syspack
 *
 * lpr would be myprinter, and syslib would be negotiated among the three
 * Hesiod entries.
 *
 * Example 2:
 *   localfile: lpr myprinter
 *              syslib new-spiffy-syspack
 *   Hesiod:    syslib random-syspack-1 9.0
 *              syslib random-syspack-2 9.1
 *              syslib old-random-syspack
 *   fallback:  syslib normal-syspack
 *
 * lpr would be myprinter, and syslib would be new-spiffy-syspack, regardless
 * of the given version.
 *
 * 
 * If any stdio errors, truncate standard output to 0 and return an exit
 * status.
 */

int main(int argc, char **argv)
{
  char **hp = NULL, **fp = NULL, **lp = NULL, **or1, **or2;
  int debug = 0, bourneshell = 0, plaintext = 0, ch;
  const char *fallbackfile = SYSCONFDIR "/cluster.fallback";
  const char *localfile = SYSCONFDIR "/cluster.local";
  const char *clusterfile = SYSCONFDIR "/cluster";
  const char *hostname = NULL, *version;
  char hostbuf[1024];
  FILE *f;
  void *hescontext;
  extern int optind;
  extern char *optarg;

  while ((ch = getopt(argc, argv, "bdpl:f:h:")) != -1)
    {
      switch (ch)
	{
	case 'd':
	  debug = 1;
	  break;
	case 'b':
	  bourneshell = 1;
	  break;
	case 'p':
	  plaintext = 1;
	  break;
	case 'f':
	  /* Deprecated option, for compatibility. */
	  fallbackfile = optarg;
	  break;
	case 'l':
	  /* Deprecated option, for compatibility */
	  localfile = optarg;
	  break;
	case 'h':
	  hostname = optarg;
	  break;
	default:
	  usage();
	}
    }
  argc -= optind;
  argv += optind;

  /* For compatibility, allow two arguments, but ignore the first one. */
  if (argc != 1 && argc != 2)
    usage();
  version = (argc == 2) ? argv[1] : argv[0];

  if (bourneshell && plaintext)
    usage();

  if (hostname == NULL)
    {
      /* We only look at the cluster, fallback, and local files when not
       * given a hostname.
       */
      f = fopen(clusterfile, "r");
      if (f)
	{
	  if (fgets(hostbuf, sizeof(hostbuf), f) == NULL)
	    die("Failed to read %s: %s", clusterfile, strerror(errno));
	  if (*hostbuf != '\0')
	    hostbuf[strlen(hostbuf) - 1] = '\0';
	  fclose(f);
	}
      else if (gethostname(hostbuf, sizeof(hostbuf)) != 0)
	die("Can't get hostname: %s", strerror(errno));
      hostname = hostbuf;

      f = fopen(fallbackfile, "r");
      if (f)
	{
	  fp = readcluster(f);
	  fclose(f);
	}

      f = fopen(localfile, "r");
      if (f)
	{
	  lp = readcluster(f);
	  fclose(f);
	}
    }

  if (debug)
    {
      /* Get clusterinfo records from standard input. */
      hp = readcluster(stdin);
    }
  else
    {
      /* Get clusterinfo records from Hesiod. */
      if (hesiod_init(&hescontext) != 0)
	perror("hesiod_init");
      else
	{
	  hp = hesiod_resolve(hescontext, hostname, "cluster");
	  if (hp == NULL && errno != ENOENT)
	    perror("hesiod_resolve");
#ifdef linux
	  if (hp == NULL && errno == ENOENT && fp == NULL) {
	    /* Fetch generic hesiod data only in the case of an explicit ENOENT
	     * and no local fallback data.
	     */
	    hp = hesiod_resolve(hescontext, "public-linux", "cluster");
	    if (hp == NULL && errno != ENOENT)
	      perror("hesiod_resolve");
	  }
#endif
	}
    }

  if (hp == NULL && lp == NULL && fp == NULL)
    {
      fprintf(stderr, "No cluster information available for %s\n", hostname);
      return 2;
    }

  or1 = merge(lp, hp);
  or2 = merge(or1, fp);
  shellenv(or2, version, bourneshell, plaintext);
  if (!debug)
    {
      if (hp != NULL)
	hesiod_free_list(hescontext, hp);
      hesiod_end(hescontext);
    }
  /* We don't bother to free memory we know we allocated just before exiting;
   * it's not worth the trouble. */
  return (ferror(stdout)) ? 1 : 0;
}

static char **merge(char **l1, char **l2)
{
  int size, point, i, j, ret, sizefroml1;
  char **nl;
  char var[256], compvar[256], dummy[256];

  if (l1 == NULL)
    return l2;
  if (l2 == NULL)
    return l1;

  size = 1;
  for (i = 0; l1[i] != NULL; i++)
    size++;
  for (i = 0; l2[i] != NULL; i++)
    size++;

  nl = emalloc(sizeof(char *) * size);
  point = 0;

  /* Copy l1 to nl. */
  for (i = 0; l1[i] != NULL; i++)
    {
      ret = sscanf(l1[i], "%s %s", var, dummy);
      if (ret == 2) /* Ignore invalid lines. */
	{
	  nl[point] = l1[i];
	  point++;
	}
    }
  sizefroml1 = point;

  /* For each entry in l2, add it to nl if nothing in l1 has that var. */
  for (i = 0; l2[i] != NULL; i++)
    {
      ret = sscanf(l2[i], "%s %s", var, dummy);
      if (ret < 2)
	continue; /* Ignore invalid lines. */
      for (j = 0; j < sizefroml1; j++)
	{
	  sscanf(nl[j], "%s", compvar);
	  if (strcmp(var, compvar) == 0)
	      break;
	}
      if (j == sizefroml1)
	{
	  nl[point] = l2[i];
	  point++;
	}
    }
  nl[point] = NULL;
  return nl;
}

static void usage(void)
{
  die("Usage: getcluster [-h hostname] [-b|-p] [-d] version");
  exit(1);
}

static void die(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  putc('\n', stderr);
  exit(1);
}

/* Cluster records will come in entries of the form:
 *
 *	variable value [version [flags]]
 *
 * There may be multiple records for the same variable, but not
 * multiple entries with the same variable and version.  There may be
 * at most one entry for a given variable that does not list a
 * version.  Records with a version less than the current workstation
 * version are ignored.
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
static void shellenv(char **hp, const char *ws_version, int bourneshell, int plaintext)
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
	    strcpy(defaultval, compval);
	  else if (vercmp(compvers, ws_version) < 0)
	    continue;
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
	  output_var(var, val, bourneshell, plaintext);
	}
    }

  if (vercmp(new_testing, ws_version) > 0)
    output_var("NEW_TESTING_RELEASE", new_testing, bourneshell, plaintext);
  if (vercmp(new_production, ws_version) > 0)
    output_var("NEW_PRODUCTION_RELEASE", new_production, bourneshell, plaintext);
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
      output_var("UPDATE_TIME", timebuf, bourneshell, plaintext);
    }
  free(seen);
}

static void output_var(const char *var, const char *val, int bourneshell, int plaintext)
{
  if (bourneshell)
    printf("%s=\"%s\" ; export %s ;\n", var, val, var);
  else if (plaintext)
    printf("%s %s\n", var, val);
  else
    printf("setenv %s \"%s\" ;\n", var, val);
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

static char **readcluster(FILE *f)
{
  char line[1024];
  char **lp;
  int nl, al;

  nl = 0;
  al = 10;
  lp = emalloc(al * sizeof(char *));

  lp[0] = NULL;
  while (fgets(line, 1024, f) != NULL)
    {
      if (nl + 1 == al)
	{
	  al = al * 2;
	  lp = realloc(lp, al * sizeof(char *));
	  if (lp == NULL)
	    {
	      fprintf(stderr, "Out of memory.");
	      exit(1);
	    }
	}
      lp[nl] = strdup(line);
      if (lp[nl] == NULL)
	{
	  fprintf(stderr, "Out of memory.");
	  exit(1);
	}
      nl++;
    }
  lp[nl] = NULL;

  return lp;
}

static void *emalloc(size_t size)
{
  void *p;

  p = malloc(size);
  if (p == NULL)
    {
      fprintf(stderr, "Out of memory.\n");
      exit(1);
    }
  return p;
}
