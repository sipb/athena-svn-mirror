/* $Header: /afs/dev.mit.edu/source/repository/third/patch/version.c,v 1.1.1.1 1996-10-06 21:29:01 ghudson Exp $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  86/09/17  15:40:11  lwall
 * Baseline for netwide release.
 * 
 */

#include "EXTERN.h"
#include "common.h"
#include "util.h"
#include "INTERN.h"
#include "patchlevel.h"
#include "version.h"

void my_exit();

/* Print out the version number and die. */

void
version()
{
    fprintf(stderr, "Patch version %s\n", PATCH_VERSION);
    my_exit(0);
}
