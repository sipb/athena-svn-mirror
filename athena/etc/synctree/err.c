#include <stdio.h>

extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];

panic(s)
     char *s;
{ fprintf(stderr,"Fatal error: %s\n",s);
  exit(1);
}
epanic(s)
     char *s;
{ fprintf(stderr, "Fatal error: %s: %s\n", s,
	  errno<sys_nerr? sys_errlist[errno] : "unknown error");
  exit(1);
}
