/*
 *
 */

#include "include.h"

#ifdef ATHENA
#ifdef KERBEROS
#include <krb.h>

lu_kerberos(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  char buf[BUFSIZ];
  char *ptr;

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

  repl->val.type = STR;  /* True of all the replies */
  
  switch(varnode->offset)
    {
    case N_KRBCREALM:
      krb_get_realm(buf, 1);
      repl->val.value.str.len = strlen(buf);
      repl->val.value.str.str = (char *) malloc(sizeof(char) * 
				repl->val.value.str.len);
      return(BUILD_SUCCESS);
    default:
      syslog (LOG_ERR, "lu_kerberos: bad offset: %d", varnode->offset);
      return(BUILD_ERR);
    }
}

#endif KERBEROS
#endif ATHENA


krb_key(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  
