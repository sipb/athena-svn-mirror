/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Nautilus PDF Property Page
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
#include <time.h>
#include "gpdf-g-switch.h"
#  include <bonobo.h>
#  include <libgnome/gnome-macros.h>
#  include <libgnomevfs/gnome-vfs.h>
#include "gpdf-g-switch.h"
#include "nautilus-pdf-property-page.h"
#include "gpdf-util.h"
#include "GnomeVFSStream.h"
#include "Object.h"
#include "PDFDoc.h"
#include "pdf-info-dict-util.h"
#include "pdf-properties-display.h"

#define noPDF_DEBUG
#ifdef PDF_DEBUG
#  define DBG(x) x
#else
#  define DBG(x)
#endif

BEGIN_EXTERN_C

struct _GPdfNautilusPropertyPagePrivate {
        GPdfPropertiesDisplay *properties_display;
};

enum {
        PROP_URI,
};

#define PARENT_TYPE BONOBO_TYPE_CONTROL
static BonoboControlClass *parent_class;

static void
gpdf_nautilus_property_page_set_uri (GPdfNautilusPropertyPage *prop_page,
                                     const gchar *uri)
{
        GPdfNautilusPropertyPagePrivate *priv;
        GnomeVFSResult vfs_result;
        GnomeVFSHandle *handle;
	GnomeVFSStream *pdf_stream;
	PDFDoc *pdf_doc;
        Object obj;

        g_return_if_fail (GPDF_IS_NAUTILUS_PROPERTY_PAGE (prop_page));
        g_return_if_fail (uri != NULL);

        priv = prop_page->priv;

        DBG (g_message ("Loading PDF from URI %s", uri));

        vfs_result = gnome_vfs_open (
                &handle, uri,
                (GnomeVFSOpenMode)(GNOME_VFS_OPEN_READ | GNOME_VFS_OPEN_RANDOM));
        DBG (g_message ("gnome_vfs_open result: %s",
                        gnome_vfs_result_to_string (vfs_result)));
        g_assert (vfs_result == GNOME_VFS_OK); /* FIXME */

        obj.initNull ();
        pdf_stream = new GnomeVFSStream (handle, 0, gFalse, 0, &obj);
        pdf_doc = new PDFDoc (pdf_stream);
	g_assert (pdf_doc->isOk ()); /* FIXME */
	g_assert (pdf_doc->getCatalog () != NULL);

        DBG (g_message ("Done load"));

	pdf_doc_process_properties (pdf_doc,
				    G_OBJECT (priv->properties_display));

	delete pdf_doc; /* this deletes pdf_stream, too */
	gnome_vfs_close (handle);
}

static void
gpdf_nautilus_property_page_get_property (BonoboPropertyBag *bag,
                                          BonoboArg         *arg,
                                          guint              arg_id,
                                          CORBA_Environment *ev,
                                          gpointer           user_data)
{
        return;
}

static void
gpdf_nautilus_property_page_set_property (BonoboPropertyBag *bag,
                                          const BonoboArg   *arg,
                                          guint              arg_id,
                                          CORBA_Environment *ev,
                                          gpointer           user_data)
{
        GPdfNautilusPropertyPage *property_page;

        property_page = GPDF_NAUTILUS_PROPERTY_PAGE (user_data);

        if (arg_id == PROP_URI)
                gpdf_nautilus_property_page_set_uri (
                        property_page, BONOBO_ARG_GET_STRING (arg));
}

static void
gpdf_nautilus_property_page_init_control (GPdfNautilusPropertyPage *prop_page)
{
        GtkWidget *widget;

        widget = GTK_WIDGET (g_object_new (GPDF_TYPE_PROPERTIES_DISPLAY, NULL));
        gtk_widget_show (widget);
        bonobo_control_construct (BONOBO_CONTROL (prop_page), widget);

        prop_page->priv->properties_display = GPDF_PROPERTIES_DISPLAY (widget);
}

static void
gpdf_nautilus_property_page_init_pbag (GPdfNautilusPropertyPage *prop_page)
{
        BonoboPropertyBag *pb;

        pb = bonobo_property_bag_new (gpdf_nautilus_property_page_get_property,
                                      gpdf_nautilus_property_page_set_property,
                                      prop_page);
        bonobo_property_bag_add (pb, "URI", PROP_URI, BONOBO_ARG_STRING,
                                 NULL, _("URI currently displayed"),
                                 BONOBO_PROPERTY_WRITABLE);
        bonobo_control_set_properties (BONOBO_CONTROL (prop_page),
                                       BONOBO_OBJREF (pb), NULL);
        bonobo_object_release_unref (BONOBO_OBJREF (pb), NULL);
}

static void
gpdf_nautilus_property_page_init (GPdfNautilusPropertyPage *prop_page)
{
        prop_page->priv = g_new0 (GPdfNautilusPropertyPagePrivate, 1);
        gpdf_nautilus_property_page_init_control (prop_page);
        gpdf_nautilus_property_page_init_pbag (prop_page);
}

static void
gpdf_nautilus_property_page_finalize (GObject *object)
{
        GPdfNautilusPropertyPage *property_page;

        property_page = GPDF_NAUTILUS_PROPERTY_PAGE (object);

        if (property_page->priv) {
                g_free (property_page->priv);
                property_page->priv = NULL;
        }

        GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static void
gpdf_nautilus_property_page_class_init (GPdfNautilusPropertyPageClass *klass)
{
        G_OBJECT_CLASS (klass)->finalize = gpdf_nautilus_property_page_finalize;
}

BONOBO_TYPE_FUNC (GPdfNautilusPropertyPage, PARENT_TYPE,
                  gpdf_nautilus_property_page)

END_EXTERN_C
