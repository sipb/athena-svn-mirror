/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* GnomePrint font-face wrapper
 *
 * Copyright (C) 2003 Filip Van Raemdonck <mechanix@debian.org>
 * 	              Martin Kretzschmar <m_kretzschmar@gmx.net>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <aconf.h>

#ifdef HAVE_FONT_EMBEDDING

#include "gpdf-font-face.h"
#include <unistd.h>

static GObjectClass* parent_class;

static void gpdf_font_face_init (GPdfFontFace* gff) {}

static void
gpdf_font_face_finalize (GObject* object) {
	GPdfFontFace* face = GPDF_FONT_FACE (object);

	if (face->font_data) {
		g_free (face->font_data);
		face->font_data = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpdf_font_face_class_init (GPdfFontFaceClass* klass) {
	parent_class = g_type_class_peek_parent (klass);
	G_OBJECT_CLASS (klass)->finalize = gpdf_font_face_finalize;
}

GType
gpdf_font_face_get_type (void) {
	static GType gff_type = 0;

	if (!gff_type) {
		static const GTypeInfo gff_info = {
			sizeof (GPdfFontFaceClass), NULL, NULL,
			(GClassInitFunc) gpdf_font_face_class_init,
			NULL, NULL, sizeof (GPdfFontFace), 0,
			(GInstanceInitFunc) gpdf_font_face_init
		};

		gff_type = g_type_register_static (GNOME_TYPE_FONT_FACE, "GPdfFontFace", &gff_info, (GTypeFlags) 0);
	}
	return gff_type;
}

/* from /usr/share/misc/magic of debian's file 3.40 */
#define TRUETYPE_MAGIC "\000\001\000\000\000"
#define TRUETYPE_MAGIC_LENGTH 5
#define TYPE1_MAGIC "%!PS-AdobeFont-1."
#define TYPE1_MAGIC_LENGTH 17

static GPFontEntryType
gff_font_entry_type_from_data (const guchar* font_data, gsize length) {
	if (length > TYPE1_MAGIC_LENGTH + 6
		 && !memcmp (font_data + 6, TYPE1_MAGIC, TYPE1_MAGIC_LENGTH)) {
		return GP_FONT_ENTRY_TYPE1; /* PFB */
	} else if (length > TYPE1_MAGIC_LENGTH
		 && !memcmp (font_data, TYPE1_MAGIC, TYPE1_MAGIC_LENGTH)) {
		return GP_FONT_ENTRY_TYPE1; /* PFA */
	} else if (length > TRUETYPE_MAGIC_LENGTH 
	    && !memcmp (font_data, TRUETYPE_MAGIC, TRUETYPE_MAGIC_LENGTH)) {
		return GP_FONT_ENTRY_TRUETYPE;
	}
	return GP_FONT_ENTRY_UNKNOWN; /* else */
}

static gchar*
gff_write_temp_data_file (const guchar* font_data, gsize length) {
	gint fd;
	gchar* temp_name;
	FILE* file;
	size_t written;

	fd = g_file_open_tmp ("gnome-print-XXXXXX", &temp_name, NULL);
	if (fd <= -1) {
		return NULL;
	}

	file = fdopen (fd, "wb");
	written = fwrite (font_data, length, 1, file);
	fclose (file);
	if (written != 1) {
		unlink (temp_name);
		return NULL;
	}

	return temp_name;
}

static void
gff_entry_fill_in_face (GPFontEntry* e) {
	GPdfFontFace* face;

	g_return_if_fail (e->face == NULL);

	face = g_object_new (GPDF_FONT_FACE_TYPE, NULL);

	gp_font_entry_ref (e);
	GNOME_FONT_FACE (face)->entry = e;
	e->face = GNOME_FONT_FACE (face);
}

static void
gff_unlink_temp_file (gpointer data, GObject* object) {
	unlink ((gchar*) data);
}

GnomeFontFace*
gpdf_font_face_download (const guchar* family, const guchar* species, GnomeFontWeight weight, gboolean italic, const guchar* font_data, gsize length) {
	GPFontEntry* entry;
	gchar* temp_fname;
	GnomeFontFace* face;

	g_return_val_if_fail (family != NULL, NULL);
	g_return_val_if_fail (font_data != NULL, NULL);
	g_return_val_if_fail (length > 0, NULL);

	temp_fname = gff_write_temp_data_file (font_data, length);
	if (temp_fname == NULL) {
		g_warning ("Could not create temporary file for embedded font");
		return NULL;
	}

	entry = g_new0 (GPFontEntry, 1);
	entry->type = gff_font_entry_type_from_data (font_data, length);
	entry->file = temp_fname;
	entry->refcount = 1;
	entry->face = NULL;
	entry->speciesname = g_strdup (species
				       ? species
				       : (const guchar*) "Regular");
	entry->Weight = weight;
	entry->familyname = g_strdup (family);
	entry->name = g_strconcat (family, " ", entry->speciesname, NULL);
	entry->italic_angle = italic ? -15 : 0;
	entry->is_alias = FALSE;

	gff_entry_fill_in_face (entry);
	face = entry->face;

	GPDF_FONT_FACE (face)->is_downloaded = TRUE;
	GPDF_FONT_FACE (face)->font_data = g_memdup (font_data, length);
	GPDF_FONT_FACE (face)->data_length = length;

	g_object_weak_ref (G_OBJECT (face), gff_unlink_temp_file, temp_fname);

	return face;
}

#endif
