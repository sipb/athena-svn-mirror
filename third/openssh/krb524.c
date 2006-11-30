/* krb.h has some conflicts with the openssl des.h header, so we need
   to contain its use to a separate file which doesn't need the
   openssl headers.  This code is used from do_exec in session.c. */

/* Convert the user's krb5 tickets to krb4 tickets. */
#include "includes.h"
RCSID("$Id: krb524.c,v 1.2 2006-11-30 22:20:11 ghudson Exp $");
#include "xmalloc.h"
#include "auth.h"
#ifdef KRB5
#include <krb5.h>
#include <krb.h>
int
do_krb524_conversion(Authctxt *authctxt)
{
  static char tktname[512];
  int problem;
  krb5_creds increds, *v5creds;
  krb5_data *realm;
  CREDENTIALS v4creds;

  realm = krb5_princ_realm(authctxt->krb5_ctx, authctxt->krb5_user);
  
  memset(&increds, 0, sizeof(increds));
  if ((problem = krb5_build_principal_ext(authctxt->krb5_ctx,
					  &(increds.server), realm->length, 
					  realm->data, 6, "krbtgt", 
					  realm->length, realm->data, NULL)))
    return problem;
    
  increds.client = authctxt->krb5_user;
  increds.times.endtime = 0;
  increds.keyblock.enctype = ENCTYPE_DES_CBC_CRC;
  if ((problem = krb5_get_credentials(authctxt->krb5_ctx, 0,
				      authctxt->krb5_fwd_ccache, &increds,
				      &v5creds)))
    return problem;
  
  if ((problem = krb5_524_convert_creds(authctxt->krb5_ctx, v5creds, &v4creds)))
    return problem;
  
  sprintf(tktname, "KRBTKFILE=/tmp/tkt_p%d", getpid());
  putenv(xstrdup(tktname));
  if (problem = in_tkt(v4creds.pname, v4creds.pinst))
    return problem;
  
  if ((problem = krb_save_credentials(v4creds.service, v4creds.instance,
				      v4creds.realm, v4creds.session,
				      v4creds.lifetime, v4creds.kvno,
				      &(v4creds.ticket_st), 
				      v4creds.issue_date)))
    return problem;
  
  chown(tkt_string(), authctxt->pw->pw_uid, authctxt->pw->pw_gid);
  return 0;
}

void
krb_cleanup(void)
{
  dest_tkt();
}
#endif
