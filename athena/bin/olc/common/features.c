/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains functions for the parsing and retreval of feature information
 *
 *      Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 *
 *      $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/common/features.c,v $
 *      $Id: features.c,v 1.1 1994-09-18 05:07:41 cfields Exp $
 *      $Author: cfields $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/common/features.c,v 1.1 1994-09-18 05:07:41 cfields Exp $";
#endif
#endif

#include <fcntl.h>
#include <sys/stat.h>


#if defined(__STDC__) && !defined(ibm032)
#include <stdlib.h>
#endif

#include <olc/olc.h>
#include <common.h>
#include "common_et.h"

/*
 * if there is more than one line for a keyword in the server data, only
 * the last will survive
 *
 * if there is more than one request for the same keyword data, only the
 * first will be filled
 */

ERRCODE
get_features_from_server(server, data, num)
     char *server;
     F_INFO *data;
     int num;
{
  return(-1);
}

ERRCODE
get_features_from_file(filename, data, num)
     char *filename;
     F_INFO *data;
     int *num;
{
  struct stat statb;
  int fd, retval;
  char *buf;

  if ((fd = open(filename,O_RDONLY,0)) < 0)
    return(OLCC_BAD_OPEN);

  if (fstat(fd,&statb) != 0) {
    close(fd);
    return(OLCC_BAD_STAT);
  }

  if ((buf = (char *)malloc(statb.st_size)) == NULL) {
    close(fd);
    return(OLCC_NOMEM);
  }

  if ((read(fd,buf,statb.st_size)) != statb.st_size) {
    free(buf);
    close(fd);
    return(OLCC_BAD_READ);
  }

  if (close(fd) != 0) {
    free(buf);
    return(OLCC_BAD_CLOSE);
  }

  retval = get_features_from_buf(buf,statb.st_size,data,num);
  free(buf);
  return(retval);
}

ERRCODE
get_features_from_buf(buf, buflen, data, num)
     char *buf;
     int buflen;
     F_INFO *data;
     int *num;
{
  char *kwd, *v_num, *kwd_data, *p;
  char *eob;
  int i, version;
  int n_changed;

  n_changed = 0;
  eob = (char *) (buf + buflen);
  while (buf < eob) {
    while(isspace(*buf))
      buf++;
    kwd = buf;
    if ((p = index(buf,' ')) == NULL)
      return(OLCC_BAD_FEATURE_FORMAT);
    *p = '\0'; /* Put null at end of keyword string */
    buf = p+1;
    while(isspace(*buf))
      buf++;
    v_num = buf;
    if ((p = index(buf,' ')) == NULL)
      return(OLCC_BAD_FEATURE_FORMAT);
    *p = '\0'; /* Put null at end of version number */
    while(isspace(*buf))
      buf++;
    buf = p+1;
    kwd_data = buf;
    if ((p = index(buf,'\n')) == NULL)
      return(OLCC_BAD_FEATURE_FORMAT);
    *p = '\0'; /* Put null at end of keyword data */
    buf = p + 1;

    version = atoi(v_num);

    for(i=0;i<*num;i++) {
      if ((strcasecmp(kwd,data[i].keyword) == 0) &&
	  (version == data[i].vers)) {
	if ((data[i].value = (char *)malloc(strlen(kwd_data)+1)) == NULL) {
	  return(-1);
	}
	strcpy(data[i].value, kwd_data);
	n_changed++;
	break;
      }
    }
  }
  *num = n_changed;
  return(0);
}

