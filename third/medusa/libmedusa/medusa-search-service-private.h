/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

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
 *  Authors: Rebecca Schulman <rebecka@eazel.com>
 *           Maciej Stachowiak <mjs@eazel.com> 
 *  
 */


/* medusa-search-service-private.h -- things that are internal to the
   search service client implementation and need to be shared with the
   search daemon. */

#ifndef MEDUSA_SEARCH_SERVICE_PRIVATE_H
#define MEDUSA_SEARCH_SERVICE_PRIVATE_H

#define MAX_LINE 512
#define SEARCH_SOCKET_PATH  "/tmp/medusa-search-server"

#define COOKIE_REQUEST "gimme cookie"
#define COOKIE_PATH "/tmp/medusa-cookies"
#define SEARCH_FILE_TRANSMISSION "File"
#define SEARCH_END_TRANSMISSION "End"
#define SEARCH_COOKIE_ACKNOWELEDGMENT_TRANSMISSION "Get your cookie\n"
#define SEARCH_INDEX_ERROR_TRANSMISSION "Index Error\n"

#endif /* MEDUSA_SEARCH_SERVICE_PRIVATE_H */


