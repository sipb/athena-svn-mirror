/* This is for AUX.  It is a wrapper around the C library regex functions. */
/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/emul_re.c,v 1.1 1990-07-09 01:06:43 jfc Exp $ */

#ifdef _AUX_SOURCE

static char *re;

char *re_comp(s)

char *s;

{
  if(!s)
    return 0;
  if(re)
    free(re);

  if(!(re = regcmp(s, (char *)0)))
    return "Bad argument to re_comp";

  return 0;
}

int re_exec(s)

char *s;

{
  return regex(re, s) != 0;
}

#endif
