/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 *  Medusa
 *
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Rebecca Schulman <rebecka@eazel.com>
 */

/* medusa-master-db-private.h:  Private definition of the 
 * master_db structure */

#ifndef MEDUSA_MASTER_DB_PRIVATE_H
#define MEDUSA_MASTER_DB_PRIVATE_H

#include "medusa-text-index.h"
#include "medusa-uri-list.h"
#include "medusa-file-index.h"

#include <libmedusa/medusa-index-progress.h>

struct MedusaMasterDB {
	MedusaTextIndex *text_index;
	MedusaFileSystemDB *file_system_db;
	MedusaURIList *uri_list;
	gboolean indexing_mode;
	MedusaIndexProgress *index_progress;
	char *root_uri;
	MedusaLogLevel log_level;
	int ref_count;
};



#endif /* MEDUSA_MASTER_DB_PRIVATE_H */
