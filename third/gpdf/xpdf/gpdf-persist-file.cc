/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; c-indent-level: 8; tab-width: 8; -*- */
/* GPdf Bonobo PersistFile implementation
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

#include "gpdf-persist-file.h"
#include "gpdf-util.h"
#include "gpdf-marshal.h"
#include "gpdf-g-switch.h"
#  include <libbonobo.h>
#  include <libbonoboui.h>
#  include <libgnomevfs/gnome-vfs.h>
#  include <gtk/gtk.h>
#  include "gpdf-control-private.h"
#include "gpdf-g-switch.h"
#include "Object.h"
#include "ErrorCodes.h"
#include "PDFDoc.h"
#include "GnomeVFSStream.h"

BEGIN_EXTERN_C

#define GPDF_IS_NON_NULL_PERSIST_FILE(obj) \
	((obj) && GPDF_IS_PERSIST_FILE ((obj)))

struct _GPdfPersistFilePrivate {
	PDFDoc *pdf_doc;

	GnomeVFSHandle *vfs_handle;
	char *uri;

	BonoboControl *control;
};

#define PARENT_TYPE BONOBO_TYPE_PERSIST
BONOBO_CLASS_BOILERPLATE_FULL (GPdfPersistFile, gpdf_persist_file,
			       Bonobo_PersistFile,
			       BonoboPersist, PARENT_TYPE);

enum {
	LOADING_FINISHED_SIGNAL,
	LOADING_FAILED_SIGNAL,
	LAST_SIGNAL
};

static guint gpdf_persist_file_signals [LAST_SIGNAL];

PDFDoc *
gpdf_persist_file_get_pdf_doc (GPdfPersistFile *persist_file)
{
	g_return_val_if_fail (GPDF_IS_NON_NULL_PERSIST_FILE (persist_file),
			      NULL);
	return persist_file->priv->pdf_doc;
}

static void
gpdf_persist_file_unload (GPdfPersistFile *persist_file)
{
	GPdfPersistFilePrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_PERSIST_FILE (persist_file));

	priv = persist_file->priv;

	if (priv->pdf_doc) {
		delete priv->pdf_doc;
		priv->pdf_doc = NULL;
	}

	if (priv->vfs_handle) {
		gnome_vfs_close (priv->vfs_handle);
		priv->vfs_handle = NULL;
	}

	if (priv->uri) {
		g_free (priv->uri);
		priv->uri = NULL;
	}
}

static void
gpdf_persist_file_on_entry_activate (GtkEntry *entry,
				     gpointer  user_data)
{
    GtkWidget *dialog1 = GTK_WIDGET (user_data);

    gtk_widget_hide (dialog1);
    g_object_set_data (G_OBJECT (dialog1),
		       "passwd",
		       (gpointer)gtk_entry_get_text (GTK_ENTRY (entry)));
    gtk_main_quit();
}

static void
gpdf_persist_file_on_dialog_response_cb (GtkDialog *dialog1,
					 guint resp, gpointer user_data)
{
	BonoboControl *control = BONOBO_CONTROL (user_data);
	GtkWidget *entry1 = GTK_WIDGET (g_object_get_data (G_OBJECT (dialog1), "entry1"));

	switch (resp) {
	case GTK_RESPONSE_HELP:
		gpdf_control_private_display_help (control, "gpdf-password");
		break;
	case GTK_RESPONSE_OK:
		gtk_widget_hide (GTK_WIDGET (dialog1));
		g_object_set_data (G_OBJECT (dialog1),
				   "passwd",
				   (gpointer)gtk_entry_get_text (GTK_ENTRY (entry1)));
		gtk_main_quit();
		break;
	case GTK_RESPONSE_CANCEL:
		gtk_widget_hide (GTK_WIDGET (dialog1));
		g_object_set_data (G_OBJECT (dialog1), "passwd", (gpointer)"");
		gtk_main_quit();
		break;
	}
}

static GtkWidget *
gpdf_persist_file_create_password_dialog (GPdfPersistFile *pf,
					  const gchar *labeltext,
					  const gchar *tiptext)
{
    GtkWidget *dialog1;
    GtkWidget *dialog_vbox1;
    GtkWidget *hbox1;
    GtkWidget *image;
    GtkWidget *vbox1;
    GtkWidget *label;
    GtkWidget *entry1;
    GtkWidget *dialog_action_area1;
    GtkWidget *helpbutton1;
    GtkWidget *cancelbutton1;
    GtkWidget *okbutton1;
    GtkTooltips *tooltips;

    g_return_val_if_fail (GPDF_IS_PERSIST_FILE (pf), NULL);
    
    tooltips = gtk_tooltips_new ();
    
    dialog1 = gtk_widget_new (GTK_TYPE_DIALOG,
			      "title", "",
			      "border-width", 6,
			      "resizable", FALSE,
			      "has-separator", FALSE,
			      NULL);
    g_object_set_data (G_OBJECT (dialog1), "control", pf->priv->control);
    
    dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
    g_object_set (dialog_vbox1, "spacing", 12, NULL);
    gtk_widget_show (dialog_vbox1);
    
    hbox1 = gtk_widget_new (GTK_TYPE_HBOX,
			    "homogeneous", FALSE,
			    "spacing", 12,
			    "border-width", 6,
			    NULL);
    gtk_widget_show (hbox1);
    gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);

    image = gtk_widget_new (GTK_TYPE_IMAGE,
			    "stock", GTK_STOCK_DIALOG_AUTHENTICATION,
			    "icon-size", GTK_ICON_SIZE_DIALOG,
			    "yalign", 0.0,
			    NULL);
    gtk_widget_show (image);
    gtk_container_add (GTK_CONTAINER (hbox1), image);

    vbox1 = gtk_vbox_new (FALSE, 2);
    g_object_set (vbox1, "spacing", 12, NULL);
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (hbox1), vbox1);
    
    label = gtk_label_new (labeltext);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (vbox1), label, FALSE, FALSE, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    entry1 = gtk_entry_new ();
    g_object_set_data (G_OBJECT (dialog1), "entry1", entry1);
    gtk_widget_show (entry1);
    gtk_box_pack_start (GTK_BOX (vbox1), entry1, FALSE, FALSE, 0);
    gtk_entry_set_max_length (GTK_ENTRY (entry1), 40);
    gtk_entry_set_visibility (GTK_ENTRY (entry1), FALSE);
    gtk_entry_set_activates_default (GTK_ENTRY (entry1), TRUE);
    gtk_entry_set_width_chars (GTK_ENTRY (entry1), 40);
    gtk_tooltips_set_tip (tooltips, entry1, tiptext, NULL);
    
    dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
    gtk_widget_show (dialog_action_area1);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

    helpbutton1 = gtk_button_new_from_stock ("gtk-help");
    gtk_widget_set_name (helpbutton1, "helpbutton1");
    gtk_widget_show (helpbutton1);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), helpbutton1, GTK_RESPONSE_HELP);
    GTK_WIDGET_SET_FLAGS (helpbutton1, GTK_CAN_DEFAULT);

    cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
    gtk_widget_show (cancelbutton1);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), cancelbutton1, GTK_RESPONSE_CANCEL);
    GTK_WIDGET_SET_FLAGS (cancelbutton1, GTK_CAN_DEFAULT);
    
    okbutton1 = gtk_button_new_with_mnemonic (_("_Open Document"));
    gtk_widget_show (okbutton1);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), okbutton1, GTK_RESPONSE_OK);
    GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);
    
    g_signal_connect ((gpointer) entry1, "activate",
		      G_CALLBACK (gpdf_persist_file_on_entry_activate),
		      (gpointer)dialog1);
    g_signal_connect ((gpointer) dialog1, "response",
		      G_CALLBACK (gpdf_persist_file_on_dialog_response_cb),
		      (gpointer)pf->priv->control);
    return dialog1;
}

static gchar *
gpdf_persist_file_get_password (GPdfPersistFile *pf)
{
	BonoboControl *control;
	gchar *passwd;
	GtkWidget *dialog;
	
	g_return_val_if_fail (GPDF_IS_PERSIST_FILE (pf), NULL);
	
	control = BONOBO_CONTROL (pf->priv->control);

	dialog = gpdf_persist_file_create_password_dialog (
		pf, 
		_("Enter document password:"),
		_("This document is encrypted and this "
		  "operation requires the document's password"));
	
	bonobo_control_set_transient_for(control,
					 GTK_WINDOW (dialog),
					 NULL);
	gtk_widget_show (dialog);
	gtk_main();
	passwd = g_strdup ((const gchar *)g_object_get_data (G_OBJECT (dialog),
							     "passwd"));
	gtk_widget_destroy (dialog);
	return passwd;
}

static void
impl_bonobo_persist_file_load (PortableServer_Servant servant,
			       const CORBA_char *uri,
			       CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	GPdfPersistFile *pf = GPDF_PERSIST_FILE (object);
	GPdfPersistFilePrivate *priv;
	gchar *uri_scheme;
	GnomeVFSResult vfs_result;
	Object obj;
	BaseStream *pdf_stream;
	const gchar *error_msg;

	bonobo_return_if_fail (GPDF_IS_NON_NULL_PERSIST_FILE (pf), ev);

	priv = pf->priv;

	if (priv->pdf_doc || priv->vfs_handle || priv->uri)
		gpdf_persist_file_unload (pf);

	uri_scheme = gnome_vfs_get_uri_scheme (uri);
	if (uri_scheme) // already looks like a URI
		priv->uri = g_strdup (uri);
	else
		priv->uri = gnome_vfs_get_uri_from_local_path (uri);
	
	g_free (uri_scheme);

	vfs_result = gnome_vfs_open (
		&priv->vfs_handle, priv->uri,
		(GnomeVFSOpenMode) (GNOME_VFS_OPEN_READ |
				    GNOME_VFS_OPEN_RANDOM));
	if (vfs_result != GNOME_VFS_OK) {
		bonobo_exception_set (ev, ex_Bonobo_Persist_FileNotFound);	
		g_signal_emit (G_OBJECT (pf),
			       gpdf_persist_file_signals [LOADING_FAILED_SIGNAL],
			       0, gnome_vfs_result_to_string (vfs_result));
		return;
	}

	obj.initNull ();
	pdf_stream = new GnomeVFSStream (priv->vfs_handle, 0, gFalse, 0, &obj);
	/* FIXME if (pdf_stream == NULL) ? */
	priv->pdf_doc = new PDFDoc (pdf_stream);

	/* FIXME if (pdf_doc == NULL) ? */
	if (!priv->pdf_doc->isOk ()) {
		int err;
		err = priv->pdf_doc->getErrorCode ();

		if (err == errEncrypted) {
			gchar *pwd = gpdf_persist_file_get_password (pf);
			GString goo_pwd (pwd);
			g_free (pwd);

			/* delete priv->pdf_doc; */ /* FIXME causes SEGV */
			priv->pdf_doc = new PDFDoc (pdf_stream, &goo_pwd, &goo_pwd);
		}
	}
	if (priv->pdf_doc->isOk ()) {
		g_signal_emit (G_OBJECT (pf),
			       gpdf_persist_file_signals [LOADING_FINISHED_SIGNAL],
			       0);
		return;
	}

	switch (priv->pdf_doc->getErrorCode ()) {
	case errBadCatalog:
		/* translators: page catalog is a part of the PDF file */
		/* The last period (.) is missing on purpose */
		error_msg = _("The PDF file is damaged. Its page catalog could not be read");
		bonobo_exception_set (ev, ex_Bonobo_IOError);	
		break;
	case errDamaged:
		error_msg = _("The PDF file is damaged or it is not a PDF file. It could not be repaired");
		bonobo_exception_set (ev, ex_Bonobo_IOError);	
		break;
	case errEncrypted:
		error_msg = _("The PDF document is encrypted and you didn't enter the correct password");
		bonobo_exception_set (ev, ex_Bonobo_IOError);	
		break;
	case errNone:
	case errOpenFile:
	case errHighlightFile:
	default:
		error_msg = NULL;
		g_assert_not_reached ();
		break;
	}			

	g_signal_emit (G_OBJECT (pf),
		       gpdf_persist_file_signals [LOADING_FAILED_SIGNAL],
		       0, error_msg);
	
	gpdf_persist_file_unload (pf);
}

static void
impl_bonobo_persist_file_save (PortableServer_Servant servant,
			       const CORBA_char *uri,
			       CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_Bonobo_NotSupported, NULL);
};

static Bonobo_Persist_ContentTypeList *
impl_get_content_types (BonoboPersist *persist, CORBA_Environment *ev)
{
	return bonobo_persist_generate_content_types (1, "application/pdf");
}

static CORBA_char *
impl_bonobo_persist_file_getCurrentFile (PortableServer_Servant servant,
					 CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	GPdfPersistFile *pfile = GPDF_PERSIST_FILE (object);

	/* if our persist_file has a filename with any length, return it */
	if (pfile->priv->uri && strlen (pfile->priv->uri))
		return CORBA_string_dup ((CORBA_char*)pfile->priv->uri);
	else {
		/* otherwise, raise a `NoCurrentName' exception */
		Bonobo_PersistFile_NoCurrentName *exception;
		exception = Bonobo_PersistFile_NoCurrentName__alloc ();

		exception->extension = CORBA_string_dup ("");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_PersistFile_NoCurrentName,
				     exception);
		return NULL;
	}
}

const char *
gpdf_persist_file_get_current_uri (GPdfPersistFile *pfile)
{
	g_return_val_if_fail (GPDF_IS_NON_NULL_PERSIST_FILE (pfile), NULL);

	return pfile->priv->uri;
}

static void
gpdf_persist_file_destroy (BonoboObject *object)
{
	gpdf_persist_file_unload (GPDF_PERSIST_FILE (object));

	BONOBO_CALL_PARENT (BONOBO_OBJECT_CLASS, destroy, (object));
}

static void
gpdf_persist_file_finalize (GObject *object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GPDF_IS_PERSIST_FILE (object));

	g_free ((GPDF_PERSIST_FILE (object))->priv);

	BONOBO_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static void
gpdf_persist_file_class_init (GPdfPersistFileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	BonoboObjectClass *bonobo_object_class = BONOBO_OBJECT_CLASS (klass);
	BonoboPersistClass *persist_class = BONOBO_PERSIST_CLASS (klass);
	POA_Bonobo_PersistFile__epv *epv = &klass->epv;

	object_class->finalize = gpdf_persist_file_finalize;
	bonobo_object_class->destroy = gpdf_persist_file_destroy;
	persist_class->get_content_types = impl_get_content_types;

	epv->load = impl_bonobo_persist_file_load;
	epv->save = impl_bonobo_persist_file_save;
	epv->getCurrentFile = impl_bonobo_persist_file_getCurrentFile;

	gpdf_persist_file_signals [LOADING_FINISHED_SIGNAL] =
		g_signal_new ("loading_finished",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GPdfPersistFileClass,
					       loading_finished),
			      NULL, NULL,
			      gpdf_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	gpdf_persist_file_signals [LOADING_FAILED_SIGNAL] =
		g_signal_new ("loading_failed",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GPdfPersistFileClass,
					       loading_failed),
			      NULL, NULL,
			      gpdf_marshal_VOID__STRING,
			      G_TYPE_NONE, 1,
			      G_TYPE_STRING);
}

static void
gpdf_persist_file_instance_init (GPdfPersistFile *persist_file)
{
	persist_file->priv = g_new0 (GPdfPersistFilePrivate, 1);
}

GPdfPersistFile *
gpdf_persist_file_construct (GPdfPersistFile *persist_file,
			     const gchar *iid)
{
	return GPDF_PERSIST_FILE (
	     bonobo_persist_construct (BONOBO_PERSIST (persist_file), iid));
}

GPdfPersistFile *
gpdf_persist_file_new (const gchar *iid)
{
	GPdfPersistFile *persist_file;

	persist_file =
	     GPDF_PERSIST_FILE (g_object_new (GPDF_TYPE_PERSIST_FILE, NULL));

	return gpdf_persist_file_construct (persist_file, iid);
}

void
gpdf_persist_file_set_control (GPdfPersistFile *gpdf_persist_file,
			       BonoboControl *control)
{
	GPdfPersistFilePrivate *priv;

	g_return_if_fail (GPDF_IS_PERSIST_FILE (gpdf_persist_file));
	g_return_if_fail (BONOBO_IS_CONTROL (control));
  
	priv = gpdf_persist_file->priv;

	priv->control = control;
}

END_EXTERN_C
