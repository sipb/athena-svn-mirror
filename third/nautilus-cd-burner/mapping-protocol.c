/* mapping-protocol.c - code to talk with the mapping daemon
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
#include "mapping-protocol.h"
#include <unistd.h>
#include <string.h>

static int
write_all (int fd, char *buf, int len)
{
	int bytes;
	int res;
	
	bytes = 0;
	while (bytes < len) {
		res = write (fd, buf + bytes, len - bytes);
		if (res <= 0) {
			if (res == 0)
				res = -1;
			return res;
		}
		bytes += res;
	}
	
	return 0;
}

static int
read_all (int fd, char *buf, int len)
{
	int bytes;
	int res;
	
	bytes = 0;
	while (bytes < len) {
		res = read (fd, buf + bytes, len - bytes);
		if (res <= 0) {
			if (res == 0)
				res = -1;
			return res;
		}
		bytes += res;
	}
	
	return 0;
}

static int
encode_int (int fd, gint32 val)
{
	char *ptr;

	val = g_htonl (val);

	ptr = (char *) &val;
	return write_all (fd, ptr, 4);
}

static int
decode_int (int fd, gint32 *out_val)
{
	unsigned char ptr[4];
	guint32 val;
	int res;
	
	res = read_all (fd, ptr, 4);
	if (res != 0)
		return res;

	val = ptr[0] << 24 |
		ptr[1] << 16 |
		ptr[2] << 8 |
		ptr[3];

	*out_val = (gint32)val;
	return 0;
}

static int
encode_string (int fd, char *str)
{
	int len;
	int res;
	
	if (str == NULL) {
		res = encode_int (fd, -1);
	} else {
		len = strlen (str);
		res = encode_int (fd, len);
		if (res == 0) {
			res = write_all (fd, str, len);
		}
	}
	return res;
}

static int
decode_string (int fd, char **out)
{
	int len;
	int res;
	char *str;

	res = decode_int (fd, &len);
	if (res != 0)
		return res;

	if (len == -1) {
		*out = NULL;
		return 0;
	}

	str = g_malloc (len + 1);
	res = read_all (fd, str, len);
	if (res != 0) {
		g_free (str);
		return res;
	}
	str[len] = 0;

	*out = str;
	return 0;
}

int
encode_request (int fd,
		gint32 operation,
		char *root,
		char *path1,
		char *path2,
		gboolean option)
{
	int res;

	res = encode_int (fd, operation);
	if (res != 0) return res;
	
	res = encode_string (fd, root);
	if (res != 0) return res;

	res = encode_string (fd, path1);
	if (res != 0) return res;

	res = encode_string (fd, path2);
	if (res != 0) return res;

	res = encode_int (fd, option);
	if (res != 0) return res;

	return 0;
}

int
decode_request (int fd,
		MappingRequest *req)
{
	int res;
	
	memset (req, 0, sizeof (MappingRequest));
	
	res = decode_int (fd, &req->operation);
	if (res != 0) return res;
	
	res = decode_string (fd, &req->root);
	if (res != 0) return res;

	res = decode_string (fd, &req->path1);
	if (res != 0) return res;

	res = decode_string (fd, &req->path2);
	if (res != 0) return res;

	res = decode_int (fd, &req->option);
	if (res != 0) return res;

	return 0;
}

void
destroy_request (MappingRequest *req)
{
	g_free (req->root);
	g_free (req->path1);
	g_free (req->path2);
}

int
encode_reply (int fd,
	      MappingReply *reply)
{
	int res;
	int i;

	res = encode_int (fd, reply->result);
	if (res != 0) return res;
	
	res = encode_string (fd, reply->path);
	if (res != 0) return res;

	res = encode_int (fd, reply->option);
	if (res != 0) return res;

	res = encode_int (fd, reply->n_strings);
	if (res != 0) return res;

	for (i = 0; i < reply->n_strings; i++) {
		res = encode_string (fd, reply->strings[i]);
		if (res != 0) return res;
	}
	
	return 0;
}

int
decode_reply (int fd,
	      MappingReply *reply)
{
	int res;
	int i;

	res = decode_int (fd, &reply->result);
	if (res != 0) return res;
	
	res = decode_string (fd, &reply->path);
	if (res != 0) return res;

	res = decode_int (fd, &reply->option);
	if (res != 0) return res;

	res = decode_int (fd, &reply->n_strings);
	if (res != 0) return res;

	reply->strings = g_new0 (char *, reply->n_strings);
	
	for (i = 0; i < reply->n_strings; i++) {
		res = decode_string (fd, &reply->strings[i]);
		if (res != 0) {
			g_free (reply->strings);
			reply->strings = NULL;
			return res;
		}
	}
	
	return 0;
}

void
destroy_reply (MappingReply *reply)
{
	/* TODO */
}
