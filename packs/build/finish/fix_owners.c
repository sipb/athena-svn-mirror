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

#if !defined(SOLARIS) && !defined(sgi)
#define lchown chown
#endif

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

void newownership(file, relative, name, newuid, newgid)
     struct stat *file;
     char *relative, *name;
     uid_t *newuid;
     gid_t *newgid;
{
  struct stat osfile;
  char ospath[PATH_MAX];

  *newuid = file->st_uid;
  *newgid = file->st_gid;

  if (file->st_uid == 1047 || file->st_uid == 3433 || file->st_uid == 3622 ||
      file->st_uid == 16453 || file->st_uid == 19558 || file->st_uid > 32767)
    *newuid = 0;

  if (file->st_gid == 101)
    *newgid = 101;

  sprintf(ospath, "/os/%s/%s", relative, name);
  if (S_ISDIR(file->st_mode) && lstat(ospath, &osfile) == 0
      && S_ISDIR(osfile.st_mode))
    {
      *newuid = osfile.st_uid;
      *newgid = osfile.st_gid;
    }
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
  uid_t newuid;
  gid_t newgid;

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

	  if (S_ISDIR(info.st_mode))
	    savefile(&dirlist, curent->d_name);

	  newownership(&info, relative, curent->d_name, &newuid, &newgid);

	  if (info.st_uid != newuid && (info.st_mode & S_ISUID))
	    {
	      fprintf(stderr, "%s: warning: preserving owner %d on %s/%s because it is setuid\n",
		      progname, info.st_uid, absolute, curent->d_name);
	      newuid = info.st_uid;
	    }

	  if (info.st_gid != newgid && (info.st_mode & S_ISGID))
	    {
	      fprintf(stderr, "%s: warning: preserving group %d on %s/%s because it is setgid\n",
		      progname, info.st_gid, absolute, curent->d_name);
	      newgid = info.st_gid;
	    }

	  if (info.st_uid != newuid || info.st_gid != newgid)
	    {
	      if (fangs)
		lchown(curent->d_name, newuid, newgid);
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
