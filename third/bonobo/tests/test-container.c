/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * test-container.c
 *
 * A simple program to act as a test container for embeddable
 * components.
 *
 * Authors:
 *    Nat Friedman (nat@nat.org)
 *    Miguel de Icaza (miguel@gnu.org)
 */
 
#include <config.h>
#include <gnome.h>
#include <liboaf/liboaf.h>

#include <gdk/gdkprivate.h>
#include <gdk/gdkx.h>
#include <bonobo.h>
#include <sys/stat.h>
#include <unistd.h>

CORBA_Environment ev;
CORBA_ORB orb;

/*
 * A handle to some Embeddables and their ClientSites so we can add
 * views to existing components.
 */
BonoboObjectClient *text_obj;
BonoboControlFrame *text_control_frame;

BonoboObjectClient *image_png_obj;
BonoboClientSite   *image_client_site;

BonoboObjectClient *paint_obj;
BonoboClientSite   *paint_client_site;

/*
 * The currently active view.  We keep track of this
 * so we can deactivate it when a new view is activated.
 */
BonoboViewFrame *active_view_frame;

char *server_id = "OAFIID:test_bonobo_object:b1ff15bb-d54f-4814-ba53-d67d3afd70fe";

typedef struct {
	GtkWidget *app;
	BonoboItemContainer *container;
	GtkWidget *box;
	Bonobo_View view;
	Bonobo_UIContainer corba_container;
	BonoboUIComponent *uic;
} Application;

static BonoboObjectClient *
launch_server (BonoboClientSite *client_site, BonoboItemContainer *container, char *id)
{
	BonoboObjectClient *object_server;
	
	bonobo_item_container_add (container, BONOBO_OBJECT (client_site));

	printf ("Launching...\n");
	object_server = bonobo_object_activate (id, 0);
	printf ("Return: %p\n", object_server);
	if (!object_server){
		g_warning (_("Can not activate object_server"));
		return NULL;
	}

	if (!bonobo_client_site_bind_embeddable (client_site, object_server)){
		g_warning (_("Can not bind object server to client_site"));
		return NULL;
	}

	return object_server;
}

static BonoboObjectClient *
launch_server_moniker (BonoboClientSite    *client_site,
		       BonoboItemContainer *container,
		       Bonobo_Moniker       moniker,
		       CORBA_Environment   *ev)
{
	char *moniker_name;
	BonoboObjectClient *object_server;
	
	bonobo_item_container_add (container, BONOBO_OBJECT (client_site));

	moniker_name = bonobo_moniker_client_get_name (moniker, ev);
	printf ("Launching moniker '%s' ...\n", moniker_name);
	CORBA_free (moniker_name);

	object_server = bonobo_moniker_client_resolve_client_default (
		moniker, "IDL:Bonobo/Embeddable:1.0", ev);

	printf ("Return: %p\n", object_server);
	if (!object_server) {
		g_warning (_("Can not activate object_server"));
		return NULL;
	}

	if (!bonobo_client_site_bind_embeddable (client_site, object_server)) {
		g_warning (_("Can not bind object server to client_site"));
		return NULL;
	}

	return object_server;
}

/*
 * This function is called when the user double clicks on a View in
 * order to activate it.
 */
static gint
user_activation_request_cb (BonoboViewFrame *view_frame)
{
	/*
	 * If there is already an active View, deactivate it.
	 */
        if (active_view_frame != NULL) {
		/*
		 * This just sends a notice to the embedded View that
		 * it is being deactivated.  We will also forcibly
		 * cover it so that it does not receive any Gtk
		 * events.
		 */
                bonobo_view_frame_view_deactivate (active_view_frame);

		/*
		 * Here we manually cover it if it hasn't acquiesced.
		 * If it has consented to be deactivated, then it will
		 * already have notified us that it is inactive, and
		 * we will have covered it and set active_view_frame
		 * to NULL.  Which is why this check is here.
		 */
		if (active_view_frame != NULL)
			bonobo_view_frame_set_covered (active_view_frame, TRUE);
									     
		active_view_frame = NULL;
	}

        /*
	 * Activate the View which the user clicked on.  This just
	 * sends a request to the embedded View to activate itself.
	 * When it agrees to be activated, it will notify its
	 * ViewFrame, and our view_activated_cb callback will be
	 * called.
	 *
	 * We do not uncover the View here, because it may not wish to
	 * be activated, and so we wait until it notifies us that it
	 * has been activated to uncover it.
	 */
        bonobo_view_frame_view_activate (view_frame);

        return FALSE;
}                                                                               

/*
 * Gets called when the View notifies the ViewFrame that it would like
 * to be activated or deactivated.
 */
static gint
view_activated_cb (BonoboViewFrame *view_frame, gboolean activated)
{

        if (activated) {
		/*
		 * If the View is requesting to be activated, then we
		 * check whether or not there is already an active
		 * View.
		 */
		if (active_view_frame != NULL) {
			g_warning ("View requested to be activated but there is already "
				   "an active View!");
			return FALSE;
		}

		/*
		 * Otherwise, uncover it so that it can receive
		 * events, and set it as the active View.
		 */
		bonobo_view_frame_set_covered (view_frame, FALSE);
                active_view_frame = view_frame;
        } else {
		/*
		 * If the View is asking to be deactivated, always
		 * oblige.  We may have already deactivated it (see
		 * user_activation_request_cb), but there's no harm in
		 * doing it again.  There is always the possibility
		 * that a View will ask to be deactivated when we have
		 * not told it to deactivate itself, and that is
		 * why we cover the view here.
		 */
		bonobo_view_frame_set_covered (view_frame, TRUE);

		if (view_frame == active_view_frame)
			active_view_frame = NULL;
        }                                                                       

        return FALSE;
}                                                                               

static BonoboViewFrame *
add_view (Application *app, BonoboClientSite *client_site, BonoboObjectClient *server) 
{
	BonoboViewFrame *view_frame;
	GtkWidget *view_widget;
	GtkWidget *frame;
	
	view_frame = bonobo_client_site_new_view (client_site, CORBA_OBJECT_NIL);

	gtk_signal_connect (GTK_OBJECT (view_frame), "user_activate",
			    GTK_SIGNAL_FUNC (user_activation_request_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (view_frame), "view_activated",
			    GTK_SIGNAL_FUNC (view_activated_cb), NULL);

	view_widget = bonobo_view_frame_get_wrapper (view_frame);

	frame = gtk_frame_new ("Embeddable");
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (app->box), frame, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (frame), view_widget);

	gtk_widget_show_all (frame);

	return view_frame;
} /* add_view */

static void
add_control (Application *app, BonoboControlFrame *control_frame, BonoboObjectClient *server) 
{
	GtkWidget *control_widget;
	GtkWidget *frame;
	
	control_widget = bonobo_control_frame_get_widget (control_frame);

	frame = gtk_frame_new ("Control");
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (app->box), frame, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (frame), control_widget);

	gtk_widget_show_all (frame);
} /* add_control */

static BonoboObjectClient *
add_cmd (Application *app, char *server_id, BonoboClientSite **client_site)
{
	BonoboObjectClient *server;
	
	*client_site = bonobo_client_site_new (app->container);

	server = launch_server (*client_site, app->container, server_id);
	if (server == NULL)
		return NULL;

	add_view (app, *client_site, server);
	return server;
}

static BonoboObjectClient *
add_cmd_control (Application *app, char *server_id, BonoboControlFrame **control_frame)
{
	BonoboObjectClient *server;
	Bonobo_Control corba_control;
	
	*control_frame = bonobo_control_frame_new (app->corba_container);

	printf ("Launching...\n");
	server = bonobo_object_activate (server_id, 0);
	printf ("Return: %p\n", server);
	if (!server){
		g_warning (_("Can not activate server"));
		return NULL;
	}

	corba_control = bonobo_object_corba_objref (BONOBO_OBJECT (server));
	bonobo_control_frame_bind_to_control (*control_frame, corba_control);
	bonobo_control_frame_control_activate (*control_frame);

	add_control (app, *control_frame, server);
	return server;
}

static BonoboObjectClient *
add_cmd_moniker (Application *app, Bonobo_Moniker moniker, BonoboClientSite **client_site,
		 CORBA_Environment *ev)
{
	BonoboObjectClient *server;
	
	*client_site = bonobo_client_site_new (app->container);

	server = launch_server_moniker (*client_site, app->container,
					moniker, ev);
	if (server == NULL)
		return NULL;

	add_view (app, *client_site, server);
	return server;
}

static void
verb_AddObject_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	Application *app = user_data;
	BonoboClientSite *client_site;

	add_cmd (app, server_id, &client_site);
}

static void
verb_AddImage_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	Application *app = user_data;
	BonoboObjectClient *object;
	BonoboStream *stream;
	Bonobo_PersistStream persist;

	object = add_cmd (app, "OAFIID:GNOME_EOG_Control",
			  &image_client_site);

	if (object == NULL) {
		gnome_warning_dialog (_("Could not launch bonobo object."));
		return;
	}

	image_png_obj = object;

	persist = bonobo_object_client_query_interface (
		object, "IDL:Bonobo/PersistStream:1.0", NULL);

        if (persist == CORBA_OBJECT_NIL) {
		printf ("No persist-stream interface\n");
                return;
	}

	printf ("Good: Embeddable supports PersistStream\n");
	
	stream = bonobo_stream_open ("fs", "/tmp/a.png", 
				     Bonobo_Storage_READ, 0664);

	if (stream == NULL) {
		printf ("I could not open /tmp/a.png!\n");
		return;
	}
	
	Bonobo_PersistStream_load (
		persist,
		(Bonobo_Stream) bonobo_object_corba_objref (BONOBO_OBJECT (stream)), "", &ev);

	Bonobo_Unknown_unref  (persist, &ev);
	CORBA_Object_release (persist, &ev);
}

static void
verb_AddPdf_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	Application *app = user_data;
	BonoboObjectClient *object;
	BonoboStream *stream;
	Bonobo_PersistStream persist;

	g_error ("Wrong oafid for pdf");
	object = add_cmd (app, "bonobo-object:application-x-pdf", &image_client_site);

	if (object == NULL)
	  {
	    gnome_warning_dialog (_("Could not launch bonobo object."));
	    return;
	  }

	image_png_obj = object;

	persist = bonobo_object_client_query_interface (
		object, "IDL:Bonobo/PersistStream:1.0", NULL);

        if (persist == CORBA_OBJECT_NIL)
                return;

	printf ("Good: Embeddable supports PersistStream\n");
	
	stream = bonobo_stream_open ("fs", "/tmp/a.pdf", 
				     Bonobo_Storage_READ, 0644);

	if (stream == NULL){
		printf ("I could not open /tmp/a.pdf!\n");
		return;
	}
	
	Bonobo_PersistStream_load (
		persist,
		(Bonobo_Stream) bonobo_object_corba_objref (BONOBO_OBJECT (stream)), "", &ev);

	Bonobo_Unknown_unref (persist, &ev);
	CORBA_Object_release (persist, &ev);
}

/*
 * Add a new view for the existing application/x-png Embeddable.
 */
static void
verb_AddImageView_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	Application *app = user_data;

	if (image_png_obj == NULL)
		return;

	add_view (app, image_client_site, image_png_obj);
} /* add_image_view */

static void
verb_AddGnumeric_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	Application *app = user_data;
	BonoboClientSite *client_site;
	Bonobo_Moniker    moniker;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	moniker = bonobo_moniker_client_new_from_name (
		"file:/tmp/sales.gnumeric!Sheet1!A1:D1", &ev);

	add_cmd_moniker (app, moniker, &client_site, &ev); 

	bonobo_object_release_unref (moniker, &ev);

	CORBA_exception_free (&ev);
}

static int
item_event_handler (GnomeCanvasItem *item, GdkEvent *event)
{
	static double last_x, last_y;
	static int pressed;
	double delta_x, delta_y;
	
	switch (event->type){
	case GDK_BUTTON_PRESS:
		pressed = 1;
		last_x = event->button.x;
		last_y = event->button.y;
		printf ("Evento: %g %g\n", last_x, last_y);
		break;

	case GDK_MOTION_NOTIFY:
		if (!pressed)
			return FALSE;
		
		delta_x = event->motion.x - last_x;
		delta_y = event->motion.y - last_y;
		gnome_canvas_item_move (item, delta_x, delta_y);
		printf ("Motion: %g %g\n", delta_x, delta_y);
		last_x = event->motion.x;
		last_y = event->motion.y;
		break;

	case GDK_BUTTON_RELEASE:
		pressed = 0;
		break;
		
	default:
		return FALSE;
	}
	return TRUE;
}

static void
do_add_canvas_cmd (Application *app, gboolean aa)
{
	BonoboClientSite *client_site;
	GtkWidget *canvas, *frame, *sw;
	CORBA_Environment ev;
	BonoboObjectClient *server;
	GnomeCanvasItem *item;
	
	client_site = bonobo_client_site_new (app->container);

	server = launch_server (
		client_site, app->container,
		"OAFIID:Bonobo_Sample_CanvasItem");

	if (server == NULL) {
		g_warning ("Can not activate OAFIID:Bonobo_SampleCanvasItem");
		return;
	}
	CORBA_exception_init (&ev);

	/*
	 * Setup our demostration canvas
	 */
	sw = gtk_scrolled_window_new (NULL, NULL);
	if (aa) {
		gtk_widget_push_visual (gdk_rgb_get_visual ());
		gtk_widget_push_colormap (gdk_rgb_get_cmap ());
		canvas = gnome_canvas_new_aa ();
		gtk_widget_pop_visual ();
		gtk_widget_pop_colormap ();
	} else
		canvas = gnome_canvas_new ();
	
	gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas), 0, 0, 400, 400);
	gtk_widget_set_usize (canvas, 400, 400);

	/*
	 * Add a background
	 */
	gnome_canvas_item_new (
		GNOME_CANVAS_GROUP (gnome_canvas_root (GNOME_CANVAS (canvas))),
		gnome_canvas_rect_get_type (),
		"x1", 0.0,
		"y1", 0.0,
		"x2", 400.0,
		"y2", 400.0,
		"fill_color", "red",
		"outline_color", "blue",
		"width_pixels", 8,
		NULL);

	/*
	 * The remote item
	 */
	item = bonobo_client_site_new_item (
		BONOBO_CLIENT_SITE (client_site),
		GNOME_CANVAS_GROUP (gnome_canvas_root (GNOME_CANVAS (canvas))));

	gtk_signal_connect (
		GTK_OBJECT (item), "event",
		GTK_SIGNAL_FUNC (item_event_handler), NULL);
	
	frame = gtk_frame_new ("Canvas with a remote item");
	gtk_box_pack_start (GTK_BOX (app->box), frame, TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (frame), sw);
	gtk_container_add (GTK_CONTAINER (sw), canvas);
	gtk_widget_show_all (frame);
}

static void
verb_AddCanvas_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	Application *app = user_data;

	do_add_canvas_cmd (app, FALSE);
}

static void
verb_AddCanvasAA_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	Application *app = user_data;

	do_add_canvas_cmd (app, TRUE);
}

static void
verb_AddPaint_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	Application *app = user_data;
	BonoboObjectClient *object;

	object = add_cmd (app, "OAFIID:Bonobo_Sample_Paint_Embeddable",
			  &paint_client_site);

	if (object == NULL)
	  {
	    gnome_warning_dialog (_("Could not launch Embeddable."));
	    return;
	  }

	paint_obj = object;
}

static void
verb_AddPaintView_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	Application *app = user_data;

	if (paint_obj == NULL)
		return;

	add_view (app, paint_client_site, paint_obj);
}

/*
 * This function uses Bonobo::PersistStream to load a set of data into
 * the text/plain Embeddable.
 */
static void
verb_AddText_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	Application *app = user_data;
	BonoboObjectClient *object;
	BonoboStream *stream;
	Bonobo_PersistStream persist;

	object = add_cmd_control (app,
				  "OAFIID:bonobo_text-plain:26e1f6ba-90dd-4783-b304-6122c4b6c821",
				  &text_control_frame);

	if (object == NULL) {
		gnome_warning_dialog (_("Could not launch Control."));
		return;
	}

	text_obj = object;

	persist = bonobo_object_client_query_interface (object,
		"IDL:Bonobo/PersistStream:1.0", &ev);

        if (ev._major != CORBA_NO_EXCEPTION)
                return;

        if (persist == CORBA_OBJECT_NIL)
                return;

	printf ("Good: Control supports PersistStream\n");
	
	stream = bonobo_stream_open ("fs", "/etc/passwd", 
				     Bonobo_Storage_READ, 0644);

	if (stream == NULL){
		printf ("I could not open /etc/passwd!\n");
		return;
	}
	
	Bonobo_PersistStream_load (
	     persist, (Bonobo_Stream) bonobo_object_corba_objref (BONOBO_OBJECT (stream)), "", &ev);

	Bonobo_Unknown_unref (persist, &ev);
	CORBA_Object_release (persist, &ev);
} /* add_text_cmd */

/*
 * These functions handle the progressive transmission of data
 * to the text/plain Embeddable.
 */
struct progressive_timeout {
	Bonobo_ProgressiveDataSink psink;
	FILE *f;
};

/*
 * Send a new line to the text/plain Embeddable.
 */
static gboolean
timeout_next_line (gpointer data)
{
	struct progressive_timeout *tmt = (struct progressive_timeout *) data;
  
	Bonobo_ProgressiveDataSink_iobuf *buffer;
	char line[1024];
	int line_len;
	
	if (fgets (line, sizeof (line), tmt->f) == NULL)
	{
		Bonobo_ProgressiveDataSink_end (tmt->psink, &ev);

		fclose (tmt->f);

		Bonobo_Unknown_unref (tmt->psink, &ev);
		CORBA_Object_release (tmt->psink, &ev);

		g_free (tmt);
		return FALSE;
	}

	line_len = strlen (line);

	buffer = Bonobo_ProgressiveDataSink_iobuf__alloc ();
	CORBA_sequence_set_release (buffer, TRUE);

	buffer->_length = line_len;
	buffer->_buffer = CORBA_sequence_CORBA_octet_allocbuf (line_len);
	memcpy (buffer->_buffer, line, line_len);

	Bonobo_ProgressiveDataSink_addData (tmt->psink, buffer, &ev);

	return TRUE;
} /* timeout_add_more_data */

/*
 * Setup a timer to send a new line to the text/plain Embeddable using
 * ProgressiveDataSink.
 */
static void
verb_SendText_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	Bonobo_ProgressiveDataSink psink;
	struct progressive_timeout *tmt;
	struct stat statbuf;
	FILE *f;

	if (text_obj == NULL)
		return;

	psink = bonobo_object_client_query_interface (text_obj,
                 "IDL:Bonobo/ProgressiveDataSink:1.0", NULL);

        if (psink == CORBA_OBJECT_NIL)
                return;

	printf ("Good: Control supports ProgressiveDataSink\n");

	tmt = g_new0 (struct progressive_timeout, 1);

	Bonobo_ProgressiveDataSink_start (psink, &ev);

	f = fopen ("/etc/passwd", "r");
	if (f == NULL) {
		printf ("I could not open /etc/passwd!\n");
		return;
	}
	
	fstat (fileno (f), &statbuf);
	Bonobo_ProgressiveDataSink_setSize (psink,
					    (CORBA_long) statbuf.st_size,
					    &ev);

	tmt->psink = psink;
	tmt->f = f;

	g_timeout_add (500, timeout_next_line, (gpointer) tmt);
} /* send_text_cmd */

static void
verb_Exit_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	gtk_main_quit ();
}

static const char *commands =
"<commands>\n"
"	<cmd name=\"AddText\" _label=\"_Add a new text/plain component\"/>\n"
"	<cmd name=\"SendText\" _label=\"_Send progressive data to an existing text/plain component\"/>\n"
"	<cmd name=\"AddPaint\" _label=\"_Add a new simple paint component\"/>\n"
"	<cmd name=\"AddPaintView\" _label=\"Add a new _view to an existing paint component\"/>\n"
"	<cmd name=\"AddImage\" _label=\"_Add a new application/x-png component\"/>\n"
"	<cmd name=\"AddImageView\" _label=\"Add a new _view to an existing application/x-png component\"/>\n"
"	<cmd name=\"AddPdf\" _label=\"_Add a new application\\x-pdf component\"/>\n"
"	<cmd name=\"AddGnumeric\" _label=\"Add a new Gnumeric instance through monikers\"/>\n"
"	<cmd name=\"AddObject\" _label=\"Add a new _object\"/>\n"
"	<cmd name=\"AddCanvasAA\" _label=\"Add a new Sample-Canvas item on an AA canvas\"/>\n"
"	<cmd name=\"AddCanvas\" _label=\"Add a new Sample-Canvas item on a regular canvas\"/>\n"
"	<cmd name=\"Exit\" _label=\"E_xit\" _tip=\"Exits the application\" pixtype=\"stock\"\n"
"		pixname=\"Quit\" accel=\"*Control*q\"/>\n"
"</commands>";

static const char *menus =
"<menu>\n"
"	<submenu name=\"File\" _label=\"_File\">\n"
"		<menuitem name=\"AddObject\" verb=\"\"/>\n"
"		<separator/>\n"
"		<menuitem name=\"Exit\"  verb=\"\"/>\n"
"	</submenu>\n"
"	<submenu name=\"TextPlain\" _label=\"_text/plain\">\n"
"		<menuitem name=\"AddText\" verb=\"\"/>\n"
"		<menuitem name=\"SendText\" verb=\"\"/>\n"
"	</submenu>\n"
"	<submenu name=\"ImagePng\" _label=\"_image/x-png\">\n"
"		<menuitem name=\"AddImage\" verb=\"\"/>\n"
"		<menuitem name=\"AddImageView\" verb=\"\"/>\n"
"	</submenu>\n"
"	<submenu name=\"AppPdf\" _label=\"_app/x-pdf\">\n"
"		<menuitem name=\"AddPdf\" verb=\"\"/>\n"
"	</submenu>\n"
"	<submenu name=\"PaintSample\" _label=\"paint sample\">\n"
"		<menuitem name=\"AddPaint\" verb=\"\"/>\n"
"		<menuitem name=\"AddPaintView\" verb=\"\"/>\n"
"	</submenu>\n"
"	<submenu name=\"Gnumeric\" _label=\"Gnumeric\">\n"
"		<menuitem name=\"AddGnumeric\" verb=\"\"/>\n"
"	</submenu>\n"
"	<submenu name=\"Canvas\" _label=\"Canvas-based\">\n"
"		<menuitem name=\"AddCanvasAA\" verb=\"\"/>\n"
"		<menuitem name=\"AddCanvas\" verb=\"\"/>\n"
"	</submenu>\n"
"</menu>";

static BonoboUIVerb verbs [] = {
	BONOBO_UI_VERB ("AddText", verb_AddText_cb),
	BONOBO_UI_VERB ("SendText", verb_SendText_cb),
	BONOBO_UI_VERB ("AddPaint", verb_AddPaint_cb),
	BONOBO_UI_VERB ("AddPaintView", verb_AddPaintView_cb),
	BONOBO_UI_VERB ("AddImage", verb_AddImage_cb),
	BONOBO_UI_VERB ("AddImageView", verb_AddImageView_cb),
	BONOBO_UI_VERB ("AddPdf", verb_AddPdf_cb),
	BONOBO_UI_VERB ("AddGnumeric", verb_AddGnumeric_cb),
	BONOBO_UI_VERB ("AddObject", verb_AddObject_cb),
	BONOBO_UI_VERB ("AddCanvasAA", verb_AddCanvasAA_cb),
	BONOBO_UI_VERB ("AddCanvas", verb_AddCanvas_cb),
	BONOBO_UI_VERB ("Exit", verb_Exit_cb),
	BONOBO_UI_VERB_END
};


static Application *
application_new (void)
{
	Application *app;
	BonoboUIContainer *ui_container;
	Bonobo_UIContainer corba_container;

	app = g_new0 (Application, 1);
	app->container = BONOBO_ITEM_CONTAINER (bonobo_item_container_new ());
	app->box = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (app->box);

	app->app = bonobo_window_new ("test-container",
				      "Sample Container Application");
	gtk_widget_set_usize (GTK_WIDGET (app->app), 600, 600);

	bonobo_window_set_contents (BONOBO_WINDOW (app->app), app->box);

	ui_container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (ui_container, BONOBO_WINDOW (app->app));

	corba_container = bonobo_object_corba_objref (BONOBO_OBJECT (ui_container));
	app->corba_container = bonobo_object_dup_ref (corba_container, NULL);

	app->uic = bonobo_ui_component_new ("test-container");
	bonobo_ui_component_set_container (app->uic, app->corba_container);

	/*
	 * Create the menus.
	 */
	bonobo_ui_component_set_translate (app->uic, "/", commands, NULL);
	bonobo_ui_component_set_translate (app->uic, "/", menus, NULL);

	bonobo_ui_component_add_verb_list_with_data (app->uic, verbs, app);

	gtk_widget_show (app->app);

	return app;
}

int
main (int argc, char *argv [])
{
	Application *app;

	if (argc != 1)
		server_id = argv [1];
	
	CORBA_exception_init (&ev);
	
        gnome_init_with_popt_table ("MyShell", "1.0",
				    argc, argv,
				    oaf_popt_options, 0, NULL); 

	orb = oaf_init (argc, argv);

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error (_("Can not bonobo_init"));

	app = application_new ();
	
	bonobo_activate ();
	gtk_main ();

	CORBA_exception_free (&ev);
	
	return 0;
}
