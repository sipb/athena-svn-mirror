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
/* itc.h - ITC standard header file
 *
 *	Author: John H. Howard
 *
 *	defines a few constants used throughout ITC code
 *
 *	$Header: /afs/transarc.com/project/fs/dev/afs/rcs/util/RCS/itc.h,v 2.2 1992/11/06 19:09:44 bob Exp $
 */

/* $Log:	itc.h,v $
 * Revision 1.2  89/03/30  21:48:14  kazar
 * update copyright notice
 * 
 * Revision 1.1  87/04/13  16:00:36  mikew
 * Initial revision
 * 
 * Revision 3.1  86/02/05  18:30:23  daemon
 * new root
 * 
 * Revision 2.1  86/02/05  18:30:19  daemon
 * release root
 * 
 * Revision 1.1  85/09/24  14:36:48  peterson
 * Initial revision
 * 
 * Revision 1.1  84/12/07  17:44:16  peterson
 * . initial install
 * 
 * Revision 1.3  84/05/24  22:46:19  wjh
 * add initial comments and move log 
 * 
 * Revision 1.1  84/04/18  15:21:18  jhh
 * Initial release of Makefile and itc.h 
 *  */


#ifndef	_ITC
#define	_ITC

/* C language patches */
typedef	int	boolean;
#define	private	static

/* parameter usage description tags */
#define	in
#define	out
#define	inout

/* very commonly used values */
#define	TRUE	1
#define	FALSE	0
#define	NULL	0

#endif /* _ITC */
