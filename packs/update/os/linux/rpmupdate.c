/* Copyright 2000 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* rpmupdate - Update a workstation from an old list of RPMs to a new
 * list, applying the update policies of a public or private
 * workstation as indicated by the flags.
 */

static const char rcsid[] = "$Id: rpmupdate.c,v 1.9 2000-08-22 05:34:07 ghudson Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <rpmlib.h>
#include <misc.h>	/* From /usr/include/rpm */

#define HASHSIZE 1009

struct rev {
  int present;			/* If 0, rest of structure is invalid */
  int epoch;
  char *version;
  char *release;
};

struct package {
  char *pkgname;
  char *filename;		/* File name in new list */
  struct rev oldlistrev;
  struct rev newlistrev;
  struct rev instrev;		/* Most recent version installed */
  int erase;
  struct package *next;
};

struct notify_data {
  FD_t fd;
  int hashmarks_flag;
  int hashmarks_printed;
};

enum act { UPDATE, ERASE, NONE };

static char *progname;

static void read_old_list(struct package **pkgtab, const char *oldlistname);
static void read_new_list(struct package **pkgtab, const char *newlistname);
static void read_installed_versions(struct package **pkgtab);
static void perform_updates(struct package **pkgtab, int public, int dryrun,
			    int hashmarks);
static void *notify(const Header h, const rpmCallbackType what,
		    const unsigned long amount, const unsigned long total,
		    const void *pkgKey, void *data);
static enum act decide_public(struct package *pkg);
static enum act decide_private(struct package *pkg);
static void schedule_update(struct package *pkg, rpmTransactionSet rpmdep);
static void display_action(struct package *pkg, enum act action);
static void update_lilo(struct package *pkg);
static char *fudge_arch_in_filename(char *filename);
static void printrev(struct rev *rev);
static int revcmp(struct rev *rev1, struct rev *rev2);
static int revsame(struct rev *rev1, struct rev *rev2);
static void freerev(struct rev *rev);
static void parse_line(const char *path, char **pkgname, int *epoch,
		       char **version, char **release, char **filename);
struct package *get_package(struct package **table, const char *pkgname);
static unsigned int hash(const char *str);
const char *find_back(const char *start, const char *end, char c);
static void *emalloc(size_t size);
static void *erealloc(void *ptr, size_t size);
static char *estrdup(const char *s);
char *estrndup(const char *s, size_t n);
static int read_line(FILE *fp, char **buf, int *bufsize);
static void die(const char *fmt, ...);
static void usage(void);

int main(int argc, char **argv)
{
  struct package *pkgtab[HASHSIZE];
  int i, c, public = 0, dryrun = 0, hashmarks = 0;
  const char *oldlistname, *newlistname;

  /* Initialize rpmlib. */
  rpmReadConfigFiles(NULL, NULL);

  /* Parse options. */
  rpmSetVerbosity(RPMMESS_NORMAL);
  progname = strrchr(argv[0], '/');
  progname = (progname == NULL) ? argv[0] : progname + 1;
  while ((c = getopt(argc, argv, "hnpv")) != EOF)
    {
      switch (c)
	{
	case 'h':
	  hashmarks = 1;
	  break;
	case 'n':
	  dryrun = 1;
	  break;
	case 'p':
	  public = 1;
	  break;
	case 'v':
	  rpmIncreaseVerbosity();
	  break;
	case '?':
	  usage();
	}
    }
  argc -= optind;
  argv += optind;
  if (argc != 2)
    usage();
  oldlistname = argv[0];
  newlistname = argv[1];

  /* Initialize the package hash table. */
  for (i = 0; i < HASHSIZE; i++)
    pkgtab[i] = NULL;

  /* Read the lists and the current versions into the hash table. */
  read_old_list(pkgtab, oldlistname);
  read_new_list(pkgtab, newlistname);
  read_installed_versions(pkgtab);

  /* Walk the table and perform the required updates. */
  perform_updates(pkgtab, public, dryrun, hashmarks);

  if (!dryrun)
    update_lilo(get_package(pkgtab, "kernel"));

  exit(0);
}

static void read_old_list(struct package **pkgtab, const char *oldlistname)
{
  FILE *fp;
  char *buf = NULL, *pkgname, *version, *release;
  int bufsize, epoch;
  struct package *pkg;

  fp = fopen(oldlistname, "r");
  if (!fp)
    die("Can't read old list %s", oldlistname);

  while (read_line(fp, &buf, &bufsize) == 0)
    {
      parse_line(buf, &pkgname, &epoch, &version, &release, NULL);
      pkg = get_package(pkgtab, pkgname);
      pkg->oldlistrev.present = 1;
      pkg->oldlistrev.epoch = epoch;
      pkg->oldlistrev.version = version;
      pkg->oldlistrev.release = release;
      free(pkgname);
    }
  free(buf);
}

static void read_new_list(struct package **pkgtab, const char *newlistname)
{
  FILE *fp;
  char *buf = NULL, *pkgname, *version, *release, *filename;
  int bufsize, epoch;
  struct package *pkg;

  fp = fopen(newlistname, "r");
  if (!fp)
    die("Can't read old list %s", newlistname);

  while (read_line(fp, &buf, &bufsize) == 0)
    {
      parse_line(buf, &pkgname, &epoch, &version, &release, &filename);
      pkg = get_package(pkgtab, pkgname);
      pkg->filename = filename;
      pkg->newlistrev.present = 1;
      pkg->newlistrev.epoch = epoch;
      pkg->newlistrev.version = version;
      pkg->newlistrev.release = release;
      free(pkgname);
    }
  free(buf);
}

static void read_installed_versions(struct package **pkgtab)
{
  Header h;
  int offset;
  char *pkgname, *version, *release;
  struct package *pkg;
  rpmdb db;
  int_32 *epoch;
  struct rev rev;

  if (rpmdbOpen(NULL, &db, O_RDONLY, 0644))
    die("Can't open RPM database for reading");

  for (offset = rpmdbFirstRecNum(db);
       offset != 0;
       offset = rpmdbNextRecNum(db, offset))
    {
      h = rpmdbGetRecord(db, offset);
      if (h == NULL)
	die("Failed to read database record\n");
      headerGetEntry(h, RPMTAG_NAME, NULL, (void **) &pkgname, NULL);
      if (!headerGetEntry(h, RPMTAG_EPOCH, NULL, (void **) &epoch, NULL))
	epoch = NULL;
      headerGetEntry(h, RPMTAG_VERSION, NULL, (void **) &version, NULL);
      headerGetEntry(h, RPMTAG_RELEASE, NULL, (void **) &release, NULL);

      /* Two versions of the same package can be installed on a system
       * with some coercion.  If so, make sure that instrev gets set to
       * the highest one.
       */
      rev.present = 1;
      rev.epoch = (epoch == NULL) ? -1 : *epoch;
      rev.version = estrdup(version);
      rev.release = estrdup(release);
      pkg = get_package(pkgtab, pkgname);
      if (!pkg->instrev.present)
	pkg->instrev = rev;
      else if (revcmp(&rev, &pkg->instrev) > 0)
	{
	  freerev(&pkg->instrev);
	  pkg->instrev = rev;
	}
      else
	freerev(&rev);

      headerFree(h);
    }

  rpmdbClose(db);
}

static void perform_updates(struct package **pkgtab, int public, int dryrun,
			    int hashmarks)
{
  int r, i, offset;
  struct package *pkg;
  rpmdb db;
  rpmTransactionSet rpmdep;
  rpmProblemSet probs = NULL;
  struct rpmDependencyConflict *conflicts;
  int nconflicts;
  Header h;
  enum act action;
  char *pkgname;
  struct notify_data ndata;

  if (!dryrun)
    {
      if (rpmdbOpen(NULL, &db, O_RDWR, 0644))
	die("Can't open RPM database for writing");
      rpmdep = rpmtransCreateSet(db, NULL);
    }

  /* Decide what to do for each package.  Add updates to the
   * transaction set.  Flag erasures for the next bit of code.
   * If we're doing a dry run, say what we're going to do here.
   */
  for (i = 0; i < HASHSIZE; i++)
    {
      for (pkg = pkgtab[i]; pkg; pkg = pkg->next)
	{
	  if (public)
	    action = decide_public(pkg);
	  else
	    action = decide_private(pkg);
	  if (dryrun)
	    display_action(pkg, action);
	  else if (action == UPDATE)
	    schedule_update(pkg, rpmdep);
	  else if (action == ERASE)
	    pkg->erase = 1;
	}
    }

  if (dryrun)
    return;

  /* Walk through the database and add transactions to erase packages
   * which we've flagged for erasure.  We do erasures this way
   * (instead of remembering the offset in read_installed_versions())
   * because the database offsets might have changed since we read the
   * versions and because this way if a package we want to erase is
   * installed at two different versions on the system, we will remove
   * both packages.
   */
  for (offset = rpmdbFirstRecNum(db);
       offset != 0;
       offset = rpmdbNextRecNum(db, offset))
    {
      h = rpmdbGetRecord(db, offset);
      headerGetEntry(h, RPMTAG_NAME, NULL, (void **) &pkgname, NULL);
      pkg = get_package(pkgtab, pkgname);
      if (pkg->erase)
	{
	  /* What we'd really like to do is display hashmarks while
	   * we're actually removing the package.  But librpm doesn't
	   * issue callbacks for erased RPMs, so we can't do that
	   * currently.  Instead, tell people what packages we're
	   * going to erase.
	   */
	  if (hashmarks)
	    printf("Scheduling removal of package %s\n", rpmtrans); 
	  rpmtransRemovePackage(rpmdep, offset);
	}
      headerFree(h);
    }

  /* The transaction set is complete.  Check for dependency problems. */
  if (rpmdepCheck(rpmdep, &conflicts, &nconflicts) != 0)
    exit(1);
  if (conflicts)
    {
      fprintf(stderr, "Update would break dependencies:\n");
      printDepProblems(stderr, conflicts, nconflicts);
      rpmdepFreeConflicts(conflicts, nconflicts);
      exit(1);
    }

  ndata.hashmarks_flag = hashmarks;
  ndata.hashmarks_printed = 0;
  r = rpmRunTransactions(rpmdep, notify, &ndata, NULL, &probs, 0,
			 RPMPROB_FILTER_OLDPACKAGE|RPMPROB_FILTER_REPLACEPKG);
  if (r < 0)
    die("Failed to run transactions\n");
  else if (r > 0)
    {
      fprintf(stderr, "Update failed due to the following problems:\n");
      rpmProblemSetPrint(stderr, probs);
      exit(1);
    }

  rpmdbClose(db);
}

/* Callback function for rpmRunTransactions. */
static void *notify(const Header h, const rpmCallbackType what,
		    const unsigned long amount, const unsigned long total,
		    const void *pkgKey, void *data)
{
  const char *filename = pkgKey;
  struct notify_data *ndata = data;
  int n;

  switch (what)
    {
    case RPMCALLBACK_INST_OPEN_FILE:
      ndata->fd = fdOpen(filename, O_RDONLY, 0);
      return ndata->fd;

    case RPMCALLBACK_INST_CLOSE_FILE:
      fdClose(ndata->fd);
      return NULL;

    case RPMCALLBACK_INST_START:
      if (ndata->hashmarks_flag)
	{
	  ndata->hashmarks_printed = 0;
	  printf("%-28s", headerSprintf(h, "%{NAME}", rpmTagTable,
					rpmHeaderFormats, NULL));
	  fflush(stdout);
	}
      return NULL;

    case RPMCALLBACK_INST_PROGRESS:
      if (ndata->hashmarks_flag)
	{
	  n = (amount == total) ? 50 : 50.0 * amount / total;
	  for (; ndata->hashmarks_printed < n; ndata->hashmarks_printed++)
	    putchar('#');
	  if (amount == total)
	    putchar('\n');
	  fflush(stdout);
	}
      return NULL;

    default:
      return NULL;
    }
}

/* Apply the public workstation rules to decide what to do with a
 * package.
 */
static enum act decide_public(struct package *pkg)
{
  /* Erase any installed package which isn't in the new list. */
  if (pkg->instrev.present && !pkg->newlistrev.present)
    return ERASE;

  /* Update any new list package which isn't installed at the new list rev. */
  if (pkg->newlistrev.present && !revsame(&pkg->instrev, &pkg->newlistrev))
    return UPDATE;

  return NONE;
}

/* Apply the private workstation rules to decide what to do with a
 * package.
 */
static enum act decide_private(struct package *pkg)
{
  /* If the installed package state is the same as it was in the old
   * list, then apply the public workstation rules to this package,
   * since we were the last ones to touch it.
   */
  if (revsame(&pkg->instrev, &pkg->oldlistrev))
    return decide_public(pkg);

  /* Otherwise, there has been a local change of some sort to the
   * package.  Update the package if it is installed and we have a
   * new version, but don't touch it otherwise.
   */
  if (pkg->instrev.present && pkg->newlistrev.present
      && revcmp(&pkg->newlistrev, &pkg->instrev) > 0)
    return UPDATE;

  return NONE;
}

/* Read the header from an RPM file and add an update transaction for it. */
static void schedule_update(struct package *pkg, rpmTransactionSet rpmdep)
{
  Header h;
  FD_t fd;

  assert(pkg->filename != NULL);
  pkg->filename = fudge_arch_in_filename(pkg->filename);
  fd = fdOpen(pkg->filename, O_RDONLY, 0);
  if (fd == NULL)
    die("Can't read package file %s", pkg->filename);
  if (rpmReadPackageHeader(fd, &h, NULL, NULL, NULL) != 0)
    die("Invalid rpm header in file %s", pkg->filename);
  if (rpmtransAddPackage(rpmdep, h, NULL, pkg->filename, 1, NULL) != 0)
    die("Can't install or update package %s", pkg->pkgname);
  fdClose(fd);
  headerFree(h);
}

static void display_action(struct package *pkg, enum act action)
{
  switch (action)
    {
    case ERASE:
      printf("Erase package %s\n", pkg->pkgname);
      break;
    case UPDATE:
      if (pkg->instrev.present)
	{
	  printf("Update package %s from rev ", pkg->pkgname);
	  printrev(&pkg->instrev);
	  printf(" to rev ");
	  printrev(&pkg->newlistrev);
	  printf("\n");
	}
      else
	{
	  printf("Install package %s at rev ", pkg->pkgname);
	  printrev(&pkg->newlistrev);
	  printf("\n");
	}
      break;
    default:
      break;
    }
}

/* Red Hat's kernel package doesn't take care of lilo.conf; the update
 * agent does.  So we have to take care of it too.
 */
static void update_lilo(struct package *pkg)
{
  const char *name = "/etc/lilo.conf", *savename = "/etc/lilo.conf.rpmsave";
  FILE *in, *out;
  char *buf = NULL, *oldkname, *newkname, *p;
  int bufsize = 0, status, replaced;
  struct stat statbuf;

  /* For now, only act on updates. */
  if (!pkg->instrev.present || !pkg->newlistrev.present
      || revsame(&pkg->instrev, &pkg->newlistrev))
    return;

  /* Figure out kernel names. */
  oldkname = emalloc(strlen("/boot/vmlinuz-")
		     + strlen(pkg->instrev.version) + strlen("-")
		     + strlen(pkg->instrev.release) + 1);
  sprintf(oldkname, "/boot/vmlinuz-%s-%s", pkg->instrev.version,
	  pkg->instrev.release);
  newkname = emalloc(strlen("/boot/vmlinuz-")
		     + strlen(pkg->newlistrev.version) + strlen("-")
		     + strlen(pkg->newlistrev.release) + 1);
  sprintf(newkname, "/boot/vmlinuz-%s-%s", pkg->newlistrev.version,
	  pkg->newlistrev.release);

  if (stat(name, &statbuf) == -1)
    die("Can't stat lilo.conf for rewrite.");

  /* This isn't very atomic, but it's what Red Hat's update agent does. */
  if (rename(name, savename) == -1)
    die("Can't rename /etc/lilo.conf to /etc/lilo.conf.rpmsave.");
  in = fopen(savename, "r");
  out = fopen(name, "w");
  if (!in || !out)
    {
      rename(savename, name);
      die("Can't open lilo.conf files for rewrite.");
    }
  fchmod(fileno(out), statbuf.st_mode & 0777);

  /* Rewrite lilo.conf, changing the old kernel name to the new kernel
   * name in "image" lines.  Remove "initrd" lines following images we
   * replace, since we don't currently use a ramdisk.
   */
  replaced = 0;
  while ((status = read_line(in, &buf, &bufsize)) == 0)
    {
      p = buf;
      while (isspace(*p))
	p++;
      if (strncmp(p, "image", 5) == 0 && !isalpha(p[5]))
	{
	  p = strstr(buf, oldkname);
	  if (p != NULL)
	    {
	      fprintf(out, "%.*s%s%s\n", p - buf, buf, newkname,
		      p + strlen(oldkname));
	      replaced = 1;
	      continue;
	    }
	  else
	    replaced = 0;
	}
      else if (replaced && strncmp(p, "initrd", 6) == 0 && !isalpha(p[6]))
	continue;
      fprintf(out, "%s\n", buf);
    }
  fclose(in);
  if (status == -1 || ferror(out) || fclose(out) == EOF)
    {
      rename(savename, name);
      die("Error rewriting lilo.conf: %s", strerror(errno));
    }

  system("/sbin/lilo");
}

/* If filename has an arch string which is too high for this machine's
 * architecture, replace it with a new filename containing this
 * machine's architecture.  filename must be an allocated string which
 * can be freed and replaced.
 */
static char *fudge_arch_in_filename(char *filename)
{
  static const char *arches[] = { "i386", "i486", "i586", "i686", NULL };
  const char *p;
  int i, j, len;
  struct utsname buf;
  char *newfile;

  /* Find the beginning of the arch string in filename. */
  p = find_back(filename, filename + strlen(filename), '.');
  assert(p != NULL);
  p = find_back(filename, p, '.');
  assert(p != NULL);
  p++;

  /* Locate this architecture in the array.  If it's not one we recognize,
   * or if it's the least common denominator, leave well enough alone.
   */
  for (i = 0; arches[i] != NULL; i++)
    {
      len = strlen(arches[i]);
      if (strncmp(p, arches[i], len) == 0 && *(p + len) == '.')
	break;
    }
  if (i == 0 || arches[i] == NULL)
    return filename;

  /* Locate this machine's architecture in the array.  If we don't
   * recognize it, or if it's at least as high as the filename's
   * architecture, don't touch anything. */
  assert(uname(&buf) == 0);
  for (j = 0; arches[j] != NULL; j++)
    {
      if (strcmp(buf.machine, arches[j]) == 0)
	break;
    }
  if (j >= i)
    return filename;

  /* We have to downgrade the architecture of the filename.  Make a
   * new string and free the old one.
   */
  newfile = malloc(strlen(filename) - strlen(arches[i]) +
		   strlen(arches[j]) + 1);
  sprintf(newfile, "%.*s%s%s", p - filename, filename, arches[j],
	  p + strlen(arches[i]));
  free(filename);
  return newfile;
}

static void printrev(struct rev *rev)
{
  assert(rev->present);
  if (rev->epoch != -1)
    printf("%d:", rev->epoch);
  printf("%s-%s", rev->version, rev->release);
}

/* Compare two present revisions to see which is greater.  Returns 1 if
 * rev1 is greater, -1 if rev2 is greater, or 0 if they are the same.
 */
static int revcmp(struct rev *rev1, struct rev *rev2)
{
  int r;

  assert(rev1->present && rev2->present);
  if (rev1->epoch != rev2->epoch)
    return (rev1->epoch > rev2->epoch) ? 1 : -1;
  else if ((r = rpmvercmp(rev1->version, rev2->version)) != 0)
    return r;
  else
    return rpmvercmp(rev1->release, rev2->release);
}

/* Compare two revisions (either of which might not be present) to see
 * if they're the same.
 */
static int revsame(struct rev *rev1, struct rev *rev2)
{
  if (rev1->present != rev2->present)
    return 0;
  else if (!rev1->present && !rev2->present)
    return 1;
  else
    return (revcmp(rev1, rev2) == 0);
}

static void freerev(struct rev *rev)
{
  assert(rev->present);
  free(rev->version);
  free(rev->release);
}

/* Get package name and version information from a path line.  It
 * would be more robust to look inside the package, but that's too
 * inefficient when the packages live in AFS.  So assume that the
 * trailing component of the path name is
 * "pkgname-version-release.arch.rpm".  The RPM name may be followed
 * by an epoch number if the RPM has one.
 *
 * The values set in *pkgname, *version, and *release are malloc'd and
 * become the responsibility of the caller.
 */
static void parse_line(const char *line, char **pkgname, int *epoch,
		       char **version, char **release, char **filename)
{
  const char *end, *p;
  const char *pkgstart, *pkgend, *verstart, *verend, *relstart, *relend;

  /* See if there's whitespace, which would be followed by an epoch. */
  end = line;
  while (*end && !isspace(*end))
    end++;
  if (*end)
    {
      p = end;
      while (isspace(*p))
	p++;
      *epoch = atoi(p);
    }
  else
    *epoch = -1;

  /* The package name starts after the last slash in the path, if any. */
  pkgstart = find_back(line, end, '/');
  pkgstart = (pkgstart == NULL) ? line : pkgstart + 1;

  /* The release string ends at the second to last period after pkgstart. */
  relend = strrchr(pkgstart, '.');
  if (!relend)
    die("Found malformed RPM path line %s", line);
  relend = find_back(pkgstart, relend, '.');
  if (!relend)
    die("Found malformed RPM path line %s", line);

  /* The release string starts after the dash before relend. */
  relstart = find_back(pkgstart, relend, '-');
  if (!relstart)
    die("Found malformed RPM path line %s", line);
  relstart++;

  /* The version string ends at that dash. */
  verend = relstart - 1;

  /* The version string starts after the dash before verend. */
  verstart = find_back(pkgstart, verend, '-');
  if (!verstart)
    die("Found malformed RPM path line %s", line);
  verstart++;

  /* The package name ends at that dash. */
  pkgend = verstart - 1;

  *pkgname = estrndup(pkgstart, pkgend - pkgstart);
  *version = estrndup(verstart, verend - verstart);
  *release = estrndup(relstart, relend - relstart);
  if (filename)
    *filename = estrndup(line, end - line);
}

struct package *get_package(struct package **table, const char *pkgname)
{
  struct package **pkgptr, *pkg;

  /* Look through the hash bucket for a package with the right name. */
  for (pkgptr = &table[hash(pkgname)]; *pkgptr; pkgptr = &(*pkgptr)->next)
    {
      pkg = *pkgptr;
      if (strcmp(pkg->pkgname, pkgname) == 0)
	return pkg;
    }

  /* Create a new package and chain it in. */
  pkg = emalloc(sizeof(struct package));
  pkg->pkgname = estrdup(pkgname);
  pkg->filename = NULL;
  pkg->oldlistrev.present = 0;
  pkg->newlistrev.present = 0;
  pkg->instrev.present = 0;
  pkg->erase = 0;
  pkg->next = NULL;
  *pkgptr = pkg;
  return pkg;
}

/* Algorithm by Peter J. Weinberger.  Uses a hard-wired modulus for now. */
static unsigned int hash(const char *str)
{
  unsigned int hashval, g;

  hashval = 0;
  for (; *str; str++)
    {
      hashval = (hashval << 4) + *str;
      if ((g = hashval & 0xf0000000) != 0)
	{
	  hashval ^= g >> 24;
	  hashval ^= g;
	}
    }
  return hashval % HASHSIZE;
}

/* Find c in the range start..end-1, or return NULL if it isn't there. */
const char *find_back(const char *start, const char *end, char c)
{
  const char *p;

  for (p = end - 1; p >= start; p--)
    {
      if (*p == c)
	return p;
    }
  return NULL;
}

static void *emalloc(size_t size)
{
  void *ptr;

  ptr = malloc(size);
  if (!ptr)
    die("malloc size %lu failed", (unsigned long) size);
  return ptr;
}

static void *erealloc(void *ptr, size_t size)
{
  ptr = realloc(ptr, size);
  if (!ptr)
    die("realloc size %lu failed", (unsigned long) size);
  return ptr;
}

static char *estrdup(const char *s)
{
  char *new_s;

  new_s = emalloc(strlen(s) + 1);
  strcpy(new_s, s);
  return new_s;
}

char *estrndup(const char *s, size_t n)
{
  char *new_s;

  new_s = emalloc(n + 1);
  memcpy(new_s, s, n);
  new_s[n] = 0;
  return new_s;
}

/* Read a line from a file into a dynamically allocated buffer,
 * zeroing the trailing newline if there is one.  The calling routine
 * may call read_line multiple times with the same buf and bufsize
 * pointers; *buf will be reallocated and *bufsize adjusted as
 * appropriate.  The initial value of *buf should be NULL.  After the
 * calling routine is done reading lines, it should free *buf.  This
 * function returns 0 if a line was successfully read, 1 if the file
 * ended, and -1 if there was an I/O error.
 */

static int read_line(FILE *fp, char **buf, int *bufsize)
{
  char *newbuf;
  int offset = 0, len;

  if (*buf == NULL)
    {
      *buf = emalloc(128);
      *bufsize = 128;
    }

  while (1)
    {
      if (!fgets(*buf + offset, *bufsize - offset, fp))
	return (offset != 0) ? 0 : (ferror(fp)) ? -1 : 1;
      len = offset + strlen(*buf + offset);
      if ((*buf)[len - 1] == '\n')
	{
	  (*buf)[len - 1] = 0;
	  return 0;
	}
      offset = len;

      /* Allocate more space. */
      newbuf = erealloc(*buf, *bufsize * 2);
      *buf = newbuf;
      *bufsize *= 2;
    }
}

static void die(const char *fmt, ...)
{
  va_list ap;

  fprintf(stderr, "%s: ", progname);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
  exit(1);
}

static void usage()
{
  fprintf(stderr, "Usage: %s [-p] oldlist newlist\n", progname);
  exit(1);
}
