/*
 * Copyright 1989 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * M.I.T. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability
 * of this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * MotifUtils:   Utilities for use with Motif and UIL
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Mu/MuHelpFile.c,v $
 * $Author: cfields $
 * $Log: not supported by cvs2svn $
 * Revision 1.1  89/12/09  15:14:31  djf
 * Initial revision
 * 
 */

#include "Mu.h"
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char *malloc();

/*
 *  This function takes a filename as its argument, stats the file to find
 *  it's length, malloc's enough space to hold the text, reads it in,
 *  then calls MuHelp with that string.
 */


void
MuHelpFile(filename)
     char *filename;
{
  struct stat statbuf;
  char *helpbuf;
  int fd;

  if (stat(filename, &statbuf))
    {
      MuError("MuHelpFile: Unable to stat help file.");
      return;
    }

  if ((helpbuf = malloc((1 + (int)statbuf.st_size) * sizeof(char)))
      == (char *) NULL)
    {
      MuError("MuHelpFile: Out of memory.\nUnable to malloc enough space for help file.");
      return;
    }

  if ((fd = open(filename, O_RDONLY, 0)) < 0)
    {
      MuError("MuHelpFile: Unable to open help file for reading.\nPerhaps the permissions on the file are wrong.");
      close(fd);
      free(helpbuf);
      return;
    }

  if ((read(fd, helpbuf, (int)statbuf.st_size)) != statbuf.st_size)
    {
      MuError("MuHelpFile: An error occurred while reading the file.\nThe number of bytes read does not match the length of the file.");
      close(fd);
      free(helpbuf);
      return;
    }

  helpbuf[statbuf.st_size] = '\0';
  MuHelp(helpbuf);
  close(fd);
  free(helpbuf);
  return;
}
