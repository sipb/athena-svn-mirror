/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/lwp/RCS/timer.h,v 2.1 1990/08/07 19:17:41 hines Exp $ */
/* $Source: /afs/transarc.com/project/fs/dev/afs/rcs/lwp/RCS/timer.h,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidtimer = "$Header: /afs/transarc.com/project/fs/dev/afs/rcs/lwp/RCS/timer.h,v 2.1 1990/08/07 19:17:41 hines Exp $";
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

/*******************************************************************\
* 								    *
* 	Information Technology Center				    *
* 	Carnegie-Mellon University				    *
* 								    *
* 								    *
\*******************************************************************/

struct TM_Elem {
    struct TM_Elem	*Next;		/* filled by package */
    struct TM_Elem	*Prev;		/* filled by package */
    struct timeval	TotalTime;	/* filled in by caller -- modified by package */
    struct timeval	TimeLeft;	/* filled by package */
    char		*BackPointer;	/* filled by caller, not interpreted by package */
};

#ifndef _TIMER_IMPL_
extern void Tm_Insert();
#define TM_Remove(list, elem) remque(elem)
extern int TM_Rescan();
extern struct TM_Elem *TM_GetExpired();
extern struct TM_Elem *TM_GetEarliest();
#endif

#define FOR_ALL_ELTS(var, list, body)\
	{\
	    register struct TM_Elem *_LIST_, *var, *_NEXT_;\
	    _LIST_ = (list);\
	    for (var = _LIST_ -> Next; var != _LIST_; var = _NEXT_) {\
		_NEXT_ = var -> Next;\
		body\
	    }\
	}
