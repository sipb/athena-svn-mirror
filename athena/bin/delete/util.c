/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/util.c,v $
 * $Author: jik $
 *
 * This program is a replacement for rm.  Instead of actually deleting
 * files, it marks them for deletion by prefixing them with a ".#"
 * prefix.
 *
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if (!defined(lint) && !defined(SABER))
     static char rcsid_util_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/util.c,v 1.2 1989-01-26 12:02:23 jik Exp $";
#endif

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <strings.h>
#include <pwd.h>
#include "util.h"

char *getenv();


char *convert_to_user_name(real_name, user_name)
char real_name[];
char user_name[];  /* RETURN */
{
     char *ptr, *q;
     
     strcpy(user_name, real_name);
     while (ptr = strrindex(user_name, ".#")) {
	  for (q = ptr; *(q + 2); q++)
	       *q = *(q + 2);
	  *q = '\0';
     }
     return (user_name);
}

     



char *strindex(str, sub_str)
char *str, *sub_str;
{
     char *ptr = str;
     while (ptr = index(ptr, *sub_str)) {
	  if (! strncmp(ptr, sub_str, strlen(sub_str)))
	       return(ptr);
	  ptr++;
     }
     return ((char *) NULL);
}



char *strrindex(str, sub_str)
char *str, *sub_str;
{
     char *ptr;

     if (strlen(str))
	  ptr = &str[strlen(str) - 1];
     else
	  return((char *) NULL);
     while ((*ptr != *sub_str) && (ptr != str)) ptr--;
     while (ptr != str) {
	  if (! strncmp(ptr, sub_str, strlen(sub_str)))
	       return(ptr);
	  ptr--;
	  while ((*ptr != *sub_str) && (ptr != str)) ptr--;
     }
     if (! strncmp(ptr, sub_str, strlen(sub_str)))
	  return(str);
     else
	  return ((char *) NULL);
}
     
     
is_dotfile(filename)
char *filename;
{
     return (! (strcmp(filename, ".") && strcmp(filename, "..")));
}



int is_deleted(filename)
char *filename;
{
     return(! strncmp(filename, ".#", 2));
}




/*
 * NOTE: Append uses a static array, so its return value must be
 * copied immediately.
 */
char *append(filepath, filename)
char *filepath, *filename;
{
     static char buf[MAXPATHLEN];

     strcpy(buf, filepath);
     if ((! *filename) || (! *filepath)) {
	  strcpy(buf, filename);
	  return(buf);
     }
     if (buf[strlen(buf) - 1] == '/')
	  buf[strlen(buf) - 1] = '\0';
     if (strlen(buf) + strlen(filename) + 2 > MAXPATHLEN) {
 	  *buf = '\0';
	  return(buf);
     }
     strcat(buf, "/");
     strcat(buf, filename);
     return(buf);
}




yes() {
     char buf[BUFSIZ];
     char *val;
     
     val = fgets(buf, BUFSIZ, stdin);
     if (! val)
	  exit(1);
     if (! index(buf, '\n')) do
	  fgets(buf + 1, BUFSIZ - 1, stdin);
     while (! index(buf + 1, '\n'));
     return(*buf == 'y');
}




char *lastpart(filename)
char *filename;
{
     char *part;

     part = rindex(filename, '/');

     if (! part)
	  part = filename;
     else if (part == filename)
	  part++;
     else if (part - filename + 1 == strlen(filename)) {
	  part = rindex(--part, '/');
	  if (! part)
	       part = filename;
	  else
	       part++;
     }
     else
	  part++;

     return(part);
}




char *firstpart(filename, rest)
char *filename;
char *rest; /* RETURN */
{
     char *part;
     static char buf[MAXPATHLEN];

     strcpy(buf, filename);
     part = index(buf, '/');
     if (! part) {
	  *rest = '\0';
	  return(buf);
     }
     strcpy(rest, part + 1);
     *part = '\0';
     return(buf);
}





get_home(buf)
char *buf;
{
     char *user;
     
     strcpy(buf, getenv("HOME"));
     struct passwd *psw;
     
     if (*buf)
	  return(0);

     user = getenv("USER");
     psw = getpwnam(user);

     if (psw) {
	  strcpy(buf, psw->pw_dir);
	  return(0);
     }
     
     psw = getpwuid(getuid());

     if (psw) {
	  strcpy(buf, psw->pw_dir);
	  return(0);
     }  
     return(1);
}
