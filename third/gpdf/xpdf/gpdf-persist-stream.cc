/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * gpdf bonobo persistor
 *
 * Authors:
 *   Michael Meeks (michael@ximian.com)
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * Copyright 1999, 2000 Ximian, Inc.
 * Copyright 2002 Martin Kretzschmar
 */

#include "gpdf-persist-stream.h"
#include "gpdf-util.h"
#include "gpdf-marshal.h"
#include "gpdf-g-switch.h"
#  include <libbonobo.h>
#  include <libbonoboui.h>
#  include <gtk/gtk.h>
#  include "gpdf-control-private.h"
#include "gpdf-g-switch.h"
#include "Object.h"
#include "PDFDoc.h"
#include "ErrorCodes.h"
#include "BonoboStream.h"

#define noPDF_DEBUG
#ifdef PDF_DEBUG
#  define DBG(x) x
#else
#  define DBG(x)
#endif

BEGIN_EXTERN_C

struct _GPdfPersistStreamPrivate {
	PDFDoc        *pdf_doc;

	Bonobo_Stream  bonobo_stream;

	BonoboControl *control;
};

#define PARENT_TYPE BONOBO_TYPE_PERSIST
BONOBO_CLASS_BOILERPLATE_FULL (GPdfPersistStream, gpdf_persist_stream,
			       Bonobo_PersistStream,
			       BonoboPersist, PARENT_TYPE);

enum {
	SET_PDF_SIGNAL,
	LAST_SIGNAL
};

static guint gpdf_persist_stream_signals [LAST_SIGNAL];

PDFDoc *
gpdf_persist_stream_get_pdf_doc (GPdfPersistStream *persist_stream)
{
	g_return_val_if_fail (persist_stream != NULL, NULL);
	g_return_val_if_fail (GPDF_IS_PERSIST_STREAM (persist_stream), NULL);

	return persist_stream->priv->pdf_doc;
}

static void
gpdf_persist_stream_delete_doc_and_stream (GPdfPersistStream *persist_stream)
{
	GPdfPersistStreamPrivate *priv;

	g_return_if_fail (persist_stream != NULL);
	g_return_if_fail (GPDF_IS_PERSIST_STREAM (persist_stream));

	priv = persist_stream->priv;

	if (priv->pdf_doc) {
		delete priv->pdf_doc;
		priv->pdf_doc = NULL;
	}

	if (priv->bonobo_stream) {
		bonobo_object_release_unref (priv->bonobo_stream, NULL);
		priv->bonobo_stream = NULL;
	}
}

static void
gpdf_persist_stream_on_entry_activate (GtkEntry *entry,
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
gpdf_persist_stream_on_dialog_response_cb (GtkDialog *dialog1,
					   guint resp, gpointer user_data)
{
	BonoboControl *control = BONOBO_CONTROL (user_data);
	GtkWidget *entry1 = GTK_WIDGET (g_object_get_data (G_OBJECT (dialog1), "entry1"));

	switch (resp) {
	case GTK_RESPONSE_HELP:
		gpdf_control_private_display_help (
			control, "gpdf-password");
		break;
	case GTK_RESPONSE_OK:
		gtk_widget_hide (GTK_WIDGET (dialog1));
		g_object_set_data (
			G_OBJECT (dialog1),
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
gpdf_persist_stream_create_password_dialog (GPdfPersistStream *ps,
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

    g_return_val_if_fail (GPDF_IS_PERSIST_STREAM (ps), NULL);
    
    tooltips = gtk_tooltips_new ();
    
    dialog1 = gtk_widget_new (GTK_TYPE_DIALOG,
			      "title", "",
			      "border-width", 6,
			      "resizable", FALSE,
			      "has-separator", FALSE,
			      NULL);
    g_object_set_data (G_OBJECT (dialog1), "control", ps->priv->control);
    
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
		      G_CALLBACK (gpdf_persist_stream_on_entry_activate),
		      (gpointer)dialog1);
    g_signal_connect ((gpointer) dialog1, "response",
		      G_CALLBACK (gpdf_persist_stream_on_dialog_response_cb),
		      (gpointer)ps->priv->control);
    return dialog1;
}

static gchar *
gpdf_persist_stream_get_password (GPdfPersistStream *ps)
{
	BonoboControl *control;
	gchar *passwd;
	GtkWidget *dialog;
	
	g_return_val_if_fail (GPDF_IS_PERSIST_STREAM (ps), NULL);
	
	control = BONOBO_CONTROL (ps->priv->control);

	dialog = gpdf_persist_stream_create_password_dialog (
                ps, 
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
impl_bonobo_persist_stream_load (PortableServer_Servant servant,
				 Bonobo_Stream stream, const CORBA_char *type,
				 CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	GPdfPersistStream *ps = GPDF_PERSIST_STREAM (object);
	GPdfPersistStreamPrivate *priv;
	Object obj;
	BaseStream *pdf_stream;

	bonobo_return_if_fail (ps != NULL, ev);
	bonobo_return_if_fail (GPDF_IS_PERSIST_STREAM (ps), ev);

	priv = ps->priv;

	if (priv->pdf_doc || priv->bonobo_stream)
		gpdf_persist_stream_delete_doc_and_stream (ps);

	/* FIXME: is this against Bonobo::PersistStream's contract ? */

	/* We need to keep the alive stream for later */

	CORBA_Object_duplicate (stream, ev);
	Bonobo_Unknown_ref (stream, ev);
	priv->bonobo_stream = stream;
	BONOBO_RET_EX (ev);

	DBG (g_message ("Loading PDF from PersistStream"));
	
	obj.initNull ();
	pdf_stream = new bonoboStream (stream, 0, gFalse, 0, &obj);
	priv->pdf_doc = new PDFDoc (pdf_stream);

	DBG (g_message ("Done load"));

	if (!priv->pdf_doc->isOk ()) {		
		int err;
		err = priv->pdf_doc->getErrorCode ();

		if (err == errEncrypted) {
			gchar *pwd = gpdf_persist_stream_get_password (ps);
			GString goo_pwd (pwd);
			g_free (pwd);
			
			/* delete priv->pdf_doc; */ /* FIXME causes SIGSEGV */
			priv->pdf_doc = new PDFDoc (pdf_stream, &goo_pwd, &goo_pwd);
		}
		
		if (!priv->pdf_doc->isOk ()) {
			g_warning ("Duff user pdf data");
			goto exit_bad_file;
		}
	}

	goto exit_clean;

exit_bad_file:
	gpdf_persist_stream_delete_doc_and_stream (ps);
	bonobo_exception_set(ev,ex_Bonobo_Stream_NoPermission);

exit_clean:
	g_signal_emit (G_OBJECT (ps),
		       gpdf_persist_stream_signals [SET_PDF_SIGNAL],
		       0);	
}

static void
impl_bonobo_persist_stream_save (PortableServer_Servant servant,
				 Bonobo_Stream stream, const CORBA_char *type,
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


static void
gpdf_persist_stream_destroy (BonoboObject *object)
{
	GPdfPersistStream *persist_stream = GPDF_PERSIST_STREAM (object);

	g_return_if_fail (object != NULL);
	g_return_if_fail (GPDF_IS_PERSIST_STREAM (object));
	
	DBG (g_message ("Destroying GPdfPersistStream"));

	gpdf_persist_stream_delete_doc_and_stream (persist_stream);

	BONOBO_CALL_PARENT (BONOBO_OBJECT_CLASS, destroy, (object));
}

static void
gpdf_persist_stream_finalize (GObject *object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GPDF_IS_PERSIST_STREAM (object));

	g_free ((GPDF_PERSIST_STREAM (object))->priv);

	BONOBO_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static void
gpdf_persist_stream_class_init (GPdfPersistStreamClass *klass)
{
 	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	BonoboObjectClass *bonobo_object_class = BONOBO_OBJECT_CLASS (klass);
	BonoboPersistClass *persist_class = BONOBO_PERSIST_CLASS (klass);
	POA_Bonobo_PersistStream__epv *epv = &klass->epv;

 	object_class->finalize = gpdf_persist_stream_finalize;
	bonobo_object_class->destroy = gpdf_persist_stream_destroy;
	persist_class->get_content_types = impl_get_content_types;

	epv->load = impl_bonobo_persist_stream_load;
	epv->save = impl_bonobo_persist_stream_save;

	gpdf_persist_stream_signals [SET_PDF_SIGNAL] =
		g_signal_new ("set_pdf",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GPdfPersistStreamClass,
					       set_pdf),
			      NULL, NULL,
			      gpdf_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
}

static void
gpdf_persist_stream_instance_init (GPdfPersistStream *persist_stream)
{
 	persist_stream->priv = g_new0 (GPdfPersistStreamPrivate, 1);
}

GPdfPersistStream *
gpdf_persist_stream_construct (GPdfPersistStream *persist_stream, 
			       const gchar *iid)
{
	return GPDF_PERSIST_STREAM (
	     bonobo_persist_construct (BONOBO_PERSIST (persist_stream), iid));
}

GPdfPersistStream *
gpdf_persist_stream_new (const gchar *iid)
{
	GPdfPersistStream *persist_stream;

	DBG (g_message ("Creating GPdfPersistStream"));

	persist_stream = 
	     GPDF_PERSIST_STREAM (g_object_new (GPDF_TYPE_PERSIST_STREAM, NULL));

	return gpdf_persist_stream_construct (persist_stream, iid);
}

void
gpdf_persist_stream_set_control (GPdfPersistStream *gpdf_persist_stream,
				 BonoboControl *control)
{
	GPdfPersistStreamPrivate *priv;

	g_return_if_fail (GPDF_IS_PERSIST_STREAM (gpdf_persist_stream));
	g_return_if_fail (BONOBO_IS_CONTROL (control));

	priv = gpdf_persist_stream->priv;

	priv->control = control;
}

END_EXTERN_C
