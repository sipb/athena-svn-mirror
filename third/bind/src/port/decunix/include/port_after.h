/*
 * Copyright (c) 1996 by Internet Software Consortium.
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

#include <sys/mbuf.h>
#include <sys/socket.h>
#include <net/route.h>

/* Digital UNIX underestimates itself. */
#if defined(BSD) && (BSD < 199103)
# undef BSD
# define BSD 199103
#endif

/* Digital UNIX deprecates send() which BIND uses. */
#undef send

#define USE_POSIX
#define USE_UTIME
#define POSIX_SIGNALS
#define NEED_STRSEP
#define USE_WAITPID
#define MAYBE_HESIOD
#define HAVE_GETRUSAGE
#define HAVE_FCHMOD
#define NEED_PSELECT
#define HAVE_SA_LEN
#define FIX_ZERO_SA_LEN
#define USE_LOG_CONS
#define RLIMIT_TYPE rlim_t
#define RLIMIT_FILE_INFINITY
#define HAVE_CHROOT
#define CAN_CHANGE_ID

#define _TIMEZONE	timezone
#define SIG_FN		void
#define PORT_NONBLOCK	O_NONBLOCK
#define PORT_WOULDBLK	EWOULDBLOCK
#define WAIT_T		int

#ifndef AF_INET6
# define AF_INET6	24
#endif

extern char *strsep(char **, const char *);

#define CHECK_UDP_SUM
#define FIX_UDP_SUM
#define KSYMS "/vmunix"
#define KMEM "/dev/kmem"
#define UDPSUM "udpcksum"
