/*
 * Copyright (c) 1996,1999 by Internet Software Consortium.
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

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$Id: getnetgrent.c,v 1.1.1.1 2002-02-03 04:24:08 ghudson Exp $";
#endif /* LIBC_SCCS and not lint */

/* Imports */

#include "port_before.h"

#if !defined(__BIND_NOSTATIC)

#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/nameser.h>

#include <errno.h>
#include <resolv.h>
#include <stdio.h>

#include <irs.h>

#include "port_after.h"

#include "irs_data.h"

/* Forward */

static struct net_data *init(void);


/* Public */

void
setnetgrent(const char *netgroup) {
	struct net_data *net_data = init();

	setnetgrent_p(netgroup, net_data);
}

void
endnetgrent(void) {
	struct net_data *net_data = init();

	endnetgrent_p(net_data);
}

int
innetgr(const char *netgroup, const char *host,
	const char *user, const char *domain) {
	struct net_data *net_data = init();

	return (innetgr_p(netgroup, host, user, domain, net_data));
}

int
getnetgrent(const char **host, const char **user, const char **domain) {
	struct net_data *net_data = init();

	return (getnetgrent_p(host, user, domain, net_data));
}

/* Shared private. */

void
setnetgrent_p(const char *netgroup, struct net_data *net_data) {
	struct irs_ng *ng;

	if ((net_data != NULL) && ((ng = net_data->ng) != NULL))
		(*ng->rewind)(ng, netgroup);
}

void
endnetgrent_p(struct net_data *net_data) {
	struct irs_ng *ng;

	if (!net_data)
		return;
	if ((ng = net_data->ng) != NULL)
		(*ng->close)(ng);
	net_data->ng = NULL;
}

int
innetgr_p(const char *netgroup, const char *host,
	  const char *user, const char *domain,
	  struct net_data *net_data) {
	struct irs_ng *ng;

	if (!net_data || !(ng = net_data->ng))
		return (0);
	return ((*ng->test)(ng, netgroup, host, user, domain));
}

int
getnetgrent_p(const char **host, const char **user, const char **domain,
	      struct net_data *net_data ) {
	struct irs_ng *ng;

	if (!net_data || !(ng = net_data->ng))
		return (0);
	return ((*ng->next)(ng, host, user, domain));
}

/* Private */

static struct net_data *
init(void) {
	struct net_data *net_data;

	if (!(net_data = net_data_init(NULL)))
		goto error;
	if (!net_data->ng) {
		net_data->ng = (*net_data->irs->ng_map)(net_data->irs);
		if (!net_data->ng) {
  error:
			errno = EIO;
			return (NULL);
		}
	}
	
	return (net_data);
}

#endif /*__BIND_NOSTATIC*/