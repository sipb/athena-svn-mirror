/* gnome program that crashes
 *
 * Copyright (C) Jacob Berkman
 *
 * Author: Jacob Berkman  <jacob@bug-buddy.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
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

#include "config.h"

#include <gnome.h>

int
main (int argc, char *argv[])
{
	int *n = NULL;
	gnome_program_init ("Crashing GNOME Program", VERSION,
			    LIBGNOMEUI_MODULE,
			    argc, argv,
			    GNOME_CLIENT_PARAM_SM_CONNECT, FALSE,
			    NULL);
	n[27] = 10-7-78;
	return 0;
}
