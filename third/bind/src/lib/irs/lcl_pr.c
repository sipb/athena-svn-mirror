/*
 * Copyright (c) 1989, 1993, 1995
 *	The Regents of the University of California.  All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 * Portions Copyright (c) 1996 by Internet Software Consortium.
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
static const char rcsid[] = "$Id: lcl_pr.c,v 1.1.1.1 1998-05-04 22:23:41 ghudson Exp $";
#endif /* LIBC_SCCS and not lint */

/* extern */

#include "port_before.h"

#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "port_after.h"

#include "irs_p.h"
#include "lcl_p.h"

#ifndef _PATH_PROTOCOLS
#define _PATH_PROTOCOLS "/etc/protocols"
#endif
#define MAXALIASES      35

/* Types */

struct pvt {
	FILE *fp;
	char line[BUFSIZ+1];
	struct protoent proto;
	char *proto_aliases[MAXALIASES];
};

/* Forward */

static void			pr_close(struct irs_pr *);
static struct protoent *	pr_next(struct irs_pr *);
static struct protoent *	pr_byname(struct irs_pr *, const char *);
static struct protoent *	pr_bynumber(struct irs_pr *, int);
static void			pr_rewind(struct irs_pr *);
static void			pr_minimize(struct irs_pr *);

/* Portability. */

#ifndef SEEK_SET
# define SEEK_SET 0
#endif

/* Public */

struct irs_pr *
irs_lcl_pr(struct irs_acc *this) {
	struct irs_pr *pr;
	struct pvt *pvt;
	
	if (!(pr = (struct irs_pr *)malloc(sizeof *pr))) {
		errno = ENOMEM;
		return (NULL);
	}
	if (!(pvt = (struct pvt *)malloc(sizeof *pvt))) {
		free(pr);
		errno = ENOMEM;
		return (NULL);
	}
	memset(pvt, 0, sizeof *pvt);
	pr->private = pvt;
	pr->close = pr_close;
	pr->byname = pr_byname;
	pr->bynumber = pr_bynumber;
	pr->next = pr_next;
	pr->rewind = pr_rewind;
	pr->minimize = pr_minimize;
	return (pr);
}

/* Methods */

static void
pr_close(struct irs_pr *this) {
	struct pvt *pvt = (struct pvt *)this->private;

	if (pvt->fp)
		(void) fclose(pvt->fp);
	free(pvt);
	free(this);
}

static struct protoent *
pr_byname(struct irs_pr *this, const char *name) {
		
	struct protoent *p;
	char **cp;

	pr_rewind(this);
	while ((p = pr_next(this))) {
		if (!strcmp(p->p_name, name))
			goto found;
		for (cp = p->p_aliases; *cp; cp++)
			if (!strcmp(*cp, name))
				goto found;
	}
 found:
	return (p);
}

static struct protoent *
pr_bynumber(struct irs_pr *this, int proto) {
	struct protoent *p;

	pr_rewind(this);
	while ((p = pr_next(this)))
		if (p->p_proto == proto)
			break;
	return (p);
}

static void
pr_rewind(struct irs_pr *this) {
	struct pvt *pvt = (struct pvt *)this->private;
	
	if (pvt->fp) {
		if (fseek(pvt->fp, 0L, SEEK_SET) == 0)
			return;
		(void)fclose(pvt->fp);
	}
	if (!(pvt->fp = fopen(_PATH_PROTOCOLS, "r" )))
		return;
	if (fcntl(fileno(pvt->fp), F_SETFD, 1) < 0) {
		(void)fclose(pvt->fp);
		pvt->fp = NULL;
	}
}

static struct protoent *
pr_next(struct irs_pr *this) {
	struct pvt *pvt = (struct pvt *)this->private;
	char *p, *cp, **q;

	if (!pvt->fp)
		pr_rewind(this);
	if (!pvt->fp)
		return (NULL);
 again:
	if ((p = fgets(pvt->line, BUFSIZ, pvt->fp)) == NULL)
		return (NULL);
	if (*p == '#')
		goto again;
	cp = strpbrk(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';
	pvt->proto.p_name = p;
	cp = strpbrk(p, " \t");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	p = strpbrk(cp, " \t");
	if (p != NULL)
		*p++ = '\0';
	pvt->proto.p_proto = atoi(cp);
	q = pvt->proto.p_aliases = pvt->proto_aliases;
	if (p != NULL) {
		cp = p;
		while (cp && *cp) {
			if (*cp == ' ' || *cp == '\t') {
				cp++;
				continue;
			}
			if (q < &pvt->proto_aliases[MAXALIASES - 1])
				*q++ = cp;
			cp = strpbrk(cp, " \t");
			if (cp != NULL)
				*cp++ = '\0';
		}
	}
	*q = NULL;
	return (&pvt->proto);
}

static void
pr_minimize(struct irs_pr *this) {
	struct pvt *pvt = (struct pvt *)this->private;

	if (pvt->fp != NULL) {
		(void)fclose(pvt->fp);
		pvt->fp = NULL;
	}
}
