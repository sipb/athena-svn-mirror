/* $Id: loadavg.c,v 1.1.1.1 2003-01-02 04:56:09 ghudson Exp $ */

/* Copyright (C) 1998-99 Martin Baulig
   This file is part of LibGTop 1.0.

   Contributed by Martin Baulig <martin@home-of-linux.org>, April 1998.

   LibGTop is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License,
   or (at your option) any later version.

   LibGTop is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with LibGTop; see the file COPYING. If not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <glibtop/error.h>
#include <glibtop/loadavg.h>

#include "kernel.h"

static const unsigned long _glibtop_sysdeps_loadavg =
(1 << GLIBTOP_LOADAVG_LOADAVG) + (1 << GLIBTOP_LOADAVG_NR_RUNNING) +
(1 << GLIBTOP_LOADAVG_NR_TASKS) + (1 << GLIBTOP_LOADAVG_LAST_PID);

/* Init function. */

void
glibtop_init_loadavg_s (glibtop *server)
{
	server->sysdeps.loadavg = _glibtop_sysdeps_loadavg;
}

/* Provides load load averange. */

void
glibtop_get_loadavg_s (glibtop *server, glibtop_loadavg *buf)
{
	union table tbl;
	
	glibtop_init_s (&server, GLIBTOP_SYSDEPS_LOADAVG, 0);

	memset (buf, 0, sizeof (glibtop_loadavg));

	if (table (TABLE_LOADAVG, &tbl, NULL))
		glibtop_error_io_r (server, "table(TABLE_LOADAVG)");
	
	buf->flags = _glibtop_sysdeps_loadavg;

	buf->loadavg [0] = tbl.loadavg.loadavg [0];
	buf->loadavg [1] = tbl.loadavg.loadavg [1];
	buf->loadavg [2] = tbl.loadavg.loadavg [2];

	buf->nr_running = tbl.loadavg.nr_running;
	buf->nr_tasks = tbl.loadavg.nr_tasks;
	buf->last_pid = tbl.loadavg.last_pid;
}