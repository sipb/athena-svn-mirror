/* $Id: auth2-pam.h,v 1.1.1.1 2001-11-15 19:24:15 ghudson Exp $ */

#include "includes.h"
#ifdef USE_PAM

int	auth2_pam(Authctxt *authctxt);

#endif /* USE_PAM */
