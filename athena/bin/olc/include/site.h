/* XXX You must edit this file in order to correctly configure OLC! XXX
 *
 * This file is part of the OLC On-Line Consulting System.
 * It provides macros defining values for site-local messages
 * shown to users (eg. the consulting phone number).
 *
 *      bert Dvornik
 *      MIT Let Me Keep My Tether Account So I Could Work On This
 *
 * Copyright (C) 1999 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: site.h,v 1.1 1999-03-06 16:48:29 ghudson Exp $
 */

#include <mit-copyright.h>

#ifndef OLC__SITE_H
#define OLC__SITE_H

/* Domain to be appended to usernames to get valid mail addresses */
#ifndef DEFAULT_MAIL_DOMAIN
#define DEFAULT_MAIL_DOMAIN     "mit.edu"
#endif

/* Kerberos realm for the OLC server */
#ifndef DFLT_SERVER_REALM
#define DFLT_SERVER_REALM       "ATHENA.MIT.EDU"
#endif

/* Phone number that can be used to reach the consulting office */
#ifndef CONSULT_PHONE_NUMBER
#define CONSULT_PHONE_NUMBER    "253-4435"
#endif

/* People to talk to if a machine has hardware problems */
#ifndef HARDWARE_MAINTAINER
#define HARDWARE_MAINTAINER     "Athena Hardware Hotline"
#endif

#endif /* OLC__SITE_H */
