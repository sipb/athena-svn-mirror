/*
 * test-focus.c: A test application to sort focus issues.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2001 Ximian, Inc.
 */

#include "config.h"

#include <stdlib.h>
#include <libbonoboui.h>
#include <bonobo/bonobo-i18n.h>
#include <bonobo/bonobo-control-internal.h>

typedef struct {
	/* Control */
	GtkWidget          *control_widget;
	BonoboControl      *control;
	BonoboPlug         *plug;
	/* Frame */
	GtkWidget          *bonobo_widget;
	BonoboControlFrame *frame;
	BonoboSocket       *socket;
} Test;

typedef enum {
	DESTROY_CONTROL,
	DESTROY_TOPLEVEL,
	DESTROY_CONTAINED,
	DESTROY_SOCKET,
	DESTROY_TYPE_LAST
} DestroyType;

static void
destroy_test (Test *test, DestroyType type)
{
	switch (type) {
	case DESTROY_CONTAINED: {
		/* Highly non-useful, should never happen */
		BonoboControlFrame *frame;

		gtk_widget_destroy (test->control_widget);

		g_assert (test->control_widget == NULL);

		frame = bonobo_widget_get_control_frame (
			BONOBO_WIDGET (test->bonobo_widget));
		g_assert (BONOBO_IS_CONTROL_FRAME (frame));
		break;
	}

	case DESTROY_SOCKET:
		g_warning ("unimpl");
		/* drop through */

	case DESTROY_CONTROL:
		bonobo_object_unref (test->control);
		g_assert (test->plug == NULL);
		g_assert (test->control_widget == NULL);
		/* drop through */

	case DESTROY_TOPLEVEL:
		gtk_widget_destroy (test->bonobo_widget);
		g_assert (test->bonobo_widget == NULL);
		g_assert (test->socket == NULL);
		g_assert (test->frame == NULL);
		g_assert (test->plug == NULL);
		g_assert (test->control == NULL);
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	g_assert (test->control_widget == NULL);

	switch (type) {
	case DESTROY_CONTAINED:
		gtk_widget_destroy (test->bonobo_widget);
		break;
	default:
		break;
	}

	g_assert (test->bonobo_widget == NULL);
}

static void
destroy_cb (GObject *object, Test *text)
{
	dprintf ("destroy %s %p\n",
		 g_type_name_from_instance (
			 (GTypeInstance *) object), object);
}

static void
create_control (Test *test)
{
	test->control_widget = gtk_entry_new ();
	g_signal_connect (G_OBJECT (test->control_widget),
			  "destroy", G_CALLBACK (destroy_cb),
			  test);

	g_assert (test->control_widget != NULL);
	gtk_widget_show (test->control_widget);

	test->control = bonobo_control_new (test->control_widget);
	g_assert (test->control != NULL);

	test->plug = bonobo_control_get_plug (test->control);
	g_assert (test->plug != NULL);

	g_signal_connect (G_OBJECT (test->plug),
			  "destroy", G_CALLBACK (destroy_cb),
			  test);

	g_object_add_weak_pointer (G_OBJECT (test->plug),
				   (gpointer *) &test->plug);
	g_object_add_weak_pointer (G_OBJECT (test->control),
				   (gpointer *) &test->control);
	g_object_add_weak_pointer (G_OBJECT (test->control_widget),
				   (gpointer *) &test->control_widget);
}

/* An ugly hack into the ORB */
extern CORBA_Object ORBit_objref_get_proxy (CORBA_Object obj);

static void
create_frame (Test *test, gboolean fake_remote)
{
	Bonobo_Control control;

	control = BONOBO_OBJREF (test->control);
/*	if (fake_remote)
	control = ORBit_objref_get_proxy (control);*/

	test->bonobo_widget = bonobo_widget_new_control_from_objref (
		control, CORBA_OBJECT_NIL);
	bonobo_object_unref (BONOBO_OBJECT (test->control));
	gtk_widget_show (test->bonobo_widget);
	test->frame = bonobo_widget_get_control_frame (
		BONOBO_WIDGET (test->bonobo_widget));
	test->socket = bonobo_control_frame_get_socket (test->frame);

	g_object_add_weak_pointer (G_OBJECT (test->frame),
				   (gpointer *) &test->frame);
	g_object_add_weak_pointer (G_OBJECT (test->socket),
				   (gpointer *) &test->socket);
	g_object_add_weak_pointer (G_OBJECT (test->bonobo_widget),
				   (gpointer *) &test->bonobo_widget);
}

static Test *
create_test (gboolean fake_remote)
{
	Test *test = g_new0 (Test, 1);

	create_control (test);

	create_frame (test, fake_remote);

	return test;
}

static int
timeout_cb (gpointer user_data)
{
	gboolean *done = user_data;

	*done = TRUE;

	return FALSE;
}

static void
mainloop_for (gulong interval)
{
	gboolean mainloop_done = FALSE;

	if (!interval) /* Wait for another process */
		interval = 50;

	g_timeout_add (interval, timeout_cb, &mainloop_done);

	while (g_main_context_pending (NULL))
		g_main_context_iteration (NULL, FALSE);
	
	while (g_main_context_iteration (NULL, TRUE) && !mainloop_done)
		;
}

#if 0
static void
realize_cb (GtkWidget *socket, gpointer user_data)
{
	GtkWidget *plug, *w;

	g_warning ("Realize");

	plug = gtk_plug_new (0);
	w = gtk_button_new_with_label ("Baa");
	gtk_widget_show_all (w);
	gtk_widget_show (plug);
	gtk_container_add (GTK_CONTAINER (plug), w);
	GTK_PLUG (plug)->socket_window = GTK_WIDGET (socket)->window;
	gtk_socket_add_id (GTK_SOCKET (socket),
			   gtk_plug_get_id (GTK_PLUG (plug)));
	gdk_window_show (GTK_WIDGET (plug)->window);
}
#endif

static void
run_tests (GtkContainer *parent,
	   gboolean      wait_for_realize,
	   gboolean      fake_remote)
{
	GtkWidget  *vbox;
	DestroyType t;
	Test       *tests[DESTROY_TYPE_LAST];

	vbox = gtk_vbox_new (TRUE, 2);
	gtk_widget_show (vbox);
	gtk_container_add (parent, vbox);


#if 0
	{ /* Test raw plug / socket */
		GtkWidget *socket;

		g_warning ("Foo Bar. !!!");

		socket = gtk_socket_new ();
		g_signal_connect (G_OBJECT (socket), "realize",
				  G_CALLBACK (realize_cb), NULL);
		gtk_widget_show (GTK_WIDGET (socket));
		gtk_box_pack_start (GTK_BOX (vbox), socket, TRUE, TRUE, 2);
	}
#endif

	printf ("create\n");
	for (t = 0; t < DESTROY_TYPE_LAST; t++) {

		tests [t] = create_test (fake_remote);

		gtk_box_pack_start (
			GTK_BOX (vbox), 
			GTK_WIDGET (tests [t]->bonobo_widget),
			TRUE, TRUE, 2);
	}

	if (wait_for_realize)
		mainloop_for (100);

	printf ("show / hide\n");
	gtk_widget_hide (GTK_WIDGET (parent));

	if (wait_for_realize)
		mainloop_for (100);

	gtk_widget_show (GTK_WIDGET (parent));

	if (wait_for_realize)
		mainloop_for (100);

	printf ("re-add\n");
	g_object_ref (G_OBJECT (vbox));
	gtk_container_remove (parent, vbox);
	gtk_container_add (parent, vbox);

	if (wait_for_realize)
		mainloop_for (100);

	printf ("re-parent\n");
	for (t = 0; t < DESTROY_TYPE_LAST; t++) {

		g_object_ref (tests [t]->bonobo_widget);
		gtk_container_remove (GTK_CONTAINER (GTK_BOX (vbox)),
				      tests [t]->bonobo_widget);

		gtk_box_pack_start (
			GTK_BOX (vbox), 
			GTK_WIDGET (tests [t]->bonobo_widget),
			TRUE, TRUE, 2);

		g_object_unref (tests [t]->bonobo_widget);
	}

	if (wait_for_realize)
		mainloop_for (100);

	printf ("destroy\n");
	for (t = 0; t < DESTROY_TYPE_LAST; t++) {
		destroy_test (tests [t], t);
		mainloop_for (0);
	}

	gtk_widget_destroy (GTK_WIDGET (vbox));
}

static void
simple_tests (void)
{
	BonoboControl *control;
	GtkWidget     *widget = gtk_button_new_with_label ("Foo");

	control = bonobo_control_new (widget);
	bonobo_object_unref (BONOBO_OBJECT (control));

	control = bonobo_control_new (gtk_entry_new ());
	bonobo_object_unref (BONOBO_OBJECT (control));
}

static void
test_gtk_weakrefs (void)
{
	gpointer   ref;
	GtkObject *object = g_object_new (GTK_TYPE_BUTTON, NULL);

	ref = object;
	gtk_object_ref (object);
	gtk_object_sink (object);
	g_object_add_weak_pointer (ref, &ref);
	gtk_object_destroy (object);
	g_object_unref (object);

	g_assert (ref == NULL);
}

int
main (int argc, char **argv)
{
	GtkWidget *window;
	CORBA_ORB  orb;

	free (malloc (8));

	textdomain (GETTEXT_PACKAGE);

	if (!bonobo_ui_init ("test-focus", VERSION, &argc, argv))
		g_error ("Can not bonobo_ui_init");

	orb = bonobo_orb ();

	bonobo_activate ();

	test_gtk_weakrefs ();

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Control test");

	gtk_widget_show_all (window);

	simple_tests ();

	run_tests (GTK_CONTAINER (window), TRUE, TRUE);
	run_tests (GTK_CONTAINER (window), FALSE, TRUE);

	gtk_widget_destroy (window);

	return bonobo_ui_debug_shutdown ();
}
