/*
 * $Id: rvdlib.h,v 1.2 1999-01-22 23:08:36 ghudson Exp $
 */
#ifndef _RVDLIB_H_
#define _RVDLIB_H_

#include <netinet/rvdconst.h>
#define RVDETAB		0100	/* can't read rvdtab */
#define RVDEARGN	0101	/* too few args */
#define RVDENOENT	0102	/* pack not found */
#define RVDEUP		0103	/* already spunup */
#define RVDEDOWN	0104	/* already spundown */
#define RVDENOTAVAIL	0105	/* pack not available */

#include <machineio/vdreg.h>

#define RVD_DEV	VD_NAME_LEN	/* max block device name */
#define RVD_PW	VD_CAPAB_LEN	/* max capability chars */
#define RVD_NAM	VD_NAME_LEN	/* max pack name */

#define RVD_TIMEOUT	3	/* timeout on each attempt (secs) */
#define RVD_XMIT	5	/* number of retransmits */
#define RVD_RETRY	0	/* number of spinup retries */
#define RVD_INTERVAL	60	/* retry interval */

#define RVD_MODES	"RrXx"
#define RVD_RD_HARD	'R'
#define RVD_RD	'r'
#define RVD_EX_HARD	'X'
#define RVD_EX	'x'
#define RVD_SH_HARD	'S'
#define RVD_SH	's'

#define RVDGETM "/etc/athena/rvdgetm"
#define RVD_GETM "/etc/athena/rvdgetm","rvdgetm"
#define RVD_GETMOTD "operation=get_message\n"

#define vdnam(_d_,_n_)        (((void)sprintf((_d_),"/dev/vd%da",(_n_))),_d_)

#define rvdnam(_d_,_n_)       (((void)sprintf((_d_),"/dev/rvd%da",(_n_))),_d_)

#define VDCONTROL "/dev/rvdcontrol"
#endif
