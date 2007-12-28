/*
 * pam_athena-locker.c
 * PAM session management functions for pam_athena_locker.so
 *
 * Copyright © 2007 Tim Abbott <tabbott@mit.edu> and Anders Kaseorg
 * <andersk@mit.edu>
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
#include <unistd.h>
#include <syslog.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>
#include <errno.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_misc.h>
#include <afs/afssyscalls.h>

/* Initiate session management by attaching locker. */
int
pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    int i;
    int debug = 0;
    int pamret;
    const char *user;
    struct passwd *pw;
    const char *filecache;
    pid_t pid;
    int status;
    struct sigaction act, oldact;

    for (i = 0; i < argc; i++) {
	if (strcmp(argv[i], "debug") == 0)
	    debug = 1;
    }

    if ((pamret = pam_get_user(pamh, &user, NULL)) != PAM_SUCCESS) {
	syslog(LOG_ERR, "pam_athena_locker: pam_get_user: %s:%d", pam_strerror(pamh, pamret), pamret);
	return PAM_SESSION_ERR;
    }

    errno = 0;
    pw = getpwnam(user);
    if (pw == NULL) {
	if (errno != 0)
	    syslog(LOG_ERR, "pam_athena_locker: getpwnam: %s", strerror(errno));
	else
	    syslog(LOG_ERR, "pam_athena_locker: no such user: %s", user);
	return PAM_SESSION_ERR;
    }

    if (lsetpag() != 0) {
	syslog(LOG_ERR, "pam_athena_locker: Could not create a PAG");
	return PAM_SESSION_ERR;
    }

    if ((filecache = pam_getenv(pamh, "KRB5CCNAME")) == NULL) {
	syslog(LOG_ERR, "pam_athena_locker: No Kerberos ticket cache; not attaching locker");
	return PAM_SESSION_ERR;
    }

    /* Override gdm's SIGCHLD handler that makes waitpid() return -1.
       Maybe this leads to some race condition if gdm used that at the time? */
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_DFL;
    sigaction(SIGCHLD, &act, &oldact);

    pid = fork();
    if (pid == -1) {
	syslog(LOG_ERR, "pam_athena_locker: fork(): %s", strerror(errno));
	return PAM_SESSION_ERR;
    }
    if (pid == 0) {
	if (setenv("KRB5CCNAME", filecache, 1) != 0) {
	    syslog(LOG_ERR, "pam_athena_locker: setenv(): %s", strerror(errno));
	    _exit(1);
	}
	if (debug)
	    syslog(LOG_DEBUG, "pam_athena_locker: uid=%d euid=%d", getuid(), geteuid());
	if (setuid(pw->pw_uid) != 0) {
	    syslog(LOG_ERR, "pam_athena_locker: setuid(): %s", strerror(errno));
	    return PAM_SESSION_ERR;
	}
	if (debug)
	    syslog(LOG_DEBUG, "pam_athena_locker: uid=%d euid=%d", getuid(), geteuid());
	if (close(1) < 0) {
	    syslog(LOG_ERR, "pam_debathena_home_type: close(): %s",
		   strerror(errno));
	    _exit(-1);
	}
	if (close(2) < 0) {
	    syslog(LOG_ERR, "pam_debathena_home_type: close(): %s",
		   strerror(errno));
	    _exit(-1);
	}
	execl("/bin/attach", "attach", "--quiet", "--nozephyr", user, NULL);
	syslog(LOG_ERR, "pam_athena_locker: execl(): %s", strerror(errno));
	_exit(1);
    }
    if (TEMP_FAILURE_RETRY(waitpid(pid, &status, 0)) == -1 ||
	!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
	syslog(LOG_ERR, "pam_athena_locker: locker attachment failed: %s", user);
	return PAM_SESSION_ERR;
    }
    if (debug)
	syslog(LOG_DEBUG, "pam_athena_locker: attached locker %s", user);
    sigaction(SIGCHLD, &oldact, NULL);
    return PAM_SUCCESS;
}

/* Terminate session management. */
int
pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}

int
pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    if (flags == PAM_ESTABLISH_CRED)
	return pam_sm_open_session(pamh, flags, argc, argv);
    return PAM_SUCCESS;
}

int
pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}

