/*
 *
 */

#include "include.h"

#ifdef ATHENA
#include <sys/dir.h>

char *mqdir = "/usr/spool/mqueue";

lu_mail(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  if (varnode->flags & NOT_AVAIL ||
      varnode->offset < 0 ||     /* not expecting offset here */
      ((varnode->flags & NULL_OBJINST) && (reqflg == NXT)))
    return (BUILD_ERR);

  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  repl->name.ncmp++;                    /* include the "0" instance */
  repl->val.type = INT;
  repl->val.value.intgr = crock_mailq();
  return(BUILD_SUCCESS);
}


crock_mailq()
{
  DIR *d;
  struct direct *dp;
  int count = 0;

  d = opendir(mqdir);
  if(d == (DIR *) NULL)
    {
      syslog(LOG_ERR, "lu_mail: cannot open %d", mqdir);
      return(BUILD_ERR);
    }

  while(dp = readdir(d))
    if(strncmp(dp->d_name, "qfa", 3) == 0)
      ++count;

  closedir(d);
  return(count);
}

#endif ATHENA
