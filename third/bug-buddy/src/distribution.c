/* bug-buddy bug submitting program
 *
 * Copyright (C) Jacob Berkman
 *
 * Author:  Jacob Berkman  <jberkman@andrew.cmu.edu>
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

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libgnome/libgnome.h>
#include "config.h"
#include "bug-buddy.h"
#include "distribution.h"

#define d(x)

static char *get_redhat_version (Distribution *distro);
static char *get_turbolinux_version (Distribution *distro);
static char *get_debian_version (Distribution *distro);
static char *get_irix_version (Distribution *distro);

Phylum redhat_phy = {
	get_redhat_version
};

Phylum turbolinux_phy = {
	get_turbolinux_version
};

Phylum debian_phy = {
	get_debian_version
};

Phylum irix_phy = {
	get_irix_version
};




DistroVersionFunc redhat_ver = get_redhat_version;
DistroVersionFunc turbolinux_ver = get_turbolinux_version;
DistroVersionFunc debian_ver = get_debian_version;
DistroVersionFunc irix_ver =  get_irix_version;


char *
get_line_from_fd (int fd)
{
	char buf[1024];
	int pos;

	buf[0] = '\0';
	for (pos = 0; pos < 1023; pos++)
		if (read (fd, buf+pos, 1) < 1 ||
		    buf[pos] == '\n')
			break;
	
	if (pos == 0 && buf[0] == '\0')
		return NULL;

	buf[pos] = '\0';
	return g_strdup (buf);
}

char *
get_line_from_file (const char *filename)
{
	char *retval;
	int fd;

	g_return_val_if_fail (filename, NULL);

	//if (!g_file_exists (filename))
	if (!g_file_test (filename, G_FILE_TEST_EXISTS))
		return NULL;

	fd = open (filename, O_RDONLY);
	if (fd == -1) {
		d(g_warning ("Could not open file '%s' for reading", filename));
		return NULL;
	}
	
	retval = get_line_from_fd (fd);
	close (fd);
	return retval;
}

static char *
get_redhat_version (Distribution *distro)
{
	g_return_val_if_fail (distro, NULL);
	g_return_val_if_fail (distro->version_file, NULL);

	return get_line_from_file (distro->version_file);
}

static char *
get_turbolinux_version (Distribution *distro)
{
	char *tmp, *ret;
	g_return_val_if_fail (distro, NULL);
	g_return_val_if_fail (distro->version_file, NULL);

	tmp = get_line_from_file (distro->version_file);
	ret = g_strdup_printf ("TurboLinux %s", tmp);
	g_free (tmp);
	return ret;
}


static char *
get_debian_version (Distribution *distro)
{
	char *retval, *version;

	g_return_val_if_fail (distro, NULL);
	g_return_val_if_fail (distro->version_file, NULL);
	g_return_val_if_fail (distro->name, NULL);

	version = get_line_from_file (distro->version_file);
	if (!version) {
		d(g_warning ("Could not get distro version"));
		return NULL;
	}
	
	retval = g_strdup_printf ("%s %s", distro->name, version);
	g_free (version);

	return retval;
}


static char *
get_irix_version (Distribution *distro)
{
	g_return_val_if_fail (distro, NULL);
	g_return_val_if_fail (distro->version_file, NULL);
	return "Irix";
/*
	return get_line_from_command ("/usr/bin/echo -n SGI `/sbin/uname -Rs | /bin/cut -d' ' -f1,3-`; "
				      "versions -b fw_common | awk 'NR >= 4 { print \", Freeware\",$3 }'");*/
}

void get_distro_name_from_file (void)
{
	int i;
        for (i = 0; distros[i].name; i++) {
		if (!g_file_test (distros[i].version_file, G_FILE_TEST_EXISTS))
			continue;
		druid_data.distro = distros[i].phylum->version (&distros[i]);
		break;
	}
}

void get_distro_name (void)
{
	get_distro_name_from_file ();
	if (!druid_data.distro) {
		druid_data.distro = g_strdup ("Unknown");
	}
}

