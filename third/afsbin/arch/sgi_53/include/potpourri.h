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
/*	Standard Preamble for .h files

Author:  	M.Satyanarayanan
		Information Technology Center
		Carnegie-Mellon University


$Log:	potpourri.h,v $
 * Revision 1.2  89/03/30  21:48:33  kazar
 * update copyright notice
 * 
 * Revision 1.1  87/04/13  16:00:27  mikew
 * Initial revision
 * 
 * Revision 3.1  86/02/05  20:53:08  daemon
 * new root
 * 
 * Revision 2.1  86/02/05  20:53:03  daemon
 * release root
 * 
 * Revision 1.8  85/10/25  10:25:20  satya
 * Fixed trivial bug in #define say()
 * 
 * Revision 1.7  85/10/05  16:46:15  satya
 * Added ifdef's to nullify say() macro and shorted assert().  Compiling
 * with -DNOASSERT and -DNOSAY will now save a lot of literal constant storage
 * if you have a program which uses these macros extensively.  Also added a null
 * else clause to say().
 * 
 * Revision 1.6  85/06/25  15:34:38  satya
 * . Made assert dump core. Defined generic say() macro.
 * 
 * Revision 1.5  85/05/12  21:43:17  satya
 * . Added say9() through say12(). Added file and line identification to sayX() printout.
 * 
 * Revision 1.4  85/04/23  21:07:06  satya
 * . Bug in SystemError() definition.
 * 
 * Revision 1.2  84/12/14  18:01:01  satya
 * . Added SystemError(x) to potpourri.h
 * 
 * Revision 1.1  84/12/07  17:44:14  peterson
 * . initial install
 * 
 * Revision 1.1  84/11/06  12:17:07  peterson
 * . add potpourri.h
 * 

Abstract:	Contains miscellaneous general-purpose  macros.
*/

#define MAXSELECT	20	/* Max no. of file desc. to be checked in select() calls */

/*------------------------------------------------------------*/
#define	IN	/* Input parameter */
#define OUT	/* Output parameter */
#define INOUT	/* Obvious */
/*------------------------------------------------------------*/


/* Ha, ha!! I did not realize C has a builtin XOR operator! */
#define XOR(a,b)  (unsigned char) (((a&~b)|(~a&b)) & 0377) /* NOTE: a and b should be unsigned char */


/* Conditional debugging output macros */

#ifndef NOSAY
#define say(when, what, how)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
			printf how;fflush(stdout);}\
	else
#else
#define say(when, what, how)	/* null macro; BEWARE: avoid side effects in say() */
#endif

/* the ones below are obsolete and are here for upward compatibility only */
#define say0(when, what, how)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
			printf(how);fflush(stdout);}
#define say1(when, what, how, x1)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
			printf(how, x1); fflush(stdout);}
#define say2(when, what, how, x1, x2)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
			printf(how, x1, x2); fflush(stdout);}
#define say3(when, what, how, x1, x2, x3)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
			printf(how, x1, x2, x3); fflush(stdout);}
#define say4(when, what, how, x1, x2, x3, x4)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
			printf(how, x1, x2, x3, x4); fflush(stdout);}
#define say5(when, what, how, x1, x2, x3, x4, x5)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
			printf(how, x1, x2, x3, x4, x5); fflush(stdout);}
#define say6(when, what, how, x1, x2, x3, x4, x5, x6)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
			printf(how, x1, x2, x3, x4, x5, x6); fflush(stdout);}
#define say7(when, what, how, x1, x2, x3, x4, x5, x6, x7)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
			printf(how, x1, x2, x3, x4, x5, x6, x7); fflush(stdout);}
#define say8(when, what, how, x1, x2, x3, x4, x5, x6, x7, x8)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
			printf(how, x1, x2, x3, x4, x5, x6, x7, x8); fflush(stdout);}
#define say9(when, what, how, x1, x2, x3, x4, x5, x6, x7, x8, x9)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
			printf(how, x1, x2, x3, x4, x5, x6, x7, x8, x9); fflush(stdout);}
#define say10(when, what, how, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
			printf(how, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10); fflush(stdout);}
#define say11(when, what, how, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
			printf(how, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11); fflush(stdout);}
#define say12(when, what, how, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12)\
	if (when < what){printf("\"%s\", line %d:    ", __FILE__, __LINE__);\
	    		printf(how, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12); fflush(stdout);}


/* length-checked string routines: return 0 on success, -1 on length violation */
#define SafeStrCat(dest,src,totalspace)\
    ((strlen(dest)+strlen(src) < totalspace) ? strcat(dest,src),0 : -1)

#define SafeStrCpy(dest,src,totalspace)\
    ((strlen(src) < totalspace) ? strcpy(dest,src),0 : -1)


/* The following definition of assert is slightly modified from the standard 4.2BSD one.
	This prints out the failing assertion, in addition to the file and line number.
	BEWARE:  avoid quotes in the assertion!!
	Also beware: you cannot make the NOASSERT case a null macro, because of side effects */

#ifndef NOASSERT
#define assert(ex) {if (!(ex)){fprintf(stderr,"Assertion failed: file %s, line %d\n", __FILE__, __LINE__);fprintf(stderr, "\t ex \n"); abort();}}
#else
#define assert(ex) {if (!(ex)) abort();}
#endif


#define TRUE 1
#define FALSE 0

#ifdef LWP
#define SystemError(y) (fprintf(stderr, "%d(%s): ", getpid(), LWP_ActiveProcess->name), perror(y))
#else
#define SystemError(y) (fprintf(stderr, "%d: ", getpid()), perror(y))
#endif
