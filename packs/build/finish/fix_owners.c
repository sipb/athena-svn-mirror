#include <unistd.h>		/* readlink, chown */
#include <limits.h>		/* PATH_MAX */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>		/* strcmp, memset */
#include <dirent.h>		/* opendir, etc. */
#include <sys/types.h>		/* lstat, chown */
#include <sys/stat.h>		/* lstat */
#include <fcntl.h>		/* open */
#include <errno.h>

char *progname;

typedef struct _filename {
  struct _filename *next;
  char *name;
} filename;

typedef struct {
  filename *first, *current, *last;
} filelist;

void makefilelist(list)
     filelist *list;
{
  list->first = NULL;
  list->current = NULL;
  list->last = NULL;

  return;
}

void freefilelist(list)
     filelist *list;
{
  filename *i, *j = NULL;

  i = list->first;
  while (i != NULL)
    {
      j = i;
      i = i->next;
      free(j->name);
      free(j);
    }
}

void savefile(list, name)
     filelist *list;
     char *name;
{
  filename *new;

  new = malloc(sizeof(filename));
  if (new == NULL)
    {
      fprintf(stderr, "%s: error in savefile during malloc()\n", progname);
      exit(1);
    }

  new->name = malloc(strlen(name) + 1);
  if (new->name == NULL)
    {
      fprintf(stderr, "%s: error in savefile during malloc()\n", progname);
      exit(1);
    }

  strcpy(new->name, name);

  new->next = NULL;
  if (list->first == NULL)
    list->first = new;
  if (list->current == NULL)
    list->current = new;
  if (list->last != NULL)
    list->last->next = new;
  list->last = new;
}

char *getfile(list)
     filelist *list;
{
  char *ptr;

  if (list->current)
    {
      ptr = list->current->name;
      list->current = list->current->next;
      return ptr;
    }
  else
    return NULL;
}

int baduid(uid)
     uid_t uid;
{
  if (uid == 1047 || uid == 3433 || uid == 3622 ||
      uid == 16453 || uid == 19558 || uid > 32767)
    return 1;
  return 0;
}

int badgid(gid)
     gid_t gid;
{
  if (gid == 101)
    return 1;
  return 0;
}

void process(absolute, relative, fangs)
     char *absolute, *relative;
     int fangs;
{
  DIR *dir;
  char abs[PATH_MAX], rel[PATH_MAX];
  char create[PATH_MAX+2]; /* Extra "./"... Is that cool? */
  char *directoryname;
  struct dirent *curent;
  struct stat info;
  filelist dirlist;
  int bu, bg, newuid, newgid;

  if (chdir(absolute))
    {
      fprintf(stderr, "%s: error %d from chdir()\n", progname, errno);
      exit(1);
    }

  makefilelist(&dirlist);

  dir = opendir(".");
  if (dir == NULL)
    {
      fprintf(stderr, "%s: could not open directory %s\n",
	      progname, relative);
      exit(1);
    }

  while (curent = readdir(dir))
    {
      if (strcmp(curent->d_name, ".") && strcmp(curent->d_name, ".."))
	{
	  if (lstat(curent->d_name, &info))
	    {
	      fprintf(stderr, "%s: error %d from lstat(%s)\n", progname,
		      errno, curent->d_name);
	      exit(1);
	    }

	  if ((info.st_mode & S_IFMT) == S_IFDIR)
	    savefile(&dirlist, curent->d_name);

	  bu = baduid(info.st_uid);
	  bg = badgid(info.st_gid);

	  if (bu && (info.st_mode & S_ISUID))
	    {
	      fprintf(stderr, "%s: warning: preserving owner %d on %s/%s because it is setuid\n",
		      progname, info.st_uid, absolute, curent->d_name);
	      bu = 0;
	    }

	  if (bg && (info.st_mode & S_ISGID))
	    {
	      fprintf(stderr, "%s: warning: preserving group %d on %s/%s because it is setgid\n",
		      progname, info.st_gid, absolute, curent->d_name);
	      bg = 0;
	    }

	  if (bu)
	    newuid = 0;
	  else
	    newuid = info.st_uid;

	  if (bg)
	    newgid = 0;
	  else
	    newgid = info.st_gid;

	  if (bu || bg)
	    {
	      if (fangs)
#ifdef SYSV
		lchown(curent->d_name, newuid, newgid);
#else
		chown(curent->d_name, newuid, newgid);
#endif
	      else
		fprintf(stdout, "%s/%s: (%d.%d) -> (%d.%d)\n",
			absolute, curent->d_name, info.st_uid,
			info.st_gid, newuid, newgid);
	    }
	}
    }

  closedir(dir);

  while (directoryname = getfile(&dirlist))
    {
      sprintf(abs, "%s/%s", absolute, directoryname);
      sprintf(rel, "%s/%s", relative, directoryname);
      process(abs, rel, fangs);
    }

  freefilelist(&dirlist);
}

void usage()
{
  fprintf(stderr,
	  "usage: %s [-n] dir\n",
	  progname);
  exit(1);
}

int main(argc, argv)
     int argc;
     char **argv;
{
  char *sourcedir;
  int fangs = 1;

  progname = *argv++;

  if (argc != 2 && argc != 3)
    usage();

  if (argc == 3)
    {
      if (strcmp(*argv, "-n"))
        usage();

      fangs = 0;
      argv++;
    }

  sourcedir = *argv++;

  process(sourcedir, ".", fangs);
  exit(0);
}
