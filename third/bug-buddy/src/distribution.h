/* bug-buddy bug submitting program
 *
 * Copyright (C) Jacob Berkman
 * Copyright (C) Fernando Herrera
 *
 * Author:  		Fernando Herrera <fherrera@onirica.com>
 * Based on code by:  	Jacob Berkman  <jberkman@andrew.cmu.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __DISTRIBUTION_H__
#define __DISTRIBUTION_H__

#include <glib.h>

typedef struct _Package      Package;
typedef struct _Phylum       Phylum;
typedef struct _Distribution Distribution;

typedef char *(*DistroVersionFunc) (Distribution *distro);

struct _Phylum {
	DistroVersionFunc version;
};


struct _Distribution {
	char *name;
	char *version_file;
	Phylum *phylum;
};

extern Phylum debian_phy;
extern Phylum redhat_phy;
extern Phylum turbolinux_phy;
extern Phylum irix_phy;

static Distribution distros[] = {
	{ "Slackware",     "/etc/slackware-version",  &debian_phy },
	{ "Debian",        "/etc/debian_version",     &debian_phy },
	{ "Mandrake",      "/etc/mandrake-release",   &redhat_phy },
	{ "TurboLinux",    "/etc/turbolinux-release", &turbolinux_phy },
	{ "SuSE",          "/etc/SuSE-release",       &redhat_phy },
	{ "Red Hat",       "/etc/redhat-release",     &redhat_phy },
	{ "Fedora",        "/etc/fedora-release",     &redhat_phy },
	{ "Gentoo",        "/etc/gentoo-release",     &redhat_phy },
	{ "Solaris",       "/etc/release",  	      &redhat_phy },
	{ "IRIX Freeware", "/bin/hinv",               &irix_phy },
	{ NULL }
};


#endif /* __DISTRIBUTION_H__ */
