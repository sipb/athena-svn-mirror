/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/util/RCS/vice.h,v 2.16 1995/09/07 15:02:28 zumach Exp $ */
/* $ACIS:vice.h 9.0$ */
/* $Source: /afs/transarc.com/project/fs/dev/afs/rcs/util/RCS/vice.h,v $ */

#if !defined(lint) && !defined(LOCORE)  && defined(RCS_HDRS)
static char *rcsidvice = "$Header: /afs/transarc.com/project/fs/dev/afs/rcs/util/RCS/vice.h,v 2.16 1995/09/07 15:02:28 zumach Exp $";
#endif

/*
****************************************************************************
*        Copyright IBM Corporation 1988, 1989 - All Rights Reserved        *
*                                                                          *
* Permission to use, copy, modify, and distribute this software and its    *
* documentation for any purpose and without fee is hereby granted,         *
* provided that the above copyright notice appear in all copies and        *
* that both that copyright notice and this permission notice appear in     *
* supporting documentation, and that the name of IBM not be used in        *
* advertising or publicity pertaining to distribution of the software      *
* without specific, written prior permission.                              *
*                                                                          *
* IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL IBM *
* BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY      *
* DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER  *
* IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING   *
* OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.    *
****************************************************************************
*/

/*
 * /usr/include/sys/vice.h
 *
 * Definitions required by user processes needing extended vice facilities
 * of the kernel.
 * 
 * NOTE:  /usr/include/sys/remote.h contains definitions common between
 *	    	Venus and the kernel.
 * 	    /usr/local/include/viceioctl.h contains ioctl definitions common
 *	    	between user processes and Venus.
 */
#include <afs/param.h>

#if defined(AFS_SGI61_ENV) && defined(KERNEL)
struct KViceIoctl {
	app32_ptr_t in, out;	/* Data to be transferred in, or out */
	short in_size;		/* Size of input buffer <= 2K */
	short out_size;		/* Maximum size of output buffer, <= 2K */
};
#endif
struct ViceIoctl {
	caddr_t in, out;	/* Data to be transferred in, or out */
	short in_size;		/* Size of input buffer <= 2K */
	short out_size;		/* Maximum size of output buffer, <= 2K */
};

/* The 2K limits above are a consequence of the size of the kernel buffer
   used to buffer requests from the user to venus--2*MAXPATHLEN.
   The buffer pointers may be null, or the counts may be 0 if there
   are no input or output parameters
 */
#if defined(AFS_SGI_ENV) || defined(AFS_HPUX_ENV) || defined(AFS_AIX32_ENV) || defined(AFS_NEXT_ENV) || defined(AFS_DEC_ENV) || defined(AFS_SUN5_ENV) || defined(AFS_OSF_ENV)
#if defined(AFS_SGI61_ENV) && defined(KERNEL)
#define _VICEIOCTL(id)  ((unsigned int ) _IOW('V', id, struct KViceIoctl))
#else
#define _VICEIOCTL(id)  ((unsigned int ) _IOW('V', id, struct ViceIoctl))
#endif
#else
#define _VICEIOCTL(id)  ((unsigned int ) _IOW(V, id, struct ViceIoctl))
#endif

/* Use this macro to define up to 256 vice ioctl's.  These ioctl's
   all potentially have in/out parameters--this depends upon the
   values in the ViceIoctl structure.  This structure is itself passed
   into the kernel by the normal ioctl parameter passing mechanism.
 */

#define _VALIDVICEIOCTL(com) (com >= _VICEIOCTL(0) && com <= _VICEIOCTL(255))
