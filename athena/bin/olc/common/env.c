/* Copyright (C) 1997 by the Massachusetts Institute of Technology
 * 
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * M.I.T. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability
 * of this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#include <mit-copyright.h>
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <common.h>

/* Set an environment variable.
 * Arguments:   var: a string containing the name of the variable.
 *              value: a string containing the new value.
 * Returns:     nothing.
 * Non-local returns: on some platforms, exits with code 8 if malloc fails.
 */
void set_env_var(const char *var, const char *value)
{
#ifdef HAVE_PUTENV
  char *buf = malloc(strlen(var)+strlen(value)+2);
  if (buf == NULL)
    {
      fprintf(stderr, "Out of memory, can't expand environment.\n");
      exit(8);
    }
  sprintf(buf, "%s=%s", var, value);
  putenv(buf);
#else /* don't HAVE_PUTENV */
  setenv (var, value, 1);
#endif /* don't HAVE_PUTENV */
}
