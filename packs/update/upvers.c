/* Copyright 1996 by the Massachusetts Institute of Technology.
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

/* This is the source for the upvers program, which runs the
 * appropriate scripts during the update process.
 */

static char rcsid[] = "$Id: upvers.c,v 1.13 1996-12-27 22:10:55 ghudson Exp $";

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

struct verfile {
  int major;
  int minor;
  int patch;
};

static int version_compare(const void *arg1, const void *arg2);

int main(int argc, char **argv)
{
  struct verfile vf[1024], oldv, newv;
  DIR *dp;
  struct dirent *dirp;
  int n = 0, i, start, end;
  char filename[1024];

  if (argc < 4)
    {
      fprintf(stderr, "Usage: upvers <old-vers> <new-vers> <libdir>\n");
      return 1;
    }

  /* Parse old-vers and new-vers. */
  if (sscanf(argv[1], "%d.%d.%d", &oldv.major, &oldv.minor, &oldv.patch) != 3
      || sscanf(argv[2], "%d.%d.%d", &newv.major, &newv.minor,
		&newv.patch) != 3)
    {
      fprintf(stderr, "First two arguments must be dotted triplets.\n");
      return 1;
    }

  /* Get the names of all the version files in libdir. */
  dp = opendir(argv[3]);
  if (!dp)
    {
      perror("opendir");
      return 1;
    }
  while (dirp = readdir(dp))
    {
      if (sscanf(dirp->d_name, "%d.%d.%d", &vf[n].major, &vf[n].minor,
		 &vf[n].patch) == 3)
	n++;
    }
  closedir(dp);

  /* Sort the version files names and decide where to start and end at. */
  qsort(vf, n, sizeof(struct verfile), version_compare);
  start = n + 1;
  end = n;
  for (i = 0; i < n; i++)
    {
      if (version_compare(&vf[i], &oldv) > 0 && start == n + 1)
	start = i;
      if (version_compare(&vf[i], &newv) > 0 && end == n)
	end = i;
    }

  /* Now run the appropriate files. */
  for (i = start; i < end; i++)
    {
      sprintf(filename, "%s/%d.%d.%d", argv[3], vf[i].major, vf[i].minor,
	      vf[i].patch);
      printf("Running %s\n", filename);
      system(filename); 
    }

  return 0;
}
		
static int version_compare(const void *arg1, const void *arg2)
{
  const struct verfile *v1 = (const struct verfile *) arg1;
  const struct verfile *v2 = (const struct verfile *) arg2;

  /* Compare major version, then minor version, then patchlevel. */
  return (v1->major != v2->major) ? v1->major - v2->major
    : (v1->minor != v2->minor) ? v1->minor - v2->minor
    : v1->patch - v2->patch;
}
