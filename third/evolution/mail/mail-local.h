/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* mail-local.h: Local mailbox support. */

/* 
 * Authors: 
 *  Michael Zucchi <NotZed@ximian.com>
 *  Dan Winship <danw@ximian.com>
 *
 * Copyright 2000 Ximian, Inc. (www.ximian.com)
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef _MAIL_LOCAL_H
#define _MAIL_LOCAL_H

#include "evolution-shell-client.h"
#include <camel/camel-folder.h>

void mail_local_storage_startup (EvolutionShellClient *shellclient, const char *evolution_path);
void mail_local_storage_shutdown (void);

void mail_local_reconfigure_folder(const char *uri, void (*done)(const char *uri, CamelFolder *folder, void *data), void *done_data);

#endif