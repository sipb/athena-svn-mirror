#include <stdio.h>
#include <string.h>
#include <errno.h>

panic(s)
     char *s;
{ fprintf(stderr,"Fatal error: %s\n",s);
  exit(1);
}
epanic(s)
     char *s;
{ fprintf(stderr, "Fatal error: %s: %s\n", s, strerror(errno));
  exit(1);
}
