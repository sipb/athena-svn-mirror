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
 *      $Id: features.c,v 1.6 1999-06-28 22:52:24 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: features.c,v 1.6 1999-06-28 22:52:24 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

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

  fd = open(filename,O_RDONLY,0);
  if (fd < 0)
    return(OLCC_BAD_OPEN);

  if (fstat(fd,&statb) != 0) {
    close(fd);
    return(OLCC_BAD_STAT);
  }

  buf = malloc(statb.st_size);
  if ((buf == NULL) {
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
    p = strchr(buf,' ');
    if (p == NULL)
      return(OLCC_BAD_FEATURE_FORMAT);
    *p = '\0'; /* Put null at end of keyword string */
    buf = p+1;
    while(isspace(*buf))
      buf++;
    v_num = buf;
    p = strchr(buf,' ');
    if (p == NULL)
      return(OLCC_BAD_FEATURE_FORMAT);
    *p = '\0'; /* Put null at end of version number */
    while(isspace(*buf))
      buf++;
    buf = p+1;
    kwd_data = buf;
    p = strchr(buf,'\n');
    if (p == NULL)
      return(OLCC_BAD_FEATURE_FORMAT);
    *p = '\0'; /* Put null at end of keyword data */
    buf = p + 1;

    version = atoi(v_num);

    for(i=0;i<*num;i++) {
      if ((strcasecmp(kwd,data[i].keyword) == 0) &&
	  (version == data[i].vers)) {
	data[i].value = malloc(strlen(kwd_data)+1);
	if (data[i].value == NULL) {
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

