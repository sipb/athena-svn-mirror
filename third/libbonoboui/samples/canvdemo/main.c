#include "config.h"
#include <libgnome/gnome-init.h>
#include <bonobo.h>
#include <bonobo/bonobo-canvas-item.h>
#include <bonobo/bonobo-moniker-util.h>

/*
static gboolean
test_event (GnomeCanvasItem *item, GdkEvent *event, GnomeCanvasItem *item2)
{
        double item_x, item_y;
        static double last_x, last_y;
        static gboolean dragging = FALSE;

        item_x = event->button.x;
        item_y = event->button.y;

        gnome_canvas_item_w2i (item->parent, &item_x, &item_y);

        switch (event->type) {
        case GDK_BUTTON_PRESS:
                printf("GDK_BUTTON_PRESS\n");
                switch (event->button.button) {
                case 1:
                        last_x = item_x;
                        last_y = item_y;

                        dragging = TRUE;
                        break;

                default:
                        break;
                }

                break;

        case GDK_MOTION_NOTIFY:
                printf("GDK_MOTION_NOTIFY\n");
                if (dragging && 
                                (event->motion.state & GDK_BUTTON1_MASK)) {
                        last_x = item_x - last_x;
                        last_y = item_y - last_y;
                        gnome_canvas_item_move(item2, last_x, last_y);
                        last_x = item_x;
                        last_y = item_y;
                }
                break;

        case GDK_BUTTON_RELEASE:
                printf("GDK_BUTTON_RELEASE\n");
                dragging = FALSE;
                break;

        case GDK_ENTER_NOTIFY:
                printf("GDK_ENTER_NOTIFY\n");
                break;

        case GDK_LEAVE_NOTIFY:
                printf("GDK_LEAVE_NOTIFY\n");
                break;

        case GDK_KEY_PRESS:
                printf("GDK_KEY_PRESS\n");
                break;

        case GDK_KEY_RELEASE:
                printf("GDK_KEY_RELEASE\n");
                break;

        case GDK_FOCUS_CHANGE:
                printf("GDK_FOCUS_CHANGE\n");
                break;

        default:
                printf("OTHER\n");
                break;
        }

        return FALSE;
}

static GnomeCanvasGroup*
setup_return_test(GnomeCanvasGroup *group)
{
        GnomeCanvasItem *group2, *item2;

        group2 = gnome_canvas_item_new (
		GNOME_CANVAS_GROUP (group),
		gnome_canvas_group_get_type (),
		NULL);

        item2 = gnome_canvas_item_new (
		GNOME_CANVAS_GROUP (group2),
		gnome_canvas_rect_get_type (),
		"x1", 90.0,
		"y1", 90.0,
		"x2", 110.0,
		"y2", 110.0,
		"outline_color", "black",
		"fill_color", "red",
		NULL);

        g_signal_connect(G_OBJECT(group2), "event",
                         G_CALLBACK (test_event),
                         item2);
        return GNOME_CANVAS_GROUP(group2);
}
*/

static void
on_destroy  (GtkWidget *app, void *data)
{
	g_print ("Thank you for using canvas components!\n");
        bonobo_main_quit ();
}


GnomeCanvasItem* get_square(GnomeCanvasGroup* group)
{
        CORBA_Object server;
        GnomeCanvasItem *item;
        CORBA_Environment ev;
  
        CORBA_exception_init (&ev);
  
        server = bonobo_activation_activate_from_id ("OAFIID:SquareItem", 
                0, NULL, &ev);
  
        if (server == CORBA_OBJECT_NIL || BONOBO_EX (&ev))
        {
                g_warning (_("Could not activate square: '%s'"),
                              bonobo_exception_get_text (&ev));
                CORBA_exception_free(&ev);
                exit(0);
        }
        g_print("Got square component connect.\n");
        item = gnome_canvas_item_new (group, bonobo_canvas_item_get_type (),
                "corba_factory", server, NULL);
  
        /* I think this tells the object it is OK to exit.
        CORBA_Object_release(server, &ev);*/
        bonobo_object_release_unref(server, &ev);
        CORBA_exception_free(&ev);

        return item;
}

GnomeCanvasItem* get_circle(GnomeCanvasGroup* group)
{
        CORBA_Object server;
        GnomeCanvasItem *item;
        CORBA_Environment ev;
  
        CORBA_exception_init (&ev);
  
        server = bonobo_activation_activate_from_id ("OAFIID:CircleItem", 
                0, NULL, &ev);
  
        if (server == CORBA_OBJECT_NIL || BONOBO_EX (&ev))
        {
                g_warning (_("Could activate Circle: '%s'"),
                              bonobo_exception_get_text (&ev));
                CORBA_exception_free(&ev);
                exit(0);
        }
        g_print("Got circle component connect.\n");

        item = gnome_canvas_item_new (group, bonobo_canvas_item_get_type (),
                "corba_factory", server, NULL);
  
        /* I think this tells the object it is OK to exit.
           Probably want to call this when I close.
           or bonobo_object_release_unref(server, &ev)
        CORBA_Object_release(server, &ev);*/
        bonobo_object_release_unref(server, &ev);
        CORBA_exception_free(&ev);

        return item;
}

static guint
create_app (void)
{
	GtkWidget *canvas, *window, *box, *hbox, *control; 
        GnomeCanvasGroup *group;
	
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_signal_connect (GTK_OBJECT(window), "destroy", 
              GTK_SIGNAL_FUNC(on_destroy), NULL);

	gtk_widget_set_usize (GTK_WIDGET(window), 400, 300);
        box = gtk_vbox_new (FALSE, 2);
        gtk_container_add(GTK_CONTAINER(window), box);

	canvas = gnome_canvas_new();
        gtk_box_pack_start_defaults (GTK_BOX (box), canvas);
	get_square(gnome_canvas_root(GNOME_CANVAS(canvas)));
        get_circle(gnome_canvas_root(GNOME_CANVAS(canvas)));

        hbox = gtk_hbox_new(FALSE, 2);
        gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 0);

        control = bonobo_widget_new_control("OAFIID:Circle_Controller", NULL);
        gtk_box_pack_start (GTK_BOX (hbox), control, FALSE, FALSE, 0);

        control = bonobo_widget_new_control("OAFIID:Square_Controller", NULL);
        gtk_box_pack_start (GTK_BOX (hbox), control, FALSE, FALSE, 0);

	gtk_widget_show_all (GTK_WIDGET(window));

	return FALSE;
}

int 
main (int argc, char** argv)
{
	CORBA_ORB orb;
	
        if (!bonobo_ui_init (argv[0], VERSION, &argc, argv))
                g_error ("Could not initialize libbonoboui!\n");

	gtk_idle_add ((GtkFunction) create_app, NULL);

	bonobo_main ();

	return 0;
}

