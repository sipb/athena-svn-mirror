#ifndef _QUOTA_LIMIT_READ_
#define _QUOTA_LIMIT_READ_

#define SERV_SZ	20	/* Size of service name */
#define PNAME_SZ 30     /* Size of printer name */
#define CURRENCY_SZ 10  /* Size of currency */
#define MESSAGE_SZ 80   /* Size of message returned from quota query */

/* krb.h */
#ifndef MAX_K_NAME_SZ
#define MAX_K_NAME_SZ 122 /* From krb.h */
#endif

#define LOGMAXRETURN	100	/* Return a maximum of 100 entries/rpc call */
				/* This insures other requests get in...
*/

#define QUOTAQUERYPORT 3702
#define QUOTAQUERYENT "lpquery"

#define QUOTALOGGERPORT 3703
#define QUOTALOGENT "lplog"
#endif
