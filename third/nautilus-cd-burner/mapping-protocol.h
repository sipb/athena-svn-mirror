/* mapping-protocol.h - code to talk with the mapping daemon
 *
 *  Copyright (C) 2002 Red Hat Inc,
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Alexander Larsson <alexl@redhat.com>
 */
#include <glib.h>

typedef struct {
  gint32 operation;
  char *root;
  char *path1;
  char *path2;
  gboolean option;
} MappingRequest;

typedef struct {
  guint32 result;
  char *path;
  gboolean option;
  int n_strings;
  char **strings;
} MappingReply;

typedef enum {
  MAPPING_GET_BACKING_FILE,
  MAPPING_CREATE_DIR,
  MAPPING_REMOVE_DIR,
  MAPPING_REMOVE_FILE,
  MAPPING_CREATE_FILE,
  MAPPING_CREATE_LINK,
  MAPPING_MOVE_FILE,
  MAPPING_LIST_DIR
} MappingOps;

int encode_request (int fd,
		    gint32 operation,
		    char *root,
		    char *path1,
		    char *path2,
		    gboolean option);

int decode_request (int fd,
		    MappingRequest *req);

void destroy_request (MappingRequest *req);

int encode_reply (int fd,
		  MappingReply *reply);

int decode_reply (int fd,
		  MappingReply *reply);

void destroy_reply (MappingReply *reply);

