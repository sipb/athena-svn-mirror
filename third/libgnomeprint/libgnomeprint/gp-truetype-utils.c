/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gp-truetype-utils.c: Utilities for true type fonts
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
 *
 *  Authors:
 *    Lauris Kaplinski <lauris@ariman.ee>
 *
 *  Copyright (C) 1999-2002 Ximian, Inc.
 */

#include <config.h>
#include <libgnomeprint/gp-truetype-utils.h>

#define GPTT_STRING_MAX 0xffff

#define GUINT32FROMPTR(p) ((*(guchar*)((p))<<24)|(*((guchar*)(p)+1)<<16)|(*((guchar*)(p)+2)<<8)|(*((guchar*)(p)+3)))
#define GUINT16FROMPTR(p) ((*(guchar*)((p))<<8)|(*((guchar*)(p)+1)))

static GSList *gp_tt_split_glyf (const guchar *buf, guint glyf, guint next, guint loca, guint head, guint maxp, GSList *slices);
static gint gp_tt_offset_compare (gconstpointer a, gconstpointer b);

GSList *
gp_tt_split_file (const guchar *buf, guint len)
{
	const guchar *b;
	GSList *tables;
	GSList *slices;
	guint stringsize;
	gint i;

	guint32 scaler_type;
	guint16 numTables;
	guint16 searchRange;
	guint16 entrySelector;
	guint16 rangeShift;

	guint32 glyf, loca, head, maxp;

	b = buf;
	tables = NULL;

	/* Font Directory */
	/* Offset Subtable */

	scaler_type   = GUINT32FROMPTR (b);
	numTables     = GUINT16FROMPTR (b + 4);
	searchRange   = GUINT16FROMPTR (b + 6);
	entrySelector = GUINT16FROMPTR (b + 8);
	rangeShift    = GUINT16FROMPTR (b + 10);

	if ((scaler_type != 0x74727565) && (scaler_type != 0x00010000))
		return NULL;

	/* Table Directory */
	b = buf + 12;
	glyf = 0;
	loca = 0;
	head = 0;
	maxp = 0;

	for (i = 0; i < numTables; i++) {
		guint32 tag;
		guint32 checkSum;
		guint32 offset;
		guint32 length;

		tag      = GUINT32FROMPTR (b);
		checkSum = GUINT32FROMPTR (b + 4);
		offset   = GUINT32FROMPTR (b + 8);
		length   = GUINT32FROMPTR (b + 12);

		if (tag == GUINT32FROMPTR ("glyf"))
			glyf = offset;
		if (tag == GUINT32FROMPTR ("loca"))
			loca = offset;
		if (tag == GUINT32FROMPTR ("head"))
			head = offset;
		if (tag == GUINT32FROMPTR ("maxp"))
			maxp = offset;

		tables = g_slist_prepend (tables, GUINT_TO_POINTER (offset));
		b += 16;
	}

	if (!glyf || !loca || !head || !maxp) {
		g_warning ("Required table is missing");
		g_slist_free (tables);
		return NULL;
	}

	/* Now we have offset list */
	/* Sort it */
	tables = g_slist_sort (tables, gp_tt_offset_compare);

	/* Prepend start of file */
	tables = g_slist_prepend (tables, GUINT_TO_POINTER (0));

	/* Step through file */
	slices = g_slist_prepend (NULL, GUINT_TO_POINTER (0));
	stringsize = 0;

	while (tables) {
		guint start, next, size;
		/* Beginning of block */
		start = GPOINTER_TO_UINT (tables->data);
		tables = g_slist_remove (tables, tables->data);
		/* Find the beginning of next block */
		next = tables ? GPOINTER_TO_UINT (tables->data) : len;
		size = next - start;
		/* start new string, if we do not fit into limit */
		if ((stringsize + size) >= GPTT_STRING_MAX) {
			slices = g_slist_prepend (slices, GUINT_TO_POINTER (start));
			stringsize = 0;
		}
		/* Test, whether current table fits at all */
		if (size >= GPTT_STRING_MAX) {
			if (start == glyf) {
				slices = gp_tt_split_glyf (buf, start, next, loca, head, maxp, slices);
				if (!slices) {
					g_warning ("Cannot split 'glyf' table");
					g_slist_free (tables);
					return NULL;
				}
			} else {
				g_warning ("Too big table in font");
				g_slist_free (tables);
				g_slist_free (slices);
				return NULL;
			}
		} else {
			stringsize += size;
		}
	}

	slices = g_slist_reverse (slices);

	return slices;
}

/* Start of 'glyf' is already prepended */

static GSList *
gp_tt_split_glyf (const guchar *buf, guint glyf, guint next, guint loca, guint head, guint maxp, GSList *slices)
{
	guint16 indexToLocFormat;
	guint16 numGlyphs;
	guint32 start;
	guint32 offset;
	gint i;

	indexToLocFormat = GUINT16FROMPTR (buf + head + 50);
	numGlyphs = GUINT16FROMPTR (buf + maxp + 4);

	start = glyf;

	switch (indexToLocFormat) {
	case 0:
		/* Short offsets */
		for (i = 0; i < numGlyphs; i++) {
			offset = glyf + GUINT16FROMPTR (buf + loca + 2 * i + 2) * 2;
			if ((offset - start) >= GPTT_STRING_MAX) {
				/* Write down current offset */
				offset = glyf + GUINT16FROMPTR (buf + loca + 2 * i) * 2;
				slices = g_slist_prepend (slices, GUINT_TO_POINTER (offset));
				start = offset;
			}
		}
		if ((next - start) >= GPTT_STRING_MAX) {
			/* This is almost impossible condition - long pad */
			offset = glyf + GUINT16FROMPTR (buf + loca + 2 * numGlyphs) * 2;
			slices = g_slist_prepend (slices, GUINT_TO_POINTER (offset));
			start = offset;
		}
		break;
	case 1:
		/* Long offsets */
		for (i = 0; i < numGlyphs; i++) {
			offset = glyf + GUINT32FROMPTR (buf + loca + 4 * i + 4);
			if ((offset - start) >= GPTT_STRING_MAX) {
				/* Write down current offset */
				offset = glyf + GUINT32FROMPTR (buf + loca + 4 * i);
				slices = g_slist_prepend (slices, GUINT_TO_POINTER (offset));
				start = offset;
			}
		}
		if ((next - start) >= GPTT_STRING_MAX) {
			/* This is almost impossible condition - long pad */
			offset = glyf + GUINT32FROMPTR (buf + loca + 4 * numGlyphs);
			slices = g_slist_prepend (slices, GUINT_TO_POINTER (offset));
			start = offset;
		}
		break;
	default:
		g_warning ("Illegal indexToLocFormat value %d", indexToLocFormat);
		g_slist_free (slices);
		return NULL;
		break;
	}

	if ((next - start) > GPTT_STRING_MAX) {
		g_warning ("Too big pad at the end of 'glyf' table");
		g_slist_free (slices);
		return NULL;
	} else if (start > next) {
		g_warning ("Size mismatch between 'loca' and table directory");
		g_slist_free (slices);
		return NULL;
	} else if (next > start) {
		slices = g_slist_prepend (slices, GUINT_TO_POINTER (next));
	}

	return slices;
}

static gint
gp_tt_offset_compare (gconstpointer a, gconstpointer b)
{
	if (GPOINTER_TO_UINT (a) < GPOINTER_TO_UINT (b))
		return -1;
	if (GPOINTER_TO_UINT (a) > GPOINTER_TO_UINT (b))
		return 1;
	return 0;
}


