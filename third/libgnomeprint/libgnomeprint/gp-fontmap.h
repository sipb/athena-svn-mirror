/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gp-fontmap.h: fontmap implementation
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
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2001 Ximian, Inc.
 *
 */

#ifndef __GP_FONTMAP_H__
#define __GP_FONTMAP_H__

#include <glib.h>

G_BEGIN_DECLS

#include <sys/types.h>
#include <libxml/tree.h>
#include <libgnomeprint/gnome-font-face.h>

typedef struct _GPFontMap GPFontMap;
typedef struct _GPFileEntry GPFileEntry;
typedef struct _GPFontEntry GPFontEntry;
typedef struct _GPFamilyEntry GPFamilyEntry;
typedef struct _GPFontEntryT1 GPFontEntryT1;
typedef struct _GPFontEntryT1Alias GPFontEntryT1Alias;
typedef struct _GPFontEntryTT GPFontEntryTT;
typedef struct _GPFontEntryAlias GPFontEntryAlias;
typedef struct _GPFontEntrySpecial GPFontEntrySpecial;

typedef enum {
	GP_FONT_ENTRY_UNKNOWN,
	GP_FONT_ENTRY_TYPE1, /* Just ordinary Type1 font */
	GP_FONT_ENTRY_TRUETYPE,
	GP_FONT_ENTRY_TYPE1_ALIAS, /* Type1 font with foreign afm */
	GP_FONT_ENTRY_ALIAS, /* Full alias */
	GP_FONT_ENTRY_SPECIAL
} GPFontEntryType;

struct _GPFileEntry {
	gchar * name;
};

struct _GPFontMap {
	gint refcount;
	gint num_fonts;
	/* Timestamp of STATIC dir */
	time_t mtime_static;
	/* Timestamp of DYNAMIC dir */
	time_t mtime_dynamic;
	/* Timestamp of USER dir */
	time_t mtime_user;

	/* Name -> FontEntry */
	GHashTable *fontdict;
	/* Family name -> FamilyEntry */
	GHashTable *familydict;
	/* List of FontEntries, sorted A-Z */
	GSList *fonts;
	/* List of FamilyEntries, sorted A-Z */
	GSList *families;
	/* List of font names (pointing to entry->name) */
	GList *fontlist;
	/* List of family names (pointing to entry->name */
	GList *familylist;

	/* List of default entries (only for parsing, will be cleaned later) */
	GSList *defaults;
	/* Dictionary of defaults by language_COUNTRY code */
	GHashTable *defaultsdict;
};

struct _GPFontEntry {
	GPFontEntryType type;
	gint refcount;
	/* Our face */
	GnomeFontFace * face;
	/* Common fields */
	gchar *name;
	gchar *version;
	gchar *familyname;
	gchar *speciesname;
	gchar *psname;
	/* fixme: fixme: fixme: */
	gchar *weight;
	/* Some parsed afm latin metrics */
	GnomeFontWeight Weight;
	gdouble ItalicAngle; /* italic < 0 */
};

struct _GPFamilyEntry {
	gint refcount;
	gchar * name;
	/* List of FontEntries */
	GSList * fonts;
};

struct _GPFontEntryT1 {
	GPFontEntry entry;
	GPFileEntry afm;
	GPFileEntry pfb;
};

struct _GPFontEntryT1Alias {
	GPFontEntryT1 t1;
	gchar * alias;
};

struct _GPFontEntryTT {
	GPFontEntry entry;
	GPFileEntry ttf;
	gint facenum;
};

struct _GPFontEntryAlias {
	GPFontEntry entry;
	GPFontEntry * ref;
};

struct _GPFontEntrySpecial {
	GPFontEntry entry;
	GPFileEntry file;
	gint subface;
	GSList *additional;
};

GPFontMap * gp_fontmap_get (void);
void gp_fontmap_release (GPFontMap *);

void gp_font_entry_ref (GPFontEntry * entry);
void gp_font_entry_unref (GPFontEntry * entry);

/* This is experimental method (not public anyways) (Lauris) */

GPFontEntry * gp_font_entry_from_files (GPFontMap *map,
					const guchar *name, const guchar *family, const guchar *species, gboolean hidden,
					const guchar *filename, gint face, const GSList *additional);

GnomeFontWeight gp_fontmap_lookup_weight (const gchar * weight);

G_END_DECLS

#endif /* __GP_FONTMAP_H__ */
