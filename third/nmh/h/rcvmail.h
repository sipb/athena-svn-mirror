
/*
 * rcvmail.h -- rcvmail hook definitions
 *
 * $Id: rcvmail.h,v 1.1.1.1 1999-02-07 18:14:06 danw Exp $
 */

#if defined(SENDMTS) || defined(SMTPMTS)
# include <ctype.h>
# include <errno.h>
# include <setjmp.h>
# include <stdio.h>
# include <sys/types.h>
# include <mts/smtp/smtp.h>
#endif /* SENDMTS || SMTPMTS */

#ifdef MMDFMTS
# include <mts/mmdf/util.h>
# include <mts/mmdf/mmdf.h>
#endif /* MMDFMTS */


#if defined(SENDMTS) || defined(SMTPMTS)
# define RCV_MOK	0
# define RCV_MBX	1
#endif /* SENDMTS || SMTPMTS */

#ifdef MMDFI
# define RCV_MOK	RP_MOK
# define RCV_MBX	RP_MECH
#endif /* MMDFI */


#ifdef NRTC			/* sigh */
# undef RCV_MOK
# undef RCV_MBX
# define RCV_MOK	RP_MOK
# define RCV_MBX	RP_MECH
#endif /* NRTC */