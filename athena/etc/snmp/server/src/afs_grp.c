/*
 *
 */

#include "include.h"

#ifdef ATHENA
#ifdef AFS
#include <krb.h>

char *cache_file = "/usr/vice/etc/cacheinfo";

lu_afs(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  char buf[BUFSIZ];

  bzero(buf, BUFSIZ);

  if (varnode->flags & NOT_AVAIL ||
      varnode->offset <= 0 ||     /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;                    /* include the "0" instance */
  
  switch(varnode->offset)
    {
    case N_AFSCCACHESIZE:
      repl->val.type = INT;
      repl->val.value.intgr = crock_cachesize();
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_kerberos: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}


crock_cachesize()
{
  FILE *fp;
  char buf[BUFSIZ];
  char *cp;

  fp = fopen(cache_file, "r");
  if(fp == (FILE *) NULL)
    {
      syslog(LOG_ERR, "lu_afs: cannot open %s", cache_file);
      return(BUILD_ERR);
    }

  if(fgets(buf, BUFSIZ, fp) == (char *) NULL)
    {
      fclose(fp);
      return(0);
    }

  fclose(fp);

  cp = rindex(buf, ':');
  if(cp++ == (char) NULL)
    return(0);
    
  return(atoi(cp));
}

#endif AFS
#endif ATHENA
