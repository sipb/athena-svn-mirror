/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* PDF Info Dictionary helper functions
 *
 * Copyright (C) 2003 Martin Kretzschmar
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
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
#include "config.h"
#include "pdf-info-dict-util.h"
#include "gpdf-g-switch.h"
#  include <glib/gi18n.h>
#include "gpdf-g-switch.h"

static gboolean
has_unicode_marker (GString *string)
{
	return ((string->getChar (0) & 0xff) == 0xfe &&
		(string->getChar (1) & 0xff) == 0xff);
}

gchar *
pdf_info_dict_get_string (Dict *info_dict, const gchar *key) {
	Object obj;
	GString *value;
	gchar *result;

	g_return_val_if_fail (info_dict != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);

	if (!info_dict->lookup ((gchar *)key, &obj)->isString ()) {
		obj.free ();
		return g_strdup (_("Unknown"));
	}

	value = obj.getString ();

	if (has_unicode_marker (value)) {
		result = g_convert (value->getCString () + 2,
				    value->getLength () - 2,
				    "UTF-8", "UTF-16BE", NULL, NULL, NULL);
	} else {
		result = g_strndup (value->getCString (), value->getLength ());
	}

	obj.free ();

	return result;
}

struct tm *
pdf_info_dict_get_date (Dict *info_dict, const gchar *key) {
	Object obj;
	gchar *date_string;
	int year, mon, day, hour, min, sec;
	int scanned_items;
	struct tm *time;

	g_return_val_if_fail (info_dict != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);

	if (!info_dict->lookup ((gchar *)key, &obj)->isString ()) {
		obj.free ();
		return NULL;
	}

	date_string = obj.getString ()->getCString ();
	if (date_string [0] == 'D' && date_string [1] == ':')
		date_string += 2;

	/* FIXME only year is mandatory; parse optional timezone offset */
	scanned_items = sscanf (date_string, "%4d%2d%2d%2d%2d%2d",
				&year, &mon, &day, &hour, &min, &sec);
	if (scanned_items != 6)
		return NULL;
	
	/* Workaround for y2k bug in Distiller 3, hoping that it won't
	 * be used after y2.2k */
	if (year < 1930 && strlen (date_string) > 14) {
		int century, years_since_1900;
		scanned_items = sscanf (date_string, "%2d%3d%2d%2d%2d%2d%2d",
					&century, &years_since_1900,
					&mon, &day, &hour, &min, &sec);
		if (scanned_items != 7)
			return NULL;

		year = century * 100 + years_since_1900;
	}

	time = g_new0 (struct tm, 1);
	
	time->tm_year = year - 1900;
	time->tm_mon = mon - 1;
	time->tm_mday = day;
	time->tm_hour = hour;
	time->tm_min = min;
	time->tm_sec = sec;
	time->tm_wday = -1;
	time->tm_yday = -1;
	time->tm_isdst = -1;
	/* FIXME process time zone on systems that support it */	

	/* compute tm_wday and tm_yday and check date */
	if (mktime (time) == (time_t)-1)
		return NULL;
			
	return time;
}

static void
pdf_info_dict_process_string_property (Dict *info_dict, GObject *target,
                                       const gchar *key, const gchar *prop)
{
	gchar *value;

	g_return_if_fail (info_dict != NULL);
	g_return_if_fail (key != NULL);

	value = pdf_info_dict_get_string (info_dict, key);
	g_object_set (target, prop, value, NULL);
	g_free (value);
}

static void
pdf_info_dict_process_date_property (Dict *info_dict, GObject *target,
                                     const gchar *key, const gchar *prop)
{
	struct tm *time_value;
	gchar buffer [128];
	gchar *string_value;

	g_return_if_fail (info_dict != NULL);
	g_return_if_fail (key != NULL);

	time_value = pdf_info_dict_get_date (info_dict, key);
	if (time_value == NULL)
		goto error_no_time_value;

	if (strftime (buffer, 128, "%c", time_value) == 0)
		goto error_strftime;

	string_value = g_locale_to_utf8 (buffer, -1, NULL, NULL, NULL);
	g_object_set (target, prop, string_value, NULL);

	g_free (string_value);
	g_free (time_value);
	return;

 error_strftime:
	g_free (time_value);
 error_no_time_value:
	pdf_info_dict_process_string_property (info_dict, target,
                                               key, prop);
}

void
pdf_doc_process_properties (PDFDoc *pdf_doc, GObject *target)
{
	Object info;
	gchar *string;

	pdf_doc->getDocInfo (&info);

	if (info.isDict ()) {
		pdf_info_dict_process_string_property (info.getDict (), target,
						       "Title", "title");
		pdf_info_dict_process_string_property (info.getDict (), target,
						       "Subject", "subject");
		pdf_info_dict_process_string_property (info.getDict (), target,
						       "Author", "author");
		pdf_info_dict_process_string_property (info.getDict (), target,
						       "Keywords", "keywords");
		pdf_info_dict_process_string_property (info.getDict (), target,
						       "Creator", "creator");
		pdf_info_dict_process_string_property (info.getDict (), target,
						       "Producer", "producer");
		pdf_info_dict_process_date_property (info.getDict (), target,
						     "CreationDate",
						     "created");
		pdf_info_dict_process_date_property (info.getDict (), target,
						     "ModDate", "modified");
	}
	info.free ();

	g_object_set (target,
		      "security",
		      pdf_doc->isEncrypted () ? _("Encrypted") : _("None"),
		      NULL);

	string = g_new (gchar, 8);
	g_ascii_formatd (string, 8, "%.1f", pdf_doc->getPDFVersion ());
	g_object_set (target, "pdfversion", string, NULL);
	g_free (string);

	string = g_strdup_printf ("%d", pdf_doc->getNumPages ());
	g_object_set (target, "numpages", string, NULL);
	g_free (string);

	g_object_set (target,
		      "optimized",
		      /* Yes/No will be displayed in the pdf properties dialog
			 in a table as "Optimized: No" */
		      pdf_doc->isLinearized () ? _("Yes") : _("No"),
		      NULL);
}
