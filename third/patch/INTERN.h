/* $Header: /afs/dev.mit.edu/source/repository/third/patch/INTERN.h,v 1.1.1.1 1996-10-06 21:29:01 ghudson Exp $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  86/09/17  15:35:58  lwall
 * Baseline for netwide release.
 * 
 */

#ifdef EXT
#undef EXT
#endif
#define EXT

#ifdef INIT
#undef INIT
#endif
#define INIT(x) = x

#define DOINIT
