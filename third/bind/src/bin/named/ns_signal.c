#if !defined(lint) && !defined(SABER)
static char sccsid[] = "@(#)ns_main.c	4.55 (Berkeley) 7/1/91";
static char rcsid[] = "$Id: ns_signal.c,v 1.1.1.1 1999-03-16 19:44:48 danw Exp $";
#endif /* not lint */

/*
 * Copyright (c) 1986, 1989, 1990
 *    The Regents of the University of California.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 	This product includes software developed by the University of
 * 	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 * Portions Copyright (c) 1996-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include "port_before.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#ifdef SVR4	/* XXX */
# include <sys/sockio.h>
#else
# include <sys/mbuf.h>
#endif

#include <netinet/in.h>
#include <net/route.h>
#include <net/if.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <pwd.h>
#include <resolv.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include <isc/eventlib.h>
#include <isc/logging.h>
#include <isc/memcluster.h>
#include <isc/list.h>

#include "port_after.h"
#include "named.h"

static void			set_signal_handler(int, SIG_FN (*)());

static SIG_FN
onhup(int sig) {
	ns_need(main_need_reload);
}

static SIG_FN
onintr(int sig) {
	ns_need(main_need_exit);
}

static SIG_FN
setdumpflg(int sig) {
	ns_need(main_need_dump);
}

#ifdef DEBUG
static SIG_FN
setIncrDbgFlg(int sig) {
	desired_debug++;
	ns_need(main_need_debug);
}

static SIG_FN
setNoDbgFlg(int sig) {
	desired_debug = 0;
	ns_need(main_need_debug);
}
#endif /*DEBUG*/

#if defined(QRYLOG) && defined(SIGWINCH)
static SIG_FN
setQrylogFlg(int sig) {
	ns_need(main_need_qrylog);
}
#endif /*QRYLOG && SIGWINCH*/

static SIG_FN
setstatsflg(int sig) {
	ns_need(main_need_statsdump);
}

static SIG_FN
discard_pipe(int sig) {
#ifdef SIGPIPE_ONE_SHOT
	int saved_errno = errno;
	set_signal_handler(SIGPIPE, discard_pipe);
	errno = saved_errno;
#endif
}

static SIG_FN
setreapflg(int sig) {
	ns_need(main_need_reap);
}


void
ns_need(int need) {
	needs[need] = 1;
}

static void
set_signal_handler(int sig, SIG_FN (*handler)()) {
	struct sigaction sa;

	memset(&sa, 0, sizeof sa);
	sa.sa_handler = handler;
	if (sigemptyset(&sa.sa_mask) < 0) {
		ns_error(ns_log_os,
			 "sigemptyset failed in set_signal_handler(%d): %s",
			 sig, strerror(errno));
		return;
	}
	if (sigaction(sig, &sa, NULL) < 0)
		ns_error(ns_log_os,
			 "sigaction failed in set_signal_handler(%d): %s",
			 sig, strerror(errno));
}

void
init_signals() {
	set_signal_handler(SIGINT, setdumpflg);
	set_signal_handler(SIGILL, setstatsflg);
#ifdef DEBUG
	set_signal_handler(SIGUSR1, setIncrDbgFlg);
	set_signal_handler(SIGUSR2, setNoDbgFlg);
#endif
	set_signal_handler(SIGHUP, onhup);
#if defined(SIGWINCH) && defined(QRYLOG)	/* XXX */
	set_signal_handler(SIGWINCH, setQrylogFlg);
#endif
	set_signal_handler(SIGCHLD, setreapflg);
	set_signal_handler(SIGPIPE, discard_pipe);
	set_signal_handler(SIGTERM, onintr);
#if defined(SIGXFSZ)	/* XXX */
	/* Wierd DEC Hesiodism, harmless. */
	set_signal_handler(SIGXFSZ, onhup);
#endif
}
