/*
 * pam_debathena_home_type.c
 * PAM session management functions for pam_debathena_home_type.so
 *
 * Copyright © 2007 Tim Abbott <tabbott@mit.edu>
 *
 * Copyright © 2011 Massachusetts Institute of Technology
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <syslog.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_misc.h>

#define AFS "/afs/"     /* What constitutes the beginning of a path in AFS */

/* Set DEBATHENA_HOME_TYPE based on whether the homedir is in AFS or local */
int
pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    int i;
    int debug = 0;
    int pamret;
    const char *user;
    struct passwd *pw;

    for (i = 0; i < argc; i++) {
	if (strcmp(argv[i], "debug") == 0)
	    debug = 1;
    }

    if ((pamret = pam_get_user(pamh, &user, NULL)) != PAM_SUCCESS) {
	syslog(LOG_ERR, "pam_debathena_home_type: pam_get_user: %s:%d",
	       pam_strerror(pamh, pamret), pamret);
	return PAM_SESSION_ERR;
    }

    errno = 0;
    pw = getpwnam(user);
    if (pw == NULL) {
	if (errno != 0)
	    syslog(LOG_ERR, "pam_debathena_home_type: getpwnam: %s",
		   strerror(errno));
	else
	    syslog(LOG_ERR, "pam_debathena_home_type: no such user: %s", user);
	return PAM_SESSION_ERR;
    }
    if (debug)
	syslog(LOG_DEBUG, "pam_debathena_home_type: user=%s home=%s",
	       user, pw->pw_dir);

    char *resolved_path = realpath(pw->pw_dir, NULL);
    if (resolved_path != NULL) {
	if (strncmp(resolved_path, AFS, strlen(AFS)) == 0) {
	    if (debug)
		syslog(LOG_DEBUG, "pam_debathena_home_type: AFS homedir %s", user);
	    pam_putenv(pamh, "DEBATHENA_HOME_TYPE=afs");
	} else {
	    if (debug)
		syslog(LOG_DEBUG, "pam_debathena_home_type: Local homedir %s", user);
	    pam_putenv(pamh, "DEBATHENA_HOME_TYPE=local");
	}
    } else {
	syslog(LOG_ERR, "pam_debathena_home_type: %s while calling realpath() on %s", strerror(errno), pw->pw_dir);
	if (errno == ENOENT) {
	    pam_putenv(pamh, "DEBATHENA_HOME_TYPE=missing");
	} else {
	    pam_putenv(pamh, "DEBATHENA_HOME_TYPE=unknown");
	}
    }
    free(resolved_path);
    return PAM_SUCCESS;
}

int
pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}

int
pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return pam_sm_open_session(pamh, flags, argc, argv);
}

int
pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_IGNORE;
}

