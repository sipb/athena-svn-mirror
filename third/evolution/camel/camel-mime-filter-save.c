/*
 *  Copyright (C) 2000 Ximian Inc.
 *
 *  Authors: Michael Zucchi <notzed@ximian.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include <errno.h>

#include "camel-mime-filter-save.h"

static void camel_mime_filter_save_class_init (CamelMimeFilterSaveClass *klass);
static void camel_mime_filter_save_init       (CamelMimeFilterSave *obj);
static void camel_mime_filter_save_finalize   (CamelObject *o);

static CamelMimeFilterClass *camel_mime_filter_save_parent;

CamelType
camel_mime_filter_save_get_type (void)
{
	static CamelType type = CAMEL_INVALID_TYPE;
	
	if (type == CAMEL_INVALID_TYPE) {
		type = camel_type_register (camel_mime_filter_get_type (), "CamelMimeFilterSave",
					    sizeof (CamelMimeFilterSave),
					    sizeof (CamelMimeFilterSaveClass),
					    (CamelObjectClassInitFunc) camel_mime_filter_save_class_init,
					    NULL,
					    (CamelObjectInitFunc) camel_mime_filter_save_init,
					    (CamelObjectFinalizeFunc) camel_mime_filter_save_finalize);
	}
	
	return type;
}

static void
camel_mime_filter_save_finalize(CamelObject *o)
{
	CamelMimeFilterSave *f = (CamelMimeFilterSave *)o;

	g_free(f->filename);
	if (f->fd != -1) {
		/* FIXME: what do we do with failed writes???? */
		close(f->fd);
	}
}

static void
reset(CamelMimeFilter *mf)
{
	CamelMimeFilterSave *f = (CamelMimeFilterSave *)mf;

	/* i dunno, how do you 'reset' a file?  reopen it? do i care? */
	if (f->fd != -1){
		lseek(f->fd, 0, SEEK_SET);
	}
}

/* all this code just to support this little trivial filter! */
static void
filter(CamelMimeFilter *mf, char *in, size_t len, size_t prespace, char **out, size_t *outlen, size_t *outprespace)
{
	CamelMimeFilterSave *f = (CamelMimeFilterSave *)mf;

	if (f->fd != -1) {
		/* FIXME: check return */
		int outlen = write(f->fd, in, len);
		if (outlen != len) {
			g_warning("could not write to '%s': %s", f->filename?f->filename:"<descriptor>", strerror(errno));
		}
	}
	*out = in;
	*outlen = len;
	*outprespace = prespace;
}

static void
camel_mime_filter_save_class_init (CamelMimeFilterSaveClass *klass)
{
	CamelMimeFilterClass *filter_class = (CamelMimeFilterClass *) klass;

	camel_mime_filter_save_parent = CAMEL_MIME_FILTER_CLASS (camel_type_get_global_classfuncs (camel_mime_filter_get_type ()));

	filter_class->reset = reset;
	filter_class->filter = filter;
}

static void
camel_mime_filter_save_init (CamelMimeFilterSave *f)
{
	f->fd = -1;
}

/**
 * camel_mime_filter_save_new:
 *
 * Create a new CamelMimeFilterSave object.
 * 
 * Return value: A new CamelMimeFilterSave widget.
 **/
CamelMimeFilterSave *
camel_mime_filter_save_new (void)
{
	CamelMimeFilterSave *new = CAMEL_MIME_FILTER_SAVE ( camel_object_new (camel_mime_filter_save_get_type ()));
	return new;
}

CamelMimeFilterSave *
camel_mime_filter_save_new_name (const char *name, int flags, int mode)
{
	CamelMimeFilterSave *new = NULL;

	new = camel_mime_filter_save_new();
	if (new) {
		new->fd = open(name, flags, mode);
		if (new->fd != -1) {
			new->filename = g_strdup(name);
		} else {
			camel_object_unref((CamelObject *)new);
			new = NULL;
		}
	}
	return new;
}

