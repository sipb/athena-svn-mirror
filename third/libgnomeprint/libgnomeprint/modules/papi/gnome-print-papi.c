/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-papi.c: A PAPI backend thingy
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useoful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Chema Celorio <chema@celorio.com>
 *    Dave Camp <dave@ximian.com>
 *    Danek Duvall <danek.duvall@sun.com>
 *
 *  Copyright 2002  Ximian, Inc. and authors
 *  Copyright 2004 Sun Microsystems, Inc.
 *
 */

#include <string.h>
#include <errno.h>

#include <config.h>
#include <glib.h>
#include <gmodule.h>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif
#include <papi.h>

#include <libgnomeprint/gnome-print-module.h>
#include <libgnomeprint/gpa/gpa-model.h>
#include <libgnomeprint/gpa/gpa-printer.h>
#include <libgnomeprint/gpa/gpa-option.h>
#include <libgnomeprint/gpa/gpa-settings.h>

#define d(x)
#define	GENERIC_PPD_FILE "file://localhost/usr/lib/lp/model/ppd/Generic/Generic-PostScript_Printer-Postscript.ppd.gz"

/* Argument order: id, name */

static xmlChar *model_xml_template = 
(xmlChar *)"<?xml version=\"1.0\"?>"
"<Model Id=\"%s\" Version=\"1.0\">"
"  <Name>%s</Name>"
"  <ModelVersion>0.0.1</ModelVersion>"
"  <Options>"
"    <Option Id=\"Transport\">"
"      <Option Id=\"Backend\" Type=\"List\" Default=\"PAPI\">"
"        <Item Id=\"PAPI\">"
"          <Name>PAPI</Name>"
"          <Key Id=\"Module\" Value=\"libgnomeprintpapi.so\"/>"
"        </Item>"
"      </Option>"
"    </Option>"
"    <Option Id=\"Output\">"
"      <Option Id=\"Media\">"
"        <Option Id=\"PhysicalOrientation\" Type=\"List\" Default=\"R0\">"
"          <Fill Ref=\"Globals.Media.PhysicalOrientation\"/>"
"        </Option>"
"        <Key Id=\"Margins\">"
"          <Key Id=\"Left\" Value=\"0\"/>"
"          <Key Id=\"Right\" Value=\"0\"/>"
"          <Key Id=\"Top\" Value=\"0\"/>"
"          <Key Id=\"Bottom\" Value=\"0\"/>"
"        </Key>"
"      </Option>"
"      <Option Id=\"Job\">"
"        <Option Id=\"NumCopies\" Type=\"String\" Default=\"1\"/>"
"        <Option Id=\"Collate\" Type=\"String\" Default=\"false\"/>"
"        <Option Id=\"PrintToFile\" Type=\"String\" Default=\"false\" Locked=\"true\"/>"
"        <Option Id=\"FileName\" Type=\"String\" Default=\"\"/>"
"      </Option>"
"    </Option>"
#if 0
"    <Option Id=\"Icon\">"
"      <Option Id=\"Filename\" Type=\"String\" Default=\"" DATADIR "/pixmaps/nautilus/default/i-printer.png\"/>"
"    </Option>"
#endif
"  </Options>"
"</Model>";

static GPANode *
load_paper_sizes (papi_attribute_t **attrs, GPANode *parent)
{
	GPANode *node;
	void *iterator = NULL;
	papi_attribute_t **value;
	char *text, *name;
	char *attribute = "PaperDimension";

	if (papiAttributeListGetCollection (attrs, &iterator, attribute, &value)
		!= PAPI_OK)
		return NULL;

	if (papiAttributeListGetString (value, NULL, "name", &name) != PAPI_OK)
		return NULL;

	node = gpa_option_list_new (parent, (guchar *)"PhysicalSize", (guchar *)name);
 	if (node == NULL)
		return NULL;

	iterator = NULL;
	for ( ;; ) {
		int x, y;
		papi_resolution_unit_t units;
		GPANode *size;
		char *height, *width;

		if (papiAttributeListGetCollection (
			attrs, &iterator, attribute, &value) != PAPI_OK)
			break;

		if (papiAttributeListGetString (value, NULL, "name", &name)
				!= PAPI_OK ||
			papiAttributeListGetString(value, NULL, "text", &text)
				!= PAPI_OK ||
			papiAttributeListGetResolution(value, NULL, "size", &x,
				&y, &units) != PAPI_OK)
			continue;

		size = gpa_option_item_new (node, (guchar *)name, (guchar *)text);

		width  = g_strdup_printf ("%d", x);
		height = g_strdup_printf ("%d", y);

		gpa_option_key_new (size, (guchar *)"Width", (guchar *)width);
		gpa_option_key_new (size, (guchar *)"Height", (guchar *)height);

		g_free (width);
		g_free (height);
	}

	gpa_node_reverse_children (node);

	return node;
}

static void
load_paper_sources (papi_attribute_t **attrs, GPANode *parent)
{
	GPANode *node;
	void *iterator = NULL;
	papi_attribute_t **value;
	char *text, *name;
	char *attribute = "InputSlot";

	if (papiAttributeListGetCollection (attrs, &iterator, attribute, &value)
		!= PAPI_OK)
		return;

	if (papiAttributeListGetString (value, NULL, "name", &name) != PAPI_OK)
		return;

	node = gpa_option_list_new (parent, (guchar *)"PaperSource", (guchar *)name);
	if (node == NULL)
		return;

	iterator = NULL;
	for ( ;; ) {
		if (papiAttributeListGetCollection (
			attrs, &iterator, attribute, &value) != PAPI_OK)
			break;

		if (papiAttributeListGetString (value, NULL, "name", &name)
				!= PAPI_OK ||
			papiAttributeListGetString(value, NULL, "text", &text)
				!= PAPI_OK)
			continue;

		/*
		 * Yes, "name" twice.  This is how the CUPS module does it, and
		 * it appears to be required.  It's not completely clear why.
		 */
		gpa_option_item_new (node, (guchar *)name, (guchar *)name);
	}
}

/*
 * "data" (input) consists of the line after the main attribute has been
 * stripped from the beginning.  The goal is to leave the option keyword in
 * "name" and the translation string in "text", with "data" pointing to the
 * actual attribute data (with leading whitespace and surrounding quotes
 * stripped).
 *
 * If either "name" or "text" are NULL, then just don't fill them in.  If "name"
 * is NULL, "text" is ignored.
 *
 * All processing is done in place, and the original string in "data" is
 * trounced on, and will point to a different location upon return.
 */
static void
parse_ppd_data (char **data, char **name, char **text)
{
	/* Save off the beginning */
	if (name != NULL)
		*name = *data;

	/* Work on the attribute data */
	*data = strchr (*data, ':');
	**data = '\0';
	++(*data);
	*data += strspn (*data, " \t");
	if ((*data)[0] == '\"' &&
		(*data)[strlen (*data) - 1] == '\"') {
		++(*data);
		(*data)[strlen (*data) - 1] = '\0';
	}

	/* Separate text from name */
	if (name != NULL && text != NULL) {
		*text = strchr (*name, '/');
		if (*text != NULL) {
			**text = '\0';
			++(*text);
		} else
			*text = *name;
	} else if (name != NULL && text == NULL) {
		char *s;

		s = strchr (*name, '/');
		if (s != NULL)
			*s = '\0';
	}
}

static void
add_ppd_attr (papi_attribute_t ***attrs, char *attr, char *data)
{
	if (strcmp(attr, "PaperDimension") == 0) {
		char *name, *text;
		int x, y;
		papi_attribute_t **dim = NULL;

		parse_ppd_data(&data, &name, &text);

		sscanf(data, "%d %d", &x, &y);

		papiAttributeListAddString (&dim, PAPI_ATTR_EXCL, "name", name);
		papiAttributeListAddString (&dim, PAPI_ATTR_EXCL, "text", text);
		papiAttributeListAddResolution (&dim, PAPI_ATTR_EXCL,
			"size", x, y, PAPI_RES_PER_INCH);
		papiAttributeListAddCollection (attrs, PAPI_ATTR_APPEND,
			attr, (const papi_attribute_t **)dim);
	} else if (strcmp (attr, "InputSlot") == 0 ||
		strcmp (attr, "ImageableArea") == 0 ||
		strcmp (attr, "PageRegion") == 0 ||
		strcmp (attr, "Resolution") == 0 ||
		strcmp (attr, "Duplex") == 0 ||
		strcmp (attr, "PageSize") == 0) {
		char *name, *text;
		papi_attribute_t **slot = NULL;

		parse_ppd_data(&data, &name, &text);

		papiAttributeListAddString (&slot, PAPI_ATTR_EXCL,
			"name", name);
		papiAttributeListAddString (&slot, PAPI_ATTR_EXCL,
			"text", text);
		papiAttributeListAddCollection (attrs, PAPI_ATTR_APPEND,
			attr, (const papi_attribute_t **)slot);
	} else {
		papiAttributeListAddString (attrs, PAPI_ATTR_EXCL, attr, data);
	}
}

typedef struct {
	char *name;
	char *text;
	char *type;
} ppd_ui_t;

typedef struct {
	int inui;
	int ingroup;
	int addattr;
	ppd_ui_t uidata;
	char *groupname;
} ppd_parse_state_t;

static void
parse_ppd_line (char *line, char **attr, char **data, ppd_parse_state_t *state)
{
	int namelen;

	state->addattr = 0;

	/* Comment or non-statement */
	if (line[0] != '*' || line[1] == '%')
		return;

	namelen = strcspn (line, ": ");
	/* XXX Is that the entire line? */

	if (strncmp (&line[1], "OpenUI", namelen - 1) == 0) {
		ppd_ui_t ui;

		state->inui = 1;

		ui.type = &line[namelen + 2];
		parse_ppd_data(&ui.type, &ui.name, &ui.text);
		ui.name = strdup(ui.name);
		ui.text = strdup(ui.text);
		ui.type = strdup(ui.type);

		state->uidata = ui;
	} else if (strncmp (&line[1], "CloseUI", namelen - 1) == 0) {
		state->inui = 0;
		free (state->uidata.name);
		free (state->uidata.text);
		free (state->uidata.type);
	} else if (strncmp (&line[1], "OpenGroup", namelen - 1) == 0) {
		state->ingroup = 1;
		state->groupname = strdup(&line[namelen + 1]);
	} else if (strncmp (&line[1], "CloseGroup", namelen - 1) == 0) {
		state->ingroup = 0;
		free (state->groupname);
	} else if (!state->inui /*&& !state->ingroup*/) { /* Simple attribute */
		line[namelen] = '\0';
		*attr = &line[1];

		*data = &line[namelen + 1];

		/* Strip leading whitespace */
		*data += strspn (*data, " \t");
		/* Strip enclosing quotes */
		if (*data[0] == '\"' &&
			(*data)[strlen (*data) - 1] == '\"') {
			++*data;
			(*data)[strlen (*data) - 1] = '\0';
		}
		state->addattr = 1;
	} else if (state->inui && strncmp (&line[1], state->uidata.name, namelen - 1) == 0) {
		line[namelen] = '\0';
		*attr = &line[1];
		*data = &line[namelen + 1];
		*data += strspn (*data, " \t");
		state->addattr = 1;
	}
}

static papi_status_t
get_ppd_attrs (papi_service_t service, const char *name,
	papi_attribute_t ***attrs)
{
	papi_printer_t printer = NULL;
	papi_status_t status, retval = PAPI_NOT_FOUND;
	const char *ppd_uri_attr[] = { "lpsched-printer-ppd-uri", NULL };
	static char generic_ppd[sizeof(GENERIC_PPD_FILE)] = GENERIC_PPD_FILE;
	papi_attribute_t **printer_attrs = NULL;
	char *ppd_uri = NULL;
	char buf[256];
#ifdef HAVE_ZLIB
	gzFile ppdf = NULL;
#else
	FILE *ppdf = NULL;
#undef gzopen
#undef gzgets
#undef gzclose
#define gzopen fopen
#define gzgets(fp, buf, len) fgets((buf), (len), (fp))
#define gzclose fclose
#endif
	ppd_parse_state_t state = { 0, 0, 0 };

	status = papiPrinterQuery (service, name, ppd_uri_attr, NULL, &printer);
	if (status != PAPI_OK || printer == NULL) {
		/*
		 * Not sure why papiPrinterQuery() could fail, but one reason
		 * seems to be that it doesn't have the requested attribute.
		 */
		d(printf ("getppdattrs: query failed (status = 0x%0x)\n", status));
	}

	if (status == PAPI_OK) {
		printer_attrs = papiPrinterGetAttributeList (printer);
		status = papiAttributeListGetString (printer_attrs, NULL,
			ppd_uri_attr[0], &ppd_uri);
	}
	if (status != PAPI_OK) {
		/*
		 * If the printer has no PPD file associated with it (at this
		 * time, that means almost all remote printers), we drop back to
		 * a default, generic PPD.
		 */
		ppd_uri = generic_ppd;
	}

	if (strncmp(ppd_uri, "file://", 7) != 0) {
		d(printf ("ppd_uri does not start with \"file://\": %s\n",
			ppd_uri));
		goto out;
	}
	/* Search for the first slash after file:// */
	ppd_uri = strchr(ppd_uri + 7, '/');

	if ((ppdf = gzopen (ppd_uri, "r")) == NULL) {
		d(printf ("couldn't open PPD file %s errno=%d\n", ppd_uri, errno));
		goto out;
	}

	/*
	 * Set the last-but-one byte in the buffer to the null byte.  If the
	 * line was longer than the buffer, then that null byte will have been
	 * written over with a different character.  If the line was exactly the
	 * right length, there'll be a newline there.  Otherwise, the null byte
	 * won't have been touched.
	 */
	buf[sizeof (buf) - 2] = '\0';
	while (gzgets (ppdf, buf, sizeof (buf)) != Z_NULL) {
		if (buf[sizeof (buf) - 2] == '\0' ||
			buf[sizeof (buf) - 2] == '\n') {
			/* Line was completely read. */
			char *attr, *data;

			*strchr(buf, '\n') = '\0';

			parse_ppd_line(buf, &attr, &data, &state);
			if (state.addattr)
				add_ppd_attr(attrs, attr, data);
		} else {
			/* Line is incomplete.  Just trash it. */
			buf[sizeof (buf) - 2] = '\0';
			while (gzgets(ppdf, buf, sizeof (buf)) != Z_NULL) {
				if (buf[sizeof (buf) - 2] == '\0' ||
					buf[sizeof (buf) - 2] == '\n')
					break;
				buf[sizeof (buf) - 2] = '\0';
			}
		}
		buf[sizeof (buf) - 2] = '\0';
	}

	retval = PAPI_OK;
out:
	if (ppdf != NULL)
		gzclose(ppdf);
	papiPrinterFree (printer);

	return retval;
}

static GPAModel *
get_model (const char *name, papi_service_t service)
{
	GPANode *media;
	GPANode *model;
	GPANode *output;
	char *xml;
	char *id;
	papi_attribute_t **attrs = NULL;
	papi_status_t status;
	char *nickname = NULL, *manufacturer = NULL;

	if ((status = get_ppd_attrs (service, name, &attrs)) != PAPI_OK)
		return NULL;

	(void) papiAttributeListGetString (attrs, NULL, "manufacturer", &manufacturer);
	(void) papiAttributeListGetString (attrs, NULL, "nickname", &nickname);

	if (manufacturer == NULL || nickname == NULL) {
		papiAttributeListFree(attrs);
		return NULL;
	}

	/*
	 * This could probably be removed.  The "Cups" prefix isn't really
	 * applicable here, except for compatibility with the CUPS module, and
	 * in practice the model file doesn't appear to exist anyway.
	 */
	id = g_strdup_printf ("Cups-%s-%s", manufacturer, nickname);
	model = gpa_model_get_by_id ((guchar *)id, TRUE);
	
	if (model == NULL) {
		xml = g_strdup_printf ((char *)model_xml_template, id, nickname);
		model = gpa_model_new_from_xml_str (xml);
		g_free (xml);

		output = gpa_node_lookup (model, (guchar *)"Options.Output");
		media  = gpa_node_lookup (model, (guchar *)"Options.Output.Media");

		(void) load_paper_sizes (attrs, media);
		load_paper_sources (attrs, output);

		gpa_node_unref (output);
		gpa_node_unref (media);
	}

	papiAttributeListFree(attrs);
	g_free (id);

	return GPA_MODEL (model);
}

static gboolean
append_printer (GPAList *printers_list, const char *name, gboolean is_default,
	papi_service_t service)
{
	GPANode *settings = NULL;
	GPANode *printer = NULL;
	GPAModel *model = NULL;
	gboolean retval = FALSE;

	model = get_model (name, service);
	if (model == NULL) {
		d(printf("model creation failed\n"));
		goto append_printer_exit;
	}

	/*
	 * Third arg is the id attribute in the <Settings> tag for
	 * <GnomePrintConfig>.  AFAIK, it can be an arbitrary (but
	 * preferably unique) string.
	 */
	settings = gpa_settings_new (model, (guchar *)"Default",
		(guchar *)"SetIdFromPAPI");

	if (settings == NULL) {
		d(printf("settings creation failed\n"));
		goto append_printer_exit;
	}

	printer = gpa_printer_new (name, name, model, GPA_SETTINGS (settings));
	if (printer == NULL) {
		d(printf("gpa_printer_new failed\n"));
		goto append_printer_exit;
	}

	if (gpa_node_verify (printer)) {
		gpa_list_prepend (printers_list, printer);
		if (is_default)
			gpa_list_set_default (printers_list, printer);
		retval = TRUE;
	} else {
		d(printf("gpa_node_verify failed\n"));
	}

 append_printer_exit:
	if (retval == FALSE) {
		g_warning ("The printer %s could not be created\n", name);

		my_gpa_node_unref (printer);
		my_gpa_node_unref (GPA_NODE (model));
		my_gpa_node_unref (settings);
	}

	return retval;
}

static int
sortprintersbyname(const void *p1, const void *p2)
{
	papi_printer_t a = *((papi_printer_t *) p1);
	papi_printer_t b = *((papi_printer_t *) p2);
	papi_attribute_t **a_attrs, **b_attrs;
	char *a_name = NULL, *b_name = NULL;

	a_attrs = papiPrinterGetAttributeList (a);
	b_attrs = papiPrinterGetAttributeList (b);

	papiAttributeListGetString (a_attrs, NULL, "printer-name", &a_name);
	papiAttributeListGetString (b_attrs, NULL, "printer-name", &b_name);

	if (a_name == NULL && b_name != NULL)
		return (1);
	else if (a_name != NULL && b_name == NULL)
		return (-1);
	else if (a_name == NULL && b_name == NULL)
		return (0);
	else
		return (strcmp(b_name, a_name));
}

/*
 * XXX: This model seems to be memory intensive.  You store a printer object,
 * complete with attributes (dunno how many, but looks like three or four at
 * least) for each printer that exists, which could be thousands, resulting in
 * hundreds of kilobytes (napkin estimate) of memory usage.  Should investigate.
 */

static void
gnome_print_papi_printer_list_append (gpointer printers_list)
{
	papi_status_t status;
	papi_service_t service = NULL;
	const char *attributes[] = { "printer-name", NULL };
	const char *_all_attr[] = { "all", NULL };
	const char *_default_attr[] = { "use", NULL };
	papi_printer_t *printers = NULL;
	papi_printer_t all_printer = NULL;
	papi_printer_t default_printer = NULL;
	char *def = NULL;
	int all_found = 0;

	g_return_if_fail (printers_list != NULL);
	g_return_if_fail (GPA_IS_LIST (printers_list));

	if ((status = papiServiceCreate (&service, NULL, NULL, NULL, NULL,
		PAPI_ENCRYPT_NEVER, NULL)) != PAPI_OK)
		return;

	if ((status = papiPrinterQuery (service, "_all", _all_attr, NULL,
		&all_printer)) == PAPI_OK) {
		char *all = NULL, *name;
#ifdef _REENTRANT
		char *lasts = NULL;
#endif
		int num = 0;
		papi_attribute_t **attrs;

		attrs = papiPrinterGetAttributeList (all_printer);
		if ((status = papiAttributeListGetString (attrs, NULL, "all",
			&all)) != PAPI_OK) {
			goto all_not_found;
		}

		if (strlen (all) == 0) {
			goto all_not_found;
		} else if (strchr (all, ',') == NULL) {
			num = 1;
			all_found = 1;
		} else {
			name = all;
			num = 1;
			while ((name = strchr (name, ',')) != NULL) {
				++name;
				if (name[0] != '\0')
					++num;
			}
			all_found = 1;
		}

		if ((printers = calloc (num + 1, sizeof (*printers))) == NULL) {
			papiPrinterFree (all_printer);
			papiServiceDestroy (service);
			return;
		}
		if ((all = strdup(all)) == NULL) {
			papiPrinterFree (all_printer);
			free (printers);
			papiServiceDestroy (service);
			return;
		}
		num = 0;
		for (
#ifdef _REENTRANT
			name = strtok_r (all, ",", &lasts);
#else
			name = strtok (all, ",");
#endif
			name != NULL;
#ifdef _REENTRANT
			name = strtok_r (NULL, ",", &lasts)
#else
			name = strtok (NULL, ",")
#endif
			) {
			if ((status = papiPrinterQuery (service, name,
				attributes, NULL, &printers[num])) != PAPI_OK)
				continue;
			++num;
		}
		free (all);
		if (num == 0)
			all_found = 0;
	}

all_not_found:
	papiPrinterFree (all_printer);
	if (all_found == 0) {
		int i;

		if ((status = papiPrintersList (service, attributes, NULL,
			&printers)) != PAPI_OK) {
			papiServiceDestroy (service);
			return;
		}

		for (i = 0; printers[i] != NULL; i++)
			;

		/*
		 * Only sort printers if they didn't come from the "_all"
		 * pseudo-printer; those might already be sorted the way the
		 * user wants them.
		 */
		qsort(printers, i, sizeof (papi_printer_t), sortprintersbyname);
	}

	/*
	 * Find the default printer.  First check for a _default printer, and
	 * see what that points to.  Failing that, check the environment.
	 */
	if ((status = papiPrinterQuery (service, "_default", _default_attr,
		NULL, &default_printer)) == PAPI_OK) {
		papi_attribute_t **attrs;

		attrs = papiPrinterGetAttributeList (default_printer);
		if ((status = papiAttributeListGetString (attrs, NULL, "use",
			&def)) == PAPI_OK) {

			def = strdup (def);
			papiPrinterFree (default_printer);

			if (def == NULL) {
				if (all_found == 0)
					papiPrinterListFree (printers);
				else {
					int i;
					for (i = 0; printers[i] != NULL; i++)
						papiPrinterFree (printers[i]);
					free (printers);
				}
				papiServiceDestroy (service);
				return;
			}
		} else
			papiPrinterFree (default_printer);
	}
	if (def == NULL &&
		(def = getenv ("PRINTER")) == NULL &&
		(def = getenv ("LPDEST")) == NULL)
		def = "lp";

	if (printers != NULL) {
		int i;

		for (i = 0; printers[i] != NULL; i++) {
			char *name = NULL, *uri = NULL;
			gboolean is_default;
			papi_attribute_t **attrs;
			papi_status_t status2;

			attrs = papiPrinterGetAttributeList (printers[i]);
			status = papiAttributeListGetString (attrs, NULL,
				"printer-name", &name);
			status2 = papiAttributeListGetString (attrs, NULL,
				"printer-uri", &uri);
			if (status == PAPI_OK && status2 == PAPI_OK) {
				is_default =
					(name != NULL && !strcmp (def, name));
				(void) append_printer (GPA_LIST (printers_list),
					name, is_default, service);
			}
		}
		if (all_found == 0)
			papiPrinterListFree (printers);
		else {
			for (i = 0; printers[i] != NULL; i++)
				papiPrinterFree (printers[i]);
			free (printers);
		}
	}

	papiServiceDestroy (service);

	return;
}


/*  ------------- GPA init ------------- */
G_MODULE_EXPORT gboolean gpa_module_init (GpaModuleInfo *info);

G_MODULE_EXPORT gboolean
gpa_module_init (GpaModuleInfo *info)
{
	info->printer_list_append = gnome_print_papi_printer_list_append;
	return TRUE;
}
