/*
 * bonobo-ui-util.c: Bonobo UI utility functions
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#include "config.h"
#include <gnome.h>
#include <ctype.h>

#include <bonobo/bonobo-ui-xml.h>
#include <bonobo/bonobo-ui-util.h>

#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>

static void
write_byte (char *start, guint8 byte)
{
	int chunk;

	chunk = (byte >> 4) & 0xf;

	if (chunk < 10)
		*start++ = '0' + chunk;
	else
		*start++ = 'a' + chunk - 10;

	chunk = byte & 0xf;

	if (chunk < 10)
		*start = '0' + chunk;
	else
		*start = 'a' + chunk - 10;
}

static char *
write_four_bytes (char *pos, int value) 
{
	write_byte (pos, value >> 24);
	pos += 2;
	write_byte (pos, value >> 16);
	pos += 2;
	write_byte (pos, value >> 8);
	pos += 2;
	write_byte (pos, value);
	pos += 2;

	return pos;
}

static guint8
read_byte (const char *start)
{
	int chunk = 0;

	if (*start >= '0' &&
	    *start <= '9')
		chunk |= *start - '0';

	else if (*start >= 'a' &&
		 *start <= 'f')
		chunk |= *start - 'a' + 10;

	else if (*start >= 'A' &&
		 *start <= 'F')
		chunk |= *start - 'A' + 10;
	else
		g_warning ("Format error in stream '%c'", *start);

	chunk <<= 4;
	start++;

	if (*start >= '0' &&
	    *start <= '9')
		chunk |= *start - '0';

	else if (*start >= 'a' &&
		 *start <= 'f')
		chunk |= *start - 'a' + 10;

	else if (*start >= 'A' &&
		 *start <= 'F')
		chunk |= *start - 'A' + 10;
	else
		g_warning ("Format error in stream '%c'", *start);

	return chunk;
}

static const guint32
read_four_bytes (const char *pos)
{
	return ((read_byte (pos) << 24) |
		(read_byte (pos + 2) << 16) |
		(read_byte (pos + 4) << 8) |
		(read_byte (pos + 6)));
}

char *
bonobo_ui_util_pixbuf_to_xml (GdkPixbuf *pixbuf)
{
	char   *xml, *dst, *src;
	int 	size, width, height, row, row_stride, col, byte_width;
	gboolean has_alpha;
			
	g_return_val_if_fail (pixbuf != NULL, NULL);

	width  = gdk_pixbuf_get_width  (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
	byte_width = width * (3 + (has_alpha ? 1 : 0));

	size =  4 * 2 * 2 + /* width, height */
		1 + 1 +     /* alpha, terminator */ 
		height * byte_width * 2;
	
	xml = g_malloc (size);
	xml [size - 1] = '\0';

	dst = xml;

	write_four_bytes (dst, gdk_pixbuf_get_width  (pixbuf));
	dst+= 4 * 2;

	write_four_bytes (dst, gdk_pixbuf_get_height (pixbuf));
	dst+= 4 * 2;

	if (has_alpha)
		*dst = 'A';
	else
		*dst = 'N';
	dst++;

	/* Copy over bitmap information */	
	src        = gdk_pixbuf_get_pixels    (pixbuf);
	row_stride = gdk_pixbuf_get_rowstride (pixbuf);
			
	for (row = 0; row < height; row++) {

		for (col = 0; col < byte_width; col++) {
			write_byte (dst, src [col]);
			dst+= 2;
		}

		src += row_stride;
	}

	return xml;
}

GdkPixbuf *
bonobo_ui_util_xml_to_pixbuf (const char *xml)
{
	GdkPixbuf 	*pixbuf;
	int 		width, height, byte_width;
	int             length, row_stride, col, row;
	gboolean 	has_alpha;
	guint8         *dst;

	g_return_val_if_fail (xml != NULL, NULL);

	while (*xml && isspace ((unsigned char) (*xml)))
		xml++;

	length = strlen (xml);
	g_return_val_if_fail (length > 4 * 2 * 2 + 1, NULL);

	width = read_four_bytes (xml);
	xml += 4 * 2;
	height = read_four_bytes (xml);
	xml += 4 * 2;

	if (*xml == 'A')
		has_alpha = TRUE;
	else if (*xml == 'N')
		has_alpha = FALSE;
	else {
		g_warning ("Unknown type '%c'", *xml);
		return NULL;
	}
	xml++;

	byte_width = width * (3 + (has_alpha ? 1 : 0));
	g_return_val_if_fail (length >= (byte_width * height * 2 + 4 * 2 * 2 + 1), NULL);

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, has_alpha, 8, width, height);

	dst        = gdk_pixbuf_get_pixels    (pixbuf);
	row_stride = gdk_pixbuf_get_rowstride (pixbuf);
	
	for (row = 0; row < height; row++) {

		for (col = 0; col < byte_width; col++) {
			dst [col] = read_byte (xml);
			xml += 2;
		}

		dst += row_stride;
	}

	return pixbuf;
}

static GdkPixbuf *
convert_from_chromakey (GdkPixbuf *pixbuf)
{
	GdkPixbuf *new;
	unsigned char *src_pixels, *dst_pixels;
	unsigned char *src_row, *dst_row;
	int src_rowstride;
	int dst_rowstride;
	int width, height;
	int i, j;

	g_assert (! gdk_pixbuf_get_has_alpha (pixbuf));

	width  = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_width (pixbuf);

	new = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, width, height);

	src_pixels = gdk_pixbuf_get_pixels (pixbuf);
	src_rowstride = gdk_pixbuf_get_rowstride (pixbuf);

	dst_pixels = gdk_pixbuf_get_pixels (new);
	dst_rowstride = gdk_pixbuf_get_rowstride (new);

	src_row = src_pixels;
	dst_row = dst_pixels;
	for (i = 0; i < height; i++) {
		unsigned char *sp, *dp;

		sp = src_row;
		dp = dst_row;
		for (j = 0; j < width; j++) {
			dp[0] = sp[0];
			dp[1] = sp[1];
			dp[2] = sp[2];

			if (sp[0] == 0xff && sp[1] == 0x00 && sp[2] == 0xff)
				dp[3] = 0x00;
			else
				dp[3] = 0xff;

			sp += 3;
			dp += 4;
		}

		src_row += src_rowstride;
		dst_row += dst_rowstride;
	}

	return new;
}

static GdkPixbuf *
pixbuf_from_imlib (const GnomeStockPixmapEntry *entry)
{
	const GnomeStockPixmapEntryImlib *imlib_entry;
	const GnomeStockPixmapEntryImlibScaled *scaled_imlib_entry;
	GdkPixbuf *pixbuf;
	GdkPixbuf *alpha_pixbuf;
	GdkPixbuf *scaled_pixbuf;

	imlib_entry = (const GnomeStockPixmapEntryImlib *) entry;

	pixbuf = gdk_pixbuf_new_from_data (imlib_entry->rgb_data,
					   GDK_COLORSPACE_RGB,
					   FALSE,
					   8,
					   imlib_entry->width, imlib_entry->height,
					   imlib_entry->width * 3,
					   NULL, NULL);

	alpha_pixbuf = convert_from_chromakey (pixbuf);

	if (imlib_entry->type == GNOME_STOCK_PIXMAP_TYPE_IMLIB) {
		gdk_pixbuf_unref (pixbuf);
		return alpha_pixbuf;
	}

	g_assert (imlib_entry->type == GNOME_STOCK_PIXMAP_TYPE_IMLIB_SCALED);

	scaled_imlib_entry = (const GnomeStockPixmapEntryImlibScaled *) entry;

	if (scaled_imlib_entry->scaled_width == imlib_entry->width
	    && scaled_imlib_entry->scaled_height == imlib_entry->height) {
		gdk_pixbuf_unref (pixbuf);
		return alpha_pixbuf;
	}
	
	scaled_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
					scaled_imlib_entry->scaled_width,
					scaled_imlib_entry->scaled_height);

	gdk_pixbuf_scale (alpha_pixbuf, scaled_pixbuf,
			  0, 0,
			  scaled_imlib_entry->scaled_width,
			  scaled_imlib_entry->scaled_height,
			  0.0, 0.0,
			  (double) scaled_imlib_entry->scaled_width / (double) imlib_entry->width,
			  (double) scaled_imlib_entry->scaled_height / (double) imlib_entry->height,
			  GDK_INTERP_HYPER);

	gdk_pixbuf_unref (alpha_pixbuf);
	gdk_pixbuf_unref (pixbuf);

	return scaled_pixbuf;
}

static GdkPixbuf *
get_stock_pixbuf (const char *name)
{
	GnomeStockPixmapEntry *entry;
	GdkPixbuf *pixbuf;
	char *path;

	if (!name)
		return NULL;

	entry = gnome_stock_pixmap_checkfor (name, GNOME_STOCK_PIXMAP_REGULAR);
	if (entry == NULL)
		return NULL;

	switch (entry->type) {
        case GNOME_STOCK_PIXMAP_TYPE_DATA:
		pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) ((GnomeStockPixmapEntryData *) entry)->xpm_data);
		break;
        case GNOME_STOCK_PIXMAP_TYPE_FILE:
		path = gnome_pixmap_file (((GnomeStockPixmapEntryFile *) entry)->filename);
		pixbuf = gdk_pixbuf_new_from_file (path);
		free (path);
		break;
        case GNOME_STOCK_PIXMAP_TYPE_PATH:
		pixbuf = gdk_pixbuf_new_from_file (((const GnomeStockPixmapEntryPath *) entry)->pathname);
		break;
	case GNOME_STOCK_PIXMAP_TYPE_IMLIB:
	case GNOME_STOCK_PIXMAP_TYPE_IMLIB_SCALED:
		pixbuf = pixbuf_from_imlib (entry);
		break;
        case GNOME_STOCK_PIXMAP_TYPE_NONE:
	/* (Don't know how to handle these.)  */
        case GNOME_STOCK_PIXMAP_TYPE_WIDGET:
	case GNOME_STOCK_PIXMAP_TYPE_GPIXMAP:
	default:
		pixbuf = NULL;
	}

	return pixbuf;
}

static gchar *
find_pixmap_in_path (const gchar *filename)
{
	gchar *path, *file;

	if (filename [0] == '/')
		return g_strdup (filename);

	file = gnome_pixmap_file (filename);
	if (file)
		return file;

	path = g_strconcat (g_get_prgname (), "/", filename, NULL);
	file = gnome_pixmap_file (path);
	if (file) {
		g_free (path);
		return file;
	}
	g_free (path);

	path = g_getenv ("GNOME_PATH");
	if (path != NULL) {
		gchar **pathv;
		gint i;

		pathv = g_strsplit (path, ":", 0);
		for (i = 0; pathv[i] != NULL; i++) {
			gchar *s;

			s = g_strconcat (pathv[i], "/share/pixmaps/",
					 filename, NULL);
			if (g_file_exists (s)) {
				g_strfreev (pathv);
				return s;
			}
			g_free (s);

			s = g_strconcat (pathv[i], "/share/pixmaps/",
					 g_get_prgname (), "/",
					 filename, NULL);
			if (g_file_exists (s)) {
				g_strfreev (pathv);
				return s;
			}
			g_free (s);
		}
		g_strfreev (pathv);
	}

	return NULL;
}

GdkPixbuf *
bonobo_ui_util_xml_get_icon_pixbuf (BonoboUINode *node, gboolean prepend_menu)
{
	GdkPixbuf *icon_pixbuf = NULL;
	char      *type, *text;

	g_return_val_if_fail (node != NULL, NULL);

	if (!(type = bonobo_ui_node_get_attr (node, "pixtype")))
		return NULL;

	if (!(text = bonobo_ui_node_get_attr (node, "pixname"))) {
		bonobo_ui_node_free_string (type);
		return NULL;
	}

	if (!text)
		return NULL;

	if (!strcmp (type, "stock")) {

		if (prepend_menu) {
			char *fullname = g_strconcat ("Menu_", text, NULL);
			icon_pixbuf = get_stock_pixbuf (fullname);
			g_free (fullname);
		} else
			icon_pixbuf = get_stock_pixbuf (text);

	} else if (!strcmp (type, "filename")) {
		char *name = find_pixmap_in_path (text);

		if ((name == NULL) || !g_file_exists (name))
			g_warning ("Could not find GNOME pixmap file %s", text);
		else
			icon_pixbuf = gdk_pixbuf_new_from_file (name);

		g_free (name);
	} else if (!strcmp (type, "pixbuf")) {
		
		/* Get pointer to GdkPixbuf */
		icon_pixbuf = bonobo_ui_util_xml_to_pixbuf (text);

		g_return_val_if_fail (icon_pixbuf != NULL, NULL);
	} else
		g_warning ("Unknown icon_pixbuf type '%s'", type);

	bonobo_ui_node_free_string (text);
	bonobo_ui_node_free_string (type);

	return icon_pixbuf;
}

GtkWidget *
bonobo_ui_util_xml_get_icon_pixmap_widget (BonoboUINode *node, gboolean prepend_menu)
{
	GnomePixmap *gpixmap;
	GdkPixbuf *pixbuf;

	g_return_val_if_fail (node != NULL, NULL);

	pixbuf = bonobo_ui_util_xml_get_icon_pixbuf (node, prepend_menu);
	if (pixbuf == NULL)
		return NULL;
	
	gpixmap = gtk_type_new (gnome_pixmap_get_type ());

	/* Get GdkPixmap and mask */
	gdk_pixbuf_render_pixmap_and_mask (pixbuf, &gpixmap->pixmap, &gpixmap->mask, 128);
	gdk_pixbuf_unref (pixbuf);

	return GTK_WIDGET (gpixmap);
}

void
bonobo_ui_util_xml_set_pixbuf (BonoboUINode     *node,
				GdkPixbuf   *pixbuf)
{
	char *data;

	g_return_if_fail (node != NULL);
	g_return_if_fail (pixbuf != NULL);

	bonobo_ui_node_set_attr (node, "pixtype", "pixbuf");
	data = bonobo_ui_util_pixbuf_to_xml (pixbuf);
	bonobo_ui_node_set_attr (node, "pixname", data);
	g_free (data);
}

void
bonobo_ui_util_xml_set_pix_xpm (BonoboUINode     *node,
				const char **xpm)
{
	GdkPixbuf *pixbuf;

	g_return_if_fail (xpm != NULL);
	g_return_if_fail (node != NULL);

	pixbuf = gdk_pixbuf_new_from_xpm_data (xpm);

	bonobo_ui_util_xml_set_pixbuf (node, pixbuf);

	gdk_pixbuf_unref (pixbuf);
}
				     
void
bonobo_ui_util_xml_set_pix_stock (BonoboUINode *node,
				  const char   *name)
{
	g_return_if_fail (node != NULL);
	g_return_if_fail (name != NULL);

	bonobo_ui_node_set_attr (node, "pixtype", "stock");
	bonobo_ui_node_set_attr (node, "pixname", name);
}

void
bonobo_ui_util_xml_set_pix_fname (BonoboUINode *node,
				  const char   *name)
{
	g_return_if_fail (node != NULL);
	g_return_if_fail (name != NULL);
	
	bonobo_ui_node_set_attr (node, "pixtype", "filename");
	bonobo_ui_node_set_attr (node, "pixname", name);
}


static void
free_help_menu_entry (GtkWidget *widget, GnomeHelpMenuEntry *entry)
{
	g_free (entry->name);
	g_free (entry->path);
	g_free (entry);
}

static void
bonobo_help_display_cb (BonoboUIComponent *component,
			gpointer           user_data,
			const char        *cname)
{
	gnome_help_display (component, user_data);
}

/*
 * Cut and paste job so we can overcome gnome-libs brokenness.
 */
static char *
bonobo_help_file_find_file (const char *prefix, const char *app,
			    const char *path)
{
	GList *language_list;
	GString *buf;
	
	gchar *res= NULL;
	gchar *p, c = 0;
	
	language_list= gnome_i18n_get_language_list ("LC_MESSAGES");
	
	while (!res && language_list) {
		const gchar *lang;
		
		lang = language_list->data;
		
		buf = g_string_new (NULL);
		g_string_sprintf (buf, "%s/gnome/help/%s/%s/%s",
				  prefix, app, lang, path);
		res = g_strdup (buf->str);
		p = strrchr (res, '#');
		if (p) {
			c = *p;
			*p = '\0';
		}
		g_string_free (buf, TRUE);
		
		if (!g_file_exists (res)) {
			g_free (res);
			res = NULL;
		}

		if (c && res) {
			*p = c;
			c = 0;
		}
		
		language_list = language_list->next;
	}
	
	return res;
}

void
bonobo_ui_util_build_help_menu (BonoboUIComponent *listener,
				const char        *app_prefix,
				const char        *app_name,
				BonoboUINode      *parent)
{
	char buf [1024];
	char *topic_file;
	FILE *file;

	g_return_if_fail (parent != NULL);
	g_return_if_fail (app_name != NULL);
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (listener));

	/* Try to open help topics file */
	topic_file = gnome_help_file_find_file ((char *)app_name, "topic.dat");
	
	/* Do something sensible */
	if (!topic_file && app_prefix)
		topic_file = bonobo_help_file_find_file (
			app_prefix, app_name, "topic.dat");

	if (!topic_file || !(file = fopen (topic_file, "rt"))) {
		g_warning ("Could not open help topics file %s for app %s", 
				topic_file ? topic_file : "NULL", app_name);

		g_free (topic_file);
		return;
	}
	g_free (topic_file);
	
	/* Read in the help topics and create menu items for them */
	while (fgets (buf, sizeof (buf), file)) {
		unsigned char *s, *id;
		GnomeHelpMenuEntry *entry;
		BonoboUINode *node;

		/* Format of lines is "help_file_name whitespace* menu_title" */
		for (s = buf; *s && !isspace (*s); s++)
			;

		*s++ = '\0';

		for (; *s && isspace (*s); s++)
			;

		if (s [strlen (s) - 1] == '\n')
			s [strlen (s) - 1] = '\0';

		node = bonobo_ui_node_new ("menuitem");
		/* Try and make something unique */
		id = g_strdup_printf ("Help%s%s", app_name, buf);
		bonobo_ui_node_set_attr (node, "name", id);
		bonobo_ui_node_set_attr (node, "verb", id);
		
		{
			char *encoded = bonobo_ui_util_encode_str (s);
			bonobo_ui_node_set_attr (node, "label", encoded);
			g_free (encoded);
		}

		bonobo_ui_node_add_child (parent, node);

		/* Create help menu entry */
		entry = g_new (GnomeHelpMenuEntry, 1);
		entry->name = g_strdup (app_name);
		entry->path = g_strdup (buf);

		bonobo_ui_component_add_verb (listener, id,
					      bonobo_help_display_cb, entry);

		gtk_signal_connect (GTK_OBJECT (listener), "destroy",
				    (GtkSignalFunc) free_help_menu_entry, 
				    entry);
		g_free (id);
	}

	fclose (file);
}

BonoboUINode *
bonobo_ui_util_build_accel (guint           accelerator_key,
			    GdkModifierType accelerator_mods,
			    const char     *verb)
{
	char    *name;
	BonoboUINode *ret;

	name = bonobo_ui_util_accel_name (accelerator_key, accelerator_mods);
	ret = bonobo_ui_node_new ("accel");
	bonobo_ui_node_set_attr (ret, "name", name);
	g_free (name);
	bonobo_ui_node_set_attr (ret, "verb", verb);

	return ret;

	/* Old Kludge due to brokenness in gnome-xml */
/*	char    *name;
	xmlDoc  *doc;
	BonoboUINode *ret;

	name = bonobo_ui_util_accel_name (accelerator_key, accelerator_mods);
	
	doc = xmlNewDoc ("1.0");
	ret = xmlNewDocNode (doc, NULL, "accel", NULL);
	bonobo_ui_node_set_attr (ret, "name", name);
	g_free (name);
	bonobo_ui_node_set_attr (ret, "verb", verb);
	doc->root = NULL;
	bonobo_ui_xml_strip (ret);
	bonobo_ui_node_free_stringDoc (doc);

	return ret;*/
}

BonoboUINode *
bonobo_ui_util_new_menu (gboolean    submenu,
			 const char *name,
			 const char *label,
			 const char *tip,
			 const char *verb)
{
	BonoboUINode *node;

	g_return_val_if_fail (name != NULL, NULL);

	if (submenu)
		node = bonobo_ui_node_new ("submenu");
	else
		node = bonobo_ui_node_new ("menuitem");

	bonobo_ui_node_set_attr (node, "name", name);
	if (label) {
		char *encoded = bonobo_ui_util_encode_str (label);
		bonobo_ui_node_set_attr (node, "label", encoded);
		g_free (encoded);
	}

	if (tip) {
		char *encoded = bonobo_ui_util_encode_str (tip);
		bonobo_ui_node_set_attr (node, "tip", encoded);
		g_free (encoded);
	}

	if (verb)
		bonobo_ui_node_set_attr (node, "verb", verb);

	return node;
}

BonoboUINode *
bonobo_ui_util_new_placeholder (const char *name,
				gboolean    top,
				gboolean    bottom)
{
	BonoboUINode *node;
	
	node = bonobo_ui_node_new ("placeholder");

	if (name)
		bonobo_ui_node_set_attr (node, "name", name);

	if (top && bottom)
		bonobo_ui_node_set_attr (node, "delimit", "both");
	else if (top)
		bonobo_ui_node_set_attr (node, "delimit", "top");
	else if (bottom)
		bonobo_ui_node_set_attr (node, "delimit", "bottom");

	return node;
}

void
bonobo_ui_util_set_radiogroup (BonoboUINode    *node,
			       const char *group_name)
{
	g_return_if_fail (node != NULL);
	g_return_if_fail (group_name != NULL);

	bonobo_ui_node_set_attr (node, "type", "radio");
	bonobo_ui_node_set_attr (node, "group", group_name);
}

void
bonobo_ui_util_set_toggle (BonoboUINode    *node,
			   const char *id,
			   const char *init_state)
{
	g_return_if_fail (node != NULL);

	bonobo_ui_node_set_attr (node, "type", "toggle");
	if (id)
		bonobo_ui_node_set_attr (node, "id", id);
	if (init_state)
		bonobo_ui_node_set_attr (node, "state", init_state);
}

BonoboUINode *
bonobo_ui_util_new_std_toolbar (const char *name,
				const char *label,
				const char *tip,
				const char *verb)
{
	BonoboUINode *node;

	g_return_val_if_fail (name != NULL, NULL);
	
	node = bonobo_ui_node_new ("toolitem");
	bonobo_ui_node_set_attr (node, "name", name);
	
	if (label) {
		char *encoded = bonobo_ui_util_encode_str (label);
		bonobo_ui_node_set_attr (node, "label", encoded);
		g_free (encoded);
	}
	if (tip) {
		char *encoded = bonobo_ui_util_encode_str (tip);
		bonobo_ui_node_set_attr (node, "tip", encoded);
		g_free (encoded);
	}
	if (verb)
		bonobo_ui_node_set_attr (node, "verb", verb);

	return node;
}
					     
BonoboUINode *
bonobo_ui_util_new_toggle_toolbar (const char *name,
				   const char *label,
				   const char *tip,
				   const char *id)
{
	BonoboUINode *node;

	g_return_val_if_fail (name != NULL, NULL);
	
	node = bonobo_ui_node_new ("toolitem");
	bonobo_ui_node_set_attr (node, "type", "toggle");
	bonobo_ui_node_set_attr (node, "name", name);
	
	if (label) {
		char *encoded = bonobo_ui_util_encode_str (label);
		bonobo_ui_node_set_attr (node, "label", encoded);
		g_free (encoded);
	}
	if (tip) {
		char *encoded = bonobo_ui_util_encode_str (tip);
		bonobo_ui_node_set_attr (node, "tip", encoded);
		g_free (encoded);
	}
	if (id)
		bonobo_ui_node_set_attr (node, "id", id);

	return node;
}
					     

/**
 * bonobo_ui_util_get_ui_fname:
 * @component_prefix: the prefix for the component.
 * @file_name: the file name of the xml file.
 * 
 * Builds a path to the xml file that stores the GUI.
 * 
 * Return value: the path to the file that describes the
 * UI or NULL if it is not found.
 **/
char *
bonobo_ui_util_get_ui_fname (const char *component_prefix,
			     const char *file_name)
{
	char *fname, *name;

#if 0
	/*
	 * This is fundamentally broken.  The user has no business defining
	 * his own user interfaces.
	 */
	/*
	 * The user copy ?
	 */
	fname = g_strdup_printf ("%s/.gnome/ui/%s",
				 g_get_home_dir (), file_name);

	/*
	 * FIXME: we should compare timestamps vs. the master copy.
	 */
	if (g_file_exists (fname))
		return fname;
	g_free (fname);
#endif
	
	/*
	 * The master copy
	 */
	if (component_prefix) {
		fname = g_strdup_printf ("%s/gnome/ui/%s",
					 component_prefix, file_name);
		if (g_file_exists (fname))
			return fname;
		g_free (fname);
	}

	name = g_strconcat (BONOBO_UIDIR, file_name, NULL);
	if (g_file_exists (name))
		return name;
	g_free (name);
	
	return NULL;
}


/* To avoid exporting property iterators on BonoboUINode
 * (not sure those should be public), this hack is used.
 */
#define XML_NODE(x) ((xmlNode*)(x))

/**
 * bonobo_ui_util_translate_ui:
 * @node: the node to start at.
 * 
 *  Quest through a tree looking for translatable properties
 * ( those prefixed with an '_' ). Translates the value of the
 * property and removes the leading '_'.
 **/
void
bonobo_ui_util_translate_ui (BonoboUINode *bnode)
{
        BonoboUINode *l;
        xmlNode *node;
	xmlAttr *prop, *old_props;

	if (!bnode)
		return;

	bonobo_ui_xml_strip (&bnode);
	if (!bnode) {
		g_warning ("All xml stripped away");
		return;
	}

	node = XML_NODE (bnode);

	old_props = node->properties;
	node->properties = NULL;

	for (prop = old_props; prop; prop = prop->next) {
		xmlChar *value;

		value = xmlNodeListGetString (NULL, prop->val, 1);

		/* Find translatable properties */
		if (prop->name && prop->name [0] == '_') {
			char *encoded;
			encoded = bonobo_ui_util_encode_str (_(value));
			xmlNewProp (node, &prop->name [1], encoded);
			g_free (encoded);
		} else
			xmlNewProp (node, prop->name, value);

		if (value)
			bonobo_ui_node_free_string (value);
	}

	for (l = bonobo_ui_node_children (bnode); l; l = bonobo_ui_node_next (l))
		bonobo_ui_util_translate_ui (l);

	xmlFreePropList (old_props);
}

void
bonobo_ui_util_fixup_help (BonoboUIComponent *component,
			   BonoboUINode      *node,
			   const char        *app_prefix,
			   const char        *app_name)
{
	BonoboUINode *l;
	gboolean build_here = FALSE;

	if (!node)
		return;

	if (bonobo_ui_node_has_name (node, "placeholder")) {
		char *txt;

		if ((txt = bonobo_ui_node_get_attr (node, "name"))) {
			build_here = !strcmp (txt, "BuiltMenuItems");
			bonobo_ui_node_free_string (txt);
		}
	}

	if (build_here) {
		bonobo_ui_util_build_help_menu (
			component, app_prefix, app_name, node);
	}

	for (l = bonobo_ui_node_children (node); l; l = bonobo_ui_node_next (l))
		bonobo_ui_util_fixup_help (component, l, app_prefix, app_name);
}

void
bonobo_ui_util_fixup_icons (BonoboUINode *node)
{
	BonoboUINode *l;
	gboolean fixup_here = FALSE;
	char *txt;

	if (!node)
		return;

	if ((txt = bonobo_ui_node_get_attr (node, "pixtype"))) {
		fixup_here = !strcmp (txt, "filename");
		bonobo_ui_node_free_string (txt);
	}

	if (fixup_here &&
	    ((txt = bonobo_ui_node_get_attr (node, "pixname")))) {
		GdkPixbuf *pixbuf = NULL;

		if (g_path_is_absolute (txt))
			pixbuf = gdk_pixbuf_new_from_file (txt);
		else {
			gchar *name = find_pixmap_in_path (txt);

			if (name) {
				pixbuf = gdk_pixbuf_new_from_file (name);
				g_free (name);
			}
		}

		if (pixbuf) {
			gchar *xml = bonobo_ui_util_pixbuf_to_xml (pixbuf);

			bonobo_ui_node_set_attr (node, "pixtype", "pixbuf");
			bonobo_ui_node_set_attr (node, "pixname", xml);
			g_free (xml);
		}

		bonobo_ui_node_free_string (txt);
	}

	for (l = bonobo_ui_node_children (node); l; l = bonobo_ui_node_next (l))
		bonobo_ui_util_fixup_icons (l);
}


/**
 * bonobo_ui_util_new_ui:
 * @component: The component help callback should be on
 * @file_name: Filename of the UI file
 * @app_name: Application name ( for finding help )
 * 
 *  Loads an xml tree from a file, cleans the 
 * doc cruft from its nodes; and translates the nodes.
 * 
 * Return value: The translated tree ready to be merged.
 **/
BonoboUINode *
bonobo_ui_util_new_ui (BonoboUIComponent *component,
		       const char        *file_name,
		       const char        *app_prefix,
		       const char        *app_name)
{
	BonoboUINode *node;

	g_return_val_if_fail (app_name != NULL, NULL);
	g_return_val_if_fail (file_name != NULL, NULL);

        node = bonobo_ui_node_from_file (file_name);

	bonobo_ui_xml_strip (&node);

	bonobo_ui_util_translate_ui (node);

	bonobo_ui_util_fixup_help (component, node, app_prefix, app_name);

	bonobo_ui_util_fixup_icons (node);

	return node;
}

void
bonobo_ui_util_set_ui (BonoboUIComponent *component,
		       const char        *app_prefix,
		       const char        *file_name,
		       const char        *app_name)
{
	char *fname;
	BonoboUINode *ui;

	if (bonobo_ui_component_get_container (component) == CORBA_OBJECT_NIL) {
		g_warning ("Component must be associated with a container first "
			   "see bonobo_component_set_container");
		return;
	}
	
	fname = bonobo_ui_util_get_ui_fname (app_prefix, file_name);
	if (!fname) {
		g_warning ("Can't find '%s' to load ui from", file_name);
		return;
	}
	
	ui = bonobo_ui_util_new_ui (component, fname, app_prefix, app_name);
	
	if (ui)
		bonobo_ui_component_set_tree (
			component, "/", ui, NULL);
	
	g_free (fname);
	bonobo_ui_node_free (ui);
	/* FIXME: we could be caching the tree here */
}

/*
 * Evil code, cut and pasted from gtkaccelgroup.c
 * needs de-Soptimizing :-)
 */

#define DELIM_PRE '*'
#define DELIM_PRE_S "*"
#define DELIM_POST '*'
#define DELIM_POST_S "*"

static inline gboolean
is_alt (const gchar *string)
{
	return ((string[0] == DELIM_PRE) &&
		(string[1] == 'a' || string[1] == 'A') &&
		(string[2] == 'l' || string[2] == 'L') &&
		(string[3] == 't' || string[3] == 'T') &&
		(string[4] == DELIM_POST));
}

static inline gboolean
is_ctl (const gchar *string)
{
	return ((string[0] == DELIM_PRE) &&
		(string[1] == 'c' || string[1] == 'C') &&
		(string[2] == 't' || string[2] == 'T') &&
		(string[3] == 'l' || string[3] == 'L') &&
		(string[4] == DELIM_POST));
}

static inline gboolean
is_modx (const gchar *string)
{
	return ((string[0] == DELIM_PRE) &&
		(string[1] == 'm' || string[1] == 'M') &&
		(string[2] == 'o' || string[2] == 'O') &&
		(string[3] == 'd' || string[3] == 'D') &&
		(string[4] >= '1' && string[4] <= '5') &&
		(string[5] == DELIM_POST));
}

static inline gboolean
is_ctrl (const gchar *string)
{
	return ((string[0] == DELIM_PRE) &&
		(string[1] == 'c' || string[1] == 'C') &&
		(string[2] == 't' || string[2] == 'T') &&
		(string[3] == 'r' || string[3] == 'R') &&
		(string[4] == 'l' || string[4] == 'L') &&
		(string[5] == DELIM_POST));
}

static inline gboolean
is_shft (const gchar *string)
{
	return ((string[0] == DELIM_PRE) &&
		(string[1] == 's' || string[1] == 'S') &&
		(string[2] == 'h' || string[2] == 'H') &&
		(string[3] == 'f' || string[3] == 'F') &&
		(string[4] == 't' || string[4] == 'T') &&
		(string[5] == DELIM_POST));
}

static inline gboolean
is_shift (const gchar *string)
{
	return ((string[0] == DELIM_PRE) &&
		(string[1] == 's' || string[1] == 'S') &&
		(string[2] == 'h' || string[2] == 'H') &&
		(string[3] == 'i' || string[3] == 'I') &&
		(string[4] == 'f' || string[4] == 'F') &&
		(string[5] == 't' || string[5] == 'T') &&
		(string[6] == DELIM_POST));
}

static inline gboolean
is_control (const gchar *string)
{
	return ((string[0] == DELIM_PRE) &&
		(string[1] == 'c' || string[1] == 'C') &&
		(string[2] == 'o' || string[2] == 'O') &&
		(string[3] == 'n' || string[3] == 'N') &&
		(string[4] == 't' || string[4] == 'T') &&
		(string[5] == 'r' || string[5] == 'R') &&
		(string[6] == 'o' || string[6] == 'O') &&
		(string[7] == 'l' || string[7] == 'L') &&
		(string[8] == DELIM_POST));
}

static inline gboolean
is_release (const gchar *string)
{
	return ((string[0] == DELIM_PRE) &&
		(string[1] == 'r' || string[1] == 'R') &&
		(string[2] == 'e' || string[2] == 'E') &&
		(string[3] == 'l' || string[3] == 'L') &&
		(string[4] == 'e' || string[4] == 'E') &&
		(string[5] == 'a' || string[5] == 'A') &&
		(string[6] == 's' || string[6] == 'S') &&
		(string[7] == 'e' || string[7] == 'E') &&
		(string[8] == DELIM_POST));
}

void
bonobo_ui_util_accel_parse (char              *accelerator,
			    guint             *accelerator_key,
			    GdkModifierType   *accelerator_mods)
{
	guint keyval;
	GdkModifierType mods;
	gint len;

	g_return_if_fail (accelerator_key != NULL);
	*accelerator_key = 0;
	g_return_if_fail (accelerator_mods != NULL);
	*accelerator_mods = 0;
	g_return_if_fail (accelerator != NULL);
  
	if (accelerator_key)
		*accelerator_key = 0;
	if (accelerator_mods)
		*accelerator_mods = 0;
  
	keyval = 0;
	mods = 0;
	len = strlen (accelerator);
	while (len)
	{
		if (*accelerator == DELIM_PRE)
		{
			if (len >= 9 && is_release (accelerator))
			{
				accelerator += 9;
				len -= 9;
				mods |= GDK_RELEASE_MASK;
			}
			else if (len >= 9 && is_control (accelerator))
			{
				accelerator += 9;
				len -= 9;
				mods |= GDK_CONTROL_MASK;
			}
			else if (len >= 7 && is_shift (accelerator))
			{
				accelerator += 7;
				len -= 7;
				mods |= GDK_SHIFT_MASK;
			}
			else if (len >= 6 && is_shft (accelerator))
			{
				accelerator += 6;
				len -= 6;
				mods |= GDK_SHIFT_MASK;
			}
			else if (len >= 6 && is_ctrl (accelerator))
			{
				accelerator += 6;
				len -= 6;
				mods |= GDK_CONTROL_MASK;
			}
			else if (len >= 6 && is_modx (accelerator))
			{
				static const guint mod_vals[] = {
					GDK_MOD1_MASK, GDK_MOD2_MASK, GDK_MOD3_MASK,
					GDK_MOD4_MASK, GDK_MOD5_MASK
				};

				len -= 6;
				accelerator += 4;
				mods |= mod_vals[*accelerator - '1'];
				accelerator += 2;
			}
			else if (len >= 5 && is_ctl (accelerator))
			{
				accelerator += 5;
				len -= 5;
				mods |= GDK_CONTROL_MASK;
			}
			else if (len >= 5 && is_alt (accelerator))
			{
				accelerator += 5;
				len -= 5;
				mods |= GDK_MOD1_MASK;
			}
			else
			{
				gchar last_ch;
	      
				last_ch = *accelerator;
				while (last_ch && last_ch != DELIM_POST)
				{
					last_ch = *accelerator;
					accelerator += 1;
					len -= 1;
				}
			}
		}
		else
		{
			keyval = gdk_keyval_from_name (accelerator);
			accelerator += len;
			len -= len;
		}
	}
  
	if (accelerator_key)
		*accelerator_key = gdk_keyval_to_lower (keyval);
	if (accelerator_mods)
		*accelerator_mods = mods;
}

gchar *
bonobo_ui_util_accel_name (guint              accelerator_key,
			   GdkModifierType    accelerator_mods)
{
	static const gchar text_release[] = DELIM_PRE_S "Release" DELIM_POST_S;
	static const gchar text_shift[] = DELIM_PRE_S "Shift" DELIM_POST_S;
	static const gchar text_control[] = DELIM_PRE_S "Control" DELIM_POST_S;
	static const gchar text_mod1[] = DELIM_PRE_S "Alt" DELIM_POST_S;
	static const gchar text_mod2[] = DELIM_PRE_S "Mod2" DELIM_POST_S;
	static const gchar text_mod3[] = DELIM_PRE_S "Mod3" DELIM_POST_S;
	static const gchar text_mod4[] = DELIM_PRE_S "Mod4" DELIM_POST_S;
	static const gchar text_mod5[] = DELIM_PRE_S "Mod5" DELIM_POST_S;
	guint l;
	gchar *keyval_name;
	gchar *accelerator;

	accelerator_mods &= GDK_MODIFIER_MASK;

	keyval_name = gdk_keyval_name (gdk_keyval_to_lower (accelerator_key));
	if (!keyval_name)
		keyval_name = "";

	l = 0;
	if (accelerator_mods & GDK_RELEASE_MASK)
		l += sizeof (text_release) - 1;
	if (accelerator_mods & GDK_SHIFT_MASK)
		l += sizeof (text_shift) - 1;
	if (accelerator_mods & GDK_CONTROL_MASK)
		l += sizeof (text_control) - 1;
	if (accelerator_mods & GDK_MOD1_MASK)
		l += sizeof (text_mod1) - 1;
	if (accelerator_mods & GDK_MOD2_MASK)
		l += sizeof (text_mod2) - 1;
	if (accelerator_mods & GDK_MOD3_MASK)
		l += sizeof (text_mod3) - 1;
	if (accelerator_mods & GDK_MOD4_MASK)
		l += sizeof (text_mod4) - 1;
	if (accelerator_mods & GDK_MOD5_MASK)
		l += sizeof (text_mod5) - 1;
	l += strlen (keyval_name);

	accelerator = g_new (gchar, l + 1);

	l = 0;
	accelerator[l] = 0;
	if (accelerator_mods & GDK_RELEASE_MASK)
	{
		strcpy (accelerator + l, text_release);
		l += sizeof (text_release) - 1;
	}
	if (accelerator_mods & GDK_SHIFT_MASK)
	{
		strcpy (accelerator + l, text_shift);
		l += sizeof (text_shift) - 1;
	}
	if (accelerator_mods & GDK_CONTROL_MASK)
	{
		strcpy (accelerator + l, text_control);
		l += sizeof (text_control) - 1;
	}
	if (accelerator_mods & GDK_MOD1_MASK)
	{
		strcpy (accelerator + l, text_mod1);
		l += sizeof (text_mod1) - 1;
	}
	if (accelerator_mods & GDK_MOD2_MASK)
	{
		strcpy (accelerator + l, text_mod2);
		l += sizeof (text_mod2) - 1;
	}
	if (accelerator_mods & GDK_MOD3_MASK)
	{
		strcpy (accelerator + l, text_mod3);
		l += sizeof (text_mod3) - 1;
	}
	if (accelerator_mods & GDK_MOD4_MASK)
	{
		strcpy (accelerator + l, text_mod4);
		l += sizeof (text_mod4) - 1;
	}
	if (accelerator_mods & GDK_MOD5_MASK)
	{
		strcpy (accelerator + l, text_mod5);
		l += sizeof (text_mod5) - 1;
	}
	strcpy (accelerator + l, keyval_name);

	return accelerator;
}

void
bonobo_ui_util_set_pixbuf (BonoboUIComponent *component,
			   const char        *path,
			   GdkPixbuf         *pixbuf)
{
	char *parent_path;
	BonoboUINode *node;

	node = bonobo_ui_component_get_tree (component, path, FALSE, NULL);

	g_return_if_fail (node != NULL);

	bonobo_ui_util_xml_set_pixbuf (node, pixbuf);

	parent_path = bonobo_ui_xml_get_parent_path (path);
	bonobo_ui_component_set_tree (component, parent_path, node, NULL);

	bonobo_ui_node_free (node);

	g_free (parent_path);
}

/*
 *   Evil routines; see invocation above for rational,
 * use a subset of utf-8 encoding to save extra library
 * dependencies and yet more complexity.
 */
#ifdef IF_ONLY_LIBXML1_COULD_COPE_WITH_UTF8

char *
bonobo_ui_util_encode_str (const char *str)
{
	const guint8 *a;
	guint8       *b, *ret;
	int           encoded_len;

	if (!str)
		return NULL;

	return g_strdup (str);

	encoded_len = 0;
	for (a = str; *a; a++)
		encoded_len += *a > 0x7f ? 2 : 1;

	ret = g_malloc (encoded_len + 1);

	b = ret;
	for (a = str; *a; a++) {
		if (*a > 0x7f) {
			*b++ = 0xc0 | ((*a) >> 6);
			*b++ = 0x80 | ((*a) & 0x3f);
		} else
			*b++ = *a;
	}
	*b = '\0';

	return ret;
}

char *
bonobo_ui_util_decode_str (const char *str, gboolean *err)
{
	const guint8 *a;
	guint8       *b, *ret;

	g_return_val_if_fail (err != NULL, NULL);
	*err = FALSE;

	if (!str)
		return NULL;

	return g_strdup (str);

	ret = g_malloc (strlen (str) + 1);

	b = ret;
	for (a = str; *a; a++) {
		if (*a > 0x7f && *(a + 1) != '\0') {
			*b++ = (*a << 6) | (*(a + 1) & 0x3f);
			a++;
		} else
			*b++ = *a;
	}
	*b = '\0';

	return ret;
}

#else

/*
 *  And here we start to have a major headache.
 * Not only does libxml1 not support utf-8 encoding
 * correctly; [ mostly this is in xmlSetProp and
 * variants ] but it seems impossible to discover
 * whether a translated string is encoded as utf8 as
 * opposed to some other 8 bit encoding. Consequently
 * I add a simple hexification encoding. This has the
 * merits of being totaly libxml1 clean, avoiding any
 * utf problems, only doubling the size, and being
 * detectable later.
 */
char *
bonobo_ui_util_encode_str (const char *str)
{
	const char *a;
	char       *b, *ret;

	if (!str)
		return NULL;

	ret = g_malloc (strlen (str) * 2 + 1);

	b = ret;
	for (a = str; *a; a++) {
		write_byte (b, *a);
		b += 2;
	}
	*b = '\0';

	return ret;
}

char *
bonobo_ui_util_decode_str (const char *str, gboolean *err)
{
	const char *a;
	char       *b, *ret;
	int         encoded_len;

	g_return_val_if_fail (err != NULL, NULL);
	*err = FALSE;
		
	if (!str)
		return NULL;

	encoded_len = 0;
	for (a = str; *a; a++) {
		if (! ((*a >= '0' && *a <= '9') ||
		       (*a >= 'a' && *a <= 'f'))) {
			*err = TRUE;
			return NULL;
		}
		encoded_len++;
	}

	ret = g_malloc ((encoded_len + 1) / 2 + 1);

	b = ret;
	for (a = str; *a && *(a + 1); a += 2)
		*b++ = read_byte (a);
	*b = '\0';

	return ret;
}
#endif
