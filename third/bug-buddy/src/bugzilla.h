/* bug-buddy bug submitting program
 *
 * Copyright (C) 2001 Jacob Berkman
 * Copyright 2001 Ximian, Inc.
 *
 * Author:  jacob berkman  <jacob@bug-buddy.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __BUGZILLA_H__
#define __BUGZILLA_H__

#include <libgnomevfs/gnome-vfs-uri.h>
#include <libxml/parser.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct _BugzillaBTS BugzillaBTS;

typedef void (*XMLFunc) (BugzillaBTS *bts, xmlDoc *doc);

typedef enum {
	BUGZILLA_SUBMIT_DEBIAN,
	BUGZILLA_SUBMIT_FREITAG
} BugzillaSubmitType;

typedef struct {
	char *system_path;
	char *cache_path;
	char *tmp_path;

	GnomeVFSURI *source_uri;
	GnomeVFSURI *dest_uri;

	XMLFunc xml_func;

	gboolean read_from_cache;
	gboolean download;
	gboolean done;
} BugzillaXMLFile;

struct _BugzillaBTS {
	char      *name;
	char      *subdir;
	char      *email;

	BugzillaXMLFile *products_xml;
	BugzillaXMLFile *config_xml;
	BugzillaXMLFile *mostfreq_xml;

	BugzillaSubmitType submit_type;
	char *icon;

	GdkPixbuf *pixbuf;

	char *severity_node;
	char *severity_item;
	char *severity_header;
	
	/* products.xml */
	GSList *products;

	/* config.xml */
	GSList    *severities;
	GSList    *opsys;

	/* mostfreq.xml */
	GSList *bugs;
};

typedef struct {
	BugzillaBTS *bts;
	char        *name;
	char        *description;
	GSList      *components;
} BugzillaProduct;

typedef struct {
	BugzillaProduct *product;
	char            *name;
	char            *description;
} BugzillaComponent;

typedef struct {
	char *id;
	char *product;
	char *component;
	char *desc;
	char *url;
	/* gboolean shown : 1; */
} BugzillaBug;

enum {
	PRODUCT_ICON,
	PRODUCT_NAME,
	PRODUCT_DESC,
	PRODUCT_DATA,

	PRODUCT_COLS
};

enum {
	COMPONENT_NAME,
	COMPONENT_DESC,
	COMPONENT_DATA,

	COMPONENT_COLS
};

enum {
	MOSTFREQ_PRODUCT,
	MOSTFREQ_COMPONENT,
	MOSTFREQ_ID,
	MOSTFREQ_URL,
	MOSTFREQ_DESC,
	MOSTFREQ_SHOWN,

	MOSTFREQ_COLS,
};

void load_bugzillas (void);
void load_bugzilla_xml (void);

void bugzilla_bts_add_products_to_clist (BugzillaBTS *bts);
void bugzilla_product_add_components_to_clist (BugzillaProduct *product);

gboolean bugzilla_add_mostfreq (BugzillaBTS *bts);

char *generate_email_text (gboolean include_headers);

gboolean start_bugzilla_download (void);

typedef enum {
	END_BUGZILLA_NOOP     = 0,
	END_BUGZILLA_CANCEL   = 1 << 0,
	END_BUGZILLA_HIDE_BOX = 1 << 1
} EndBugzillaFlags;
void end_bugzilla_download (EndBugzillaFlags flags);

#endif /* __BUGZILLA_H__ */
