/* $Header: /afs/dev.mit.edu/source/repository/third/patch/EXTERN.h,v 1.1.1.1 1996-10-06 21:29:01 ghudson Exp $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  86/09/17  15:35:37  lwall
 * Baseline for netwide release.
 * 
 */

#ifdef EXT
#undef EXT
#endif
#define EXT extern

#ifdef INIT
#undef INIT
#endif
#define INIT(x)

#ifdef DOINIT
#undef DOINIT
#endif
