/* $Header: /afs/dev.mit.edu/source/repository/third/mh/miscellany/patch-2.0.12u8/EXTERN.h,v 1.1.1.1 1996-10-07 07:13:14 ghudson Exp $
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
