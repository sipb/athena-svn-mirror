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

static const char rcsid[] = "$Id: rpmupdate.c,v 1.29 2005-03-31 22:15:12 ghudson Exp $";

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/vfs.h>
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
#include <rpmdb.h>
#include <rpmts.h>
#include <rpmps.h>
#include <rpmds.h>
#include <misc.h>	/* From /usr/include/rpm */

#define HASHSIZE 1009
#define RPMCACHE "/var/athena/rpms"

enum act { UPDATE, ERASE, ELIMINATE_MULTIPLES, NONE };

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
  int multiple_installed_revs;
  int notouch;
  int only_upgrade;
  enum act action;
  struct package *next;
};

struct notify_data {
  FD_t fd;
  int total;
  int hashmarks_flag;
  int hashmarks_printed;
  int packages;			/* Packages installed so far */
  int npackages;		/* Total packages to install (not remove) */
  struct package **pkgtab;
};

static char *progname;

static void read_old_list(struct package **pkgtab, const char *oldlistname);
static void read_new_list(struct package **pkgtab, const char *newlistname);
static void read_upgrade_list(struct package **pkgtab, const char *listname);
static void read_exception_list(struct package **pkgtab, const char *listname);
static void read_installed_versions(struct package **pkgtab, rpmts ts);
static void decide_actions(struct package **pkgtab, int public);
static void perform_updates(struct package **pkgtab, rpmts ts, int depcheck,
			    int hashmarks, int copy);
static void *notify(const void *arg, rpmCallbackType what,
		    unsigned long amount, unsigned long total,
		    const void *pkgKey, void *data);
static void print_hash(struct notify_data *ndata, int amount);
static enum act decide_public(struct package *pkg);
static enum act decide_private(struct package *pkg);
static void schedule_update(struct package *pkg, rpmts ts, int copy);
static void display_actions(struct package **pkgtab);
static int kernel_was_updated(rpmts ts, struct package *pkg);
static void update_lilo(struct package *pkg);
static char *fudge_arch_in_filename(char *filename);
static void prepare_copy_area(void);
static void check_copy_size(struct package **pkgtab);
static char *copy_local(const char *filename);
static void clear_copy_area(void);
static void read_rev_from_header(struct rev *rev, Header h);
static void printrev(struct rev *rev);
static int revcmp(struct rev *rev1, struct rev *rev2);
static int revsame(struct rev *rev1, struct rev *rev2);
static void freerev(struct rev *rev);
static void parse_line(const char *path, char **pkgname, int *epoch,
		       char **version, char **release, char **filename);
struct package *get_package(struct package **table, const char *pkgname);
static unsigned int hash(const char *str);
static const char *find_back(const char *start, const char *end, char c);
static void *emalloc(size_t size);
static void *erealloc(void *ptr, size_t size);
static char *estrdup(const char *s);
static char *estrndup(const char *s, size_t n);
static int easprintf(char **ptr, const char *fmt, ...);
static int read_line(FILE *fp, char **buf, int *bufsize);
static void die(const char *fmt, ...);
static void usage(void);

int main(int argc, char **argv)
{
  struct package *pkgtab[HASHSIZE];
  int i, c, depcheck = 0, public = 0, dryrun = 0, hashmarks = 0, copy = 0;
  const char *oldlistname, *newlistname, *upgradelistname = NULL;
  rpmts ts;

  /* Initialize rpmlib. */
  rpmReadConfigFiles(NULL, NULL);

  /* Parse options. */
  rpmSetVerbosity(RPMMESS_NORMAL);
  progname = strrchr(argv[0], '/');
  progname = (progname == NULL) ? argv[0] : progname + 1;
  while ((c = getopt(argc, argv, "cdhnpu:v")) != EOF)
    {
      switch (c)
	{
	case 'c':
	  copy = 1;
	  break;
	case 'd':
	  depcheck = 1;
	  break;
	case 'h':
	  hashmarks = 1;
	  break;
	case 'n':
	  dryrun = 1;
	  break;
	case 'p':
	  public = 1;
	  break;
	case 'u':
	  upgradelistname = optarg;
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

  /* Work around a bug in rpm 4.2's rpmlib which allowed several of
   * Red Hat 9's packages to be built with missing epoch
   * specifications in requirements (gnome-libs-devel, ORBit-devel,
   * esound-devel, probably others).  The missing epochs are
   * mistakenly let through at install time, but are checked for when
   * we try to remove a package which Provides the same thing as the
   * Red Hat package.  See
   * https://bugzilla.redhat.com/bugzilla/show_bug.cgi?id=81965
   */
  { extern int _rpmds_nopromote; _rpmds_nopromote = 0; }

  /* Initialize the package hash table. */
  for (i = 0; i < HASHSIZE; i++)
    pkgtab[i] = NULL;

  /* Read the lists into the hash table. */
  read_old_list(pkgtab, oldlistname);
  read_new_list(pkgtab, newlistname);
  if (upgradelistname != NULL)
    read_upgrade_list(pkgtab, upgradelistname);
  if (!public)
    read_exception_list(pkgtab, SYSCONFDIR "/rpmupdate.exceptions");

  /* Create a transaction set and read the current versions into the table. */
  ts = rpmtsCreate();
  rpmtsSetRootDir(ts, "/");
  read_installed_versions(pkgtab, ts);

  /* Decide what updates to perform, and do them. */
  decide_actions(pkgtab, public);
  if (dryrun)
    display_actions(pkgtab);
  if (!dryrun || depcheck)
    perform_updates(pkgtab, ts, depcheck, hashmarks, copy);
  rpmtsFree(ts);

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
      if (pkg->oldlistrev.present)
	die("Duplicate package %s in old list %s", pkgname, oldlistname);
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
    die("Can't read new list %s", newlistname);

  while (read_line(fp, &buf, &bufsize) == 0)
    {
      parse_line(buf, &pkgname, &epoch, &version, &release, &filename);
      pkg = get_package(pkgtab, pkgname);
      if (pkg->newlistrev.present)
	die("Duplicate package %s in new list %s", pkgname, newlistname);
      pkg->filename = filename;
      pkg->newlistrev.present = 1;
      pkg->newlistrev.epoch = epoch;
      pkg->newlistrev.version = version;
      pkg->newlistrev.release = release;
      free(pkgname);
    }
  free(buf);
}

static void read_upgrade_list(struct package **pkgtab, const char *listname)
{
  FILE *fp;
  char *buf = NULL, *pkgname, *version, *release, *filename;
  int bufsize, epoch;
  struct package *pkg;

  fp = fopen(listname, "r");
  if (!fp)
    die("Can't read upgrade list %s", listname);

  while (read_line(fp, &buf, &bufsize) == 0)
    {
      parse_line(buf, &pkgname, &epoch, &version, &release, &filename);
      pkg = get_package(pkgtab, pkgname);
      if (pkg->newlistrev.present)
	die("Duplicate package %s in upgrade list %s", pkgname, listname);
      pkg->filename = filename;
      pkg->newlistrev.present = 1;
      pkg->newlistrev.epoch = epoch;
      pkg->newlistrev.version = version;
      pkg->newlistrev.release = release;
      pkg->only_upgrade = 1;
      free(pkgname);
    }
  free(buf);
}

static void read_exception_list(struct package **pkgtab, const char *listname)
{
  FILE *fp;
  char *buf = NULL;
  int bufsize;
  struct package *pkg;

  fp = fopen(listname, "r");
  if (!fp)
    return;

  while (read_line(fp, &buf, &bufsize) == 0)
    {
      pkg = get_package(pkgtab, buf);
      pkg->notouch = 1;
    }
  free(buf);
}

static void read_installed_versions(struct package **pkgtab, rpmts ts)
{
  rpmdbMatchIterator mi;
  Header h;
  char *pkgname;
  struct package *pkg;
  struct rev rev;

  mi = rpmtsInitIterator(ts, RPMDBI_PACKAGES, NULL, 0);
  if (mi == NULL)
    die("Failed to initialize database iterator");
  while ((h = rpmdbNextIterator(mi)) != NULL)
    {
      headerGetEntry(h, RPMTAG_NAME, NULL, (void **) &pkgname, NULL);
      read_rev_from_header(&rev, h);

      /* Two versions of the same package can be installed on a system
       * with some coercion.  If so, make sure that instrev gets set to
       * the highest one.
       */
      pkg = get_package(pkgtab, pkgname);
      if (!pkg->instrev.present)
	pkg->instrev = rev;
      else
	{
	  pkg->multiple_installed_revs = 1;
	  if (revcmp(&rev, &pkg->instrev) > 0)
	    {
	      freerev(&pkg->instrev);
	      pkg->instrev = rev;
	    }
	  else
	    freerev(&rev);
	}
    }
  rpmdbFreeIterator(mi);
}

static void decide_actions(struct package **pkgtab, int public)
{
  int i;
  struct package *pkg;

  for (i = 0; i < HASHSIZE; i++)
    {
      for (pkg = pkgtab[i]; pkg; pkg = pkg->next)
	{
	  if (pkg->notouch)
	    {
	      pkg->action = NONE;
	      continue;
	    }
	  pkg->action = (public) ? decide_public(pkg) : decide_private(pkg);

	  /* Multiple revs of an rpm is usually a sign of a previous
	   * failed update.  Clean up by eliminating the old revs.
	   */
	  if (pkg->action == NONE && pkg->multiple_installed_revs)
	    pkg->action = ELIMINATE_MULTIPLES;
	}
    }
}

static void perform_updates(struct package **pkgtab, rpmts ts, int depcheck,
			    int hashmarks, int copy)
{
  int r, i, npackages, any;
  struct package *pkg;
  rpmps ps;
  rpmdbMatchIterator mi;
  Header h;
  char *pkgname;
  struct notify_data ndata;
  struct rev rev;

  if (copy)
    {
      prepare_copy_area();
      check_copy_size(pkgtab);
    }

  /* Add updates to the transaction set.  Count package updates for
   * the notification display.  If there is nothing to do, quit now;
   * as of rpm 4.2, rpmlib considers it an error to attempt to run an
   * empty transaction set.
   */
  npackages = 0;
  any = 0;
  for (i = 0; i < HASHSIZE; i++)
    {
      for (pkg = pkgtab[i]; pkg; pkg = pkg->next)
	{
	  if (pkg->action == UPDATE)
	    {
	      schedule_update(pkg, ts, copy);
	      npackages++;
	    }
	  if (pkg->action != NONE)
	    any = 1;
	}
    }
  if (!any)
    return;

  /* Walk through the database and add transactions to erase packages
   * which we've flagged for erasure.  We do erasures this way
   * (instead of remembering the offset in read_installed_versions())
   * because the database offsets might have changed since we read the
   * versions and because this way if a package we want to erase is
   * installed at two different versions on the system, we will remove
   * both packages.
   */
  mi = rpmtsInitIterator(ts, RPMDBI_PACKAGES, NULL, 0);
  if (mi == NULL)
    die("Failed to initialize database iterator");
  while ((h = rpmdbNextIterator(mi)) != NULL)
    {
      if (!headerGetEntry(h, RPMTAG_NAME, NULL, (void **) &pkgname, NULL))
	continue;
      pkg = get_package(pkgtab, pkgname);
      if (pkg->action == ERASE)
	rpmtsAddEraseElement(ts, h, rpmdbGetIteratorOffset(mi));
      if (pkg->action == ELIMINATE_MULTIPLES)
	{
	  /* Erase anything which doesn't match the most current rev. */
	  read_rev_from_header(&rev, h);
	  if (revcmp(&rev, &pkg->instrev) != 0)
	    rpmtsAddEraseElement(ts, h, rpmdbGetIteratorOffset(mi));
	  freerev(&rev);
	}
    }
  rpmdbFreeIterator(mi);

  /* The transaction set is complete.  Check for dependency problems. */
  if (rpmtsCheck(ts) != 0)
    exit(1);
  ps = rpmtsProblems(ts);
  if (rpmpsNumProblems(ps) > 0)
    {
      fprintf(stderr, "Update would break dependencies:\n");
      rpmpsPrint(NULL, rpmtsProblems(ts));
      exit(1);
    }
  if (depcheck)
    return;

  rpmtsOrder(ts);

  /* Set up the notification callback. */
  ndata.hashmarks_flag = hashmarks;
  ndata.packages = 0;
  ndata.npackages = npackages;
  ndata.pkgtab = pkgtab;
  rpmtsSetNotifyCallback(ts, notify, &ndata);

  rpmtsClean(ts);
  r = rpmtsRun(ts, NULL, RPMPROB_FILTER_OLDPACKAGE|RPMPROB_FILTER_REPLACEPKG);
  if (copy)
    clear_copy_area();
  if (r < 0)
    {
      /* The kernel may have been upgraded; make sure to edit
       * lilo.conf if so.
       */
      if (kernel_was_updated(ts, get_package(pkgtab, "kernel")))
	update_lilo(get_package(pkgtab, "kernel"));
      rpmtsFree(ts);
      die("Failed to run transactions");
    }
  else if (r > 0)
    {
      fprintf(stderr, "Update failed due to the following problems:\n");
      rpmpsPrint(NULL, rpmtsProblems(ts));
      rpmtsFree(ts);
      exit(1);
    }

  update_lilo(get_package(pkgtab, "kernel"));
}

/* Callback function for rpmRunTransactions. */
static void *notify(const void *arg, rpmCallbackType what,
		    unsigned long amount, unsigned long total,
		    const void *pkgKey, void *data)
{
  Header h = (Header) arg;
  const char *filename = pkgKey;
  struct notify_data *ndata = data;
  struct package *pkg;

  /* Take care of the "important" callbacks. */
  switch (what)
    {
    case RPMCALLBACK_INST_OPEN_FILE:
      ndata->fd = fdOpen(filename, O_RDONLY, 0);
      return ndata->fd;

    case RPMCALLBACK_INST_CLOSE_FILE:
      fdClose(ndata->fd);
      return NULL;

    default:
      break;
    }

  /* Everything else does nothing unless -h was specified. */
  if (!ndata->hashmarks_flag)
    return NULL;

  /* Filter out uninst callbacks for old versions of packages we're
   * upgrading.
   */
  switch (what)
    {
    case RPMCALLBACK_UNINST_START:
    case RPMCALLBACK_UNINST_PROGRESS:
    case RPMCALLBACK_UNINST_STOP:
      pkg = get_package(ndata->pkgtab, headerSprintf(h, "%{NAME}", rpmTagTable,
						     rpmHeaderFormats, NULL));
      if (pkg && pkg->action == UPDATE)
	return NULL;

    default:
      break;
    }

  switch (what)
    {
    case RPMCALLBACK_TRANS_START:
      ndata->total = total;
      ndata->hashmarks_printed = 0;
      printf("%-28s", "Preparing:");
      break;

    case RPMCALLBACK_TRANS_PROGRESS:
      print_hash(ndata, amount);
      break;

    case RPMCALLBACK_TRANS_STOP:
      print_hash(ndata, ndata->total);
      putchar('\n');
      break;

    case RPMCALLBACK_INST_START:
    case RPMCALLBACK_UNINST_START:
      ndata->total = total;
      ndata->hashmarks_printed = 0;
      printf("%c ", (what == RPMCALLBACK_INST_START) ? '+' : '-');
      printf("%-26s", headerSprintf(h, "%{NAME}", rpmTagTable,
				    rpmHeaderFormats, NULL));
      fflush(stdout);
      break;

    case RPMCALLBACK_UNINST_PROGRESS:
      /* "amount" is the number of files left to delete, or was when
       * rpmlib bothered to make this callback.
       */
      print_hash(ndata, ndata->total - amount);
      break;

    case RPMCALLBACK_INST_PROGRESS:
      print_hash(ndata, amount);
      if (amount == total)
	{
	  ndata->packages++;
	  printf(" [%3d%%]\n",
		 (int)(100 * ((ndata->packages == ndata->npackages) ? 1
			      : (float) ndata->packages / ndata->npackages)));
	}
      break;

    case RPMCALLBACK_UNINST_STOP:
      print_hash(ndata, ndata->total);
      putchar('\n');
      break;

    default:
      break;
    }
  return NULL;
}

static void print_hash(struct notify_data *ndata, int amount)
{
  int n;

  n = 43 * ((amount == ndata->total) ? 1 : (float) amount / ndata->total);
  for (; ndata->hashmarks_printed < n; ndata->hashmarks_printed++)
    putchar('#');
  fflush(stdout);
}

/* Apply the public workstation rules to decide what to do with a
 * package.
 */
static enum act decide_public(struct package *pkg)
{
  /* Erase any installed package which isn't in the new list. */
  if (pkg->instrev.present && !pkg->newlistrev.present)
    return ERASE;

  /* Erase or don't install packages in the upgrade list. */
  if (pkg->only_upgrade)
    return (pkg->instrev.present) ? ERASE : NONE;

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
static void schedule_update(struct package *pkg, rpmts ts, int copy)
{
  Header h;
  FD_t fd;
  int status;

  assert(pkg->filename != NULL);
  pkg->filename = fudge_arch_in_filename(pkg->filename);
  if (copy)
    pkg->filename = copy_local(pkg->filename);
  fd = fdOpen(pkg->filename, O_RDONLY, 0);
  if (fd == NULL)
    die("Can't read package file %s", pkg->filename);
  status = rpmReadPackageFile(ts, fd, pkg->filename, &h);
  if (status == RPMRC_FAIL || status == RPMRC_NOTFOUND)
    die("Invalid package file %s", pkg->filename);
  if (rpmtsAddInstallElement(ts, h, pkg->filename, 1, NULL) != 0)
    die("Can't install or update package %s", pkg->pkgname);
  fdClose(fd);
  headerFree(h);
}

static void display_actions(struct package **pkgtab)
{
  int i;
  struct package *pkg;

  for (i = 0; i < HASHSIZE; i++)
    {
      for (pkg = pkgtab[i]; pkg; pkg = pkg->next)
	{
	  switch (pkg->action)
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
	    case ELIMINATE_MULTIPLES:
	      printf("Remove old versions of package %s\n", pkg->pkgname);
	      break;
	    default:
	      break;
	    }
	}
    }
}

/* If rpmRunTransactions() fails, we don't really know whether all,
 * some, or none of the updates happens.  So we have to examine the
 * database to find out whether the kernel was updated so that we know
 * whether to update lilo.
 */
static int kernel_was_updated(rpmts ts, struct package *pkg)
{
  rpmdbMatchIterator mi;
  Header h;
  char *version, *release;
  int_32 *epoch;
  struct rev rev;

  /* Don't bother checking if there was no kernel update. */
  if (!pkg->instrev.present || !pkg->newlistrev.present
      || revsame(&pkg->instrev, &pkg->newlistrev))
    return 0;

  mi = rpmtsInitIterator(ts, RPMDBI_LABEL, "kernel", 0);
  if (mi == NULL)
    die("Failed to initialize database iterator while recovering.");
  while ((h = rpmdbNextIterator(mi)) != NULL)
    {
      if (!headerGetEntry(h, RPMTAG_EPOCH, NULL, (void **) &epoch, NULL))
	epoch = NULL;
      headerGetEntry(h, RPMTAG_VERSION, NULL, (void **) &version, NULL);
      headerGetEntry(h, RPMTAG_RELEASE, NULL, (void **) &release, NULL);
      rev.present = 1;
      rev.epoch = (epoch == NULL) ? -1 : *epoch;
      rev.version = version;
      rev.release = release;
    }
  rpmdbFreeIterator(mi);
  return (rev.present && revcmp(&rev, &pkg->newlistrev) == 0);
}

/* Red Hat's kernel package doesn't take care of lilo.conf; the update
 * agent does.  So we have to take care of it too.  (If the machine
 * uses grub, then we don't have to do anything.)
 */
static void update_lilo(struct package *pkg)
{
  const char *name = "/etc/lilo.conf", *savename = "/etc/lilo.conf.rpmsave";
  FILE *in, *out;
  char *buf = NULL, *oldktag, *oldkname;
  char *newktag, *newkname, *initrdcmd, *newitag;
  const char *p, *q;
  int bufsize = 0, status, replaced;
  struct stat statbuf;

  /* For now, only act on updates. */
  if (!pkg->instrev.present || !pkg->newlistrev.present
      || revsame(&pkg->instrev, &pkg->newlistrev))
    return;

  /* If there's no lilo.conf, then the machine presumably doesn't use lilo. */
  if (stat(name, &statbuf) == -1)
    return;

  /* Figure out kernel names. */
  easprintf(&oldktag, "%s-%s", pkg->instrev.version, pkg->instrev.release);
  easprintf(&oldkname, "/boot/vmlinuz-%s", oldktag);
  easprintf(&newktag, "%s-%s", pkg->newlistrev.version,
	    pkg->newlistrev.release);
  easprintf(&newkname, "/boot/vmlinuz-%s", newktag);

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
   * name in "image" lines.  Update "initrd" following images we
   * replace, if it exists, for the benefit of machines that need it.
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
        {
	  p = strstr(buf, oldktag);
	  if (p != NULL)
	    q = strstr(p, ".img");
	  if (p != NULL && q != NULL)
	    {
	      easprintf(&newitag, "%s%.*s", newktag, q - (p + strlen(oldktag)),
			p + strlen(oldktag));
	      easprintf(&initrdcmd, "/sbin/mkinitrd -f /boot/initrd-%s.img %s",
			newitag, newitag);
	      fprintf(out, "%.*s%s%s\n", p - buf, buf, newitag, q);
	      if (system(initrdcmd) != 0)
		fprintf(stderr, "Error running %s", initrdcmd);
	      free(newitag);
	      free(initrdcmd);
	      continue;
	    }
	}
      fprintf(out, "%s\n", buf);
    }
  fclose(in);
  if (status == -1 || ferror(out) || fclose(out) == EOF)
    {
      rename(savename, name);
      die("Error rewriting lilo.conf: %s", strerror(errno));
    }

  free(oldktag);
  free(oldkname);
  free(newktag);
  free(newkname);

  /* If a machine has both lilo.conf and grub.conf, then it's anyone's
   * guess which one is installed.  We will guess that the machine
   * uses grub, so we update lilo.conf but don't run lilo.
   */
  if (stat("/boot/grub/grub.conf", &statbuf) == -1)
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
  int i, j, k, len;
  struct utsname buf;
  char *newfile;
  struct stat statbuf;

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
   * new string with the highest version of the architecture available
   * and free the old one.  
   */
  for (k = j; k >= 0; k--)
    {
      easprintf(&newfile, "%.*s%s%s", p - filename, filename, arches[k],
		p + strlen(arches[i]));
      if (!stat(newfile,&statbuf))
	{
	  free(filename);
	  return newfile;
	}
    }
  /* No appropriate RPM exists. */
  fprintf(stderr, "Can't find appropriate RPM for architecture %s for %s\n",
	  arches[j], filename);
  exit(1);
}

static void prepare_copy_area(void)
{
  struct stat statbuf;

  /* Make sure the local copy area exists. */
  if (stat(RPMCACHE, &statbuf) == -1)
    {
      if (mkdir(RPMCACHE, 0755) == -1)
	die("Can't create local copy area %s: %s", RPMCACHE, strerror(errno));
    }
  else if (!S_ISDIR(statbuf.st_mode))
    die("Local copy area %s exists but is not a directory.", RPMCACHE);

  clear_copy_area();
}

static void check_copy_size(struct package **pkgtab)
{
  int i;
  struct package *pkg;
  struct statfs sf;
  struct stat st;
  off_t total = 0;

  for (i = 0; i < HASHSIZE; i++)
    {
      for (pkg = pkgtab[i]; pkg; pkg = pkg->next)
	{
	  if (pkg->action != UPDATE)
	    continue;
	  assert(pkg->filename != NULL);
	  if (stat(pkg->filename, &st) == -1)
	    die("Can't stat %s: %s", pkg->filename, strerror(errno));
	  total += st.st_blocks;
	}
    }

  if (statfs(RPMCACHE, &sf) == -1)
    die("Can't statfs %s: %s", RPMCACHE, strerror(errno));
  if (sf.f_bavail * (sf.f_bsize / 512) < total)
    {
      die("Need %dMB free space in %s to copy RPMs (probably more to update).",
	  total / 2048, RPMCACHE);
    }
}

static char *copy_local(const char *filename)
{
  const char *basename;
  char *lname, buf[8192];
  int rfd, wfd, rcount, wcount, wtotal;

  /* Determine the local filename. */
  basename = strrchr(filename, '/');
  basename = (basename == NULL) ? filename : basename + 1;
  easprintf(&lname, "%s/%s", RPMCACHE, basename);

  /* Open the input and output files. */
  rfd = open(filename, O_RDONLY);
  if (rfd == -1)
    die("Can't read %s: %s", filename, strerror(errno));
  wfd = open(lname, O_RDWR|O_CREAT|O_EXCL, 0644);
  if (wfd == -1)
    die("Can't write %s: %s", lname, strerror(errno));

  /* Do the copy. */
  while ((rcount = read(rfd, buf, sizeof(buf))) != 0)
    {
      if (rcount == -1)
	die("Can't read data from %s: %s", filename, strerror(errno));
      wtotal = 0;
      while (wtotal < rcount)
	{
	  wcount = write(wfd, buf + wtotal, rcount - wtotal);
	  if (wcount == -1)
	    die("Can't write to %s: %s", lname, strerror(errno));
	  wtotal += wcount;
	}
    }

  close(rfd);
  if (close(wfd) == -1)
    die("Can't close %s: %s", lname, strerror(errno));

  return lname;
}

/* Clear out the local copy area. */
static void clear_copy_area(void)
{
  DIR *dir;
  struct dirent *d;
  char *path;

  dir = opendir(RPMCACHE);
  if (!dir)
    return;
  while ((d = readdir(dir)) != NULL)
    {
      if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
	continue;
      easprintf(&path, "%s/%s", RPMCACHE, d->d_name);
      unlink(path);
      free(path);
    }
  closedir(dir);
}

static void read_rev_from_header(struct rev *rev, Header h)
{
  int32_t *epoch;
  char *version, *release;

  if (!headerGetEntry(h, RPMTAG_EPOCH, NULL, (void **) &epoch, NULL))
    epoch = NULL;
  headerGetEntry(h, RPMTAG_VERSION, NULL, (void **) &version, NULL);
  headerGetEntry(h, RPMTAG_RELEASE, NULL, (void **) &release, NULL);
  rev->present = 1;
  rev->epoch = (epoch == NULL) ? -1 : *epoch;
  rev->version = estrdup(version);
  rev->release = estrdup(release);
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
  pkg->multiple_installed_revs = 0;
  pkg->notouch = 0;
  pkg->only_upgrade = 0;
  pkg->action = NONE;
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
static const char *find_back(const char *start, const char *end, char c)
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

static char *estrndup(const char *s, size_t n)
{
  char *new_s;

  new_s = emalloc(n + 1);
  memcpy(new_s, s, n);
  new_s[n] = 0;
  return new_s;
}

static int easprintf(char **ptr, const char *fmt, ...)
{
  va_list ap;
  int ret;

  va_start(ap, fmt);
  ret = vasprintf(ptr, fmt, ap);
  va_end(ap);
  if (ret == -1)
    die("asprintf malloc failed for format string %s", fmt);
  return ret;
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
  fprintf(stderr, "Usage: %s [-cdhnpv] [-u upgradelist] oldlist newlist\n", progname);
  exit(1);
}
