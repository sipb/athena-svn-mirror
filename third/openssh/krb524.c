/* Convert the user's krb5 tickets to krb4 tickets. */
#include "includes.h"
RCSID("$Id: krb524.c,v 1.1 2002-02-16 22:47:12 zacheiss Exp $");
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

  krb524_init_ets(authctxt->krb5_ctx);
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
  
  if ((problem = krb524_convert_creds_kdc(authctxt->krb5_ctx, v5creds,
					  &v4creds)))
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
#endif
