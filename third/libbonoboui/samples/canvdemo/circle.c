/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <glib/gmacros.h>
#include <gtk/gtk.h>
#include <libgnomecanvas/gnome-canvas.h>
#include <bonobo/Bonobo.h>
#include <bonobo.h>

typedef struct
{
        int state;
        int dragging;
        int timer;
        double speed;
        double pos;
        double inc;
        double last_x;
        double last_y;
        const char *color;
        GSList *list;
} CommonData;

typedef struct
{
        GtkWidget *button;
        GtkAdjustment *adj;
        GnomeCanvasItem *item;
} ObjectData;

static void
move_component(ObjectData *object, CommonData *com)
{
        if (object->item)
        {
                gnome_canvas_item_set(object->item,
                       "y1", com->pos,
                       "y2", com->pos + 20,
                       "outline_color", "black",
                       "fill_color", com->color, NULL);
        }
}
                        

static gboolean 
move_all_components(CommonData *com)
{
        if (com->state && com->list)
        {
                com->pos += com->inc;
                if (com->pos > 50 || com->pos < -50) com->inc *= -1.0;
                if (com->inc >= 0) com->color = "blue";
                else com->color = "green";
                g_slist_foreach(com->list, (GFunc)move_component, com);
        }

        return 1;
}

static void
update_button(ObjectData *object, CommonData *com)
{
        if (object->button)
        {
                gtk_button_set_label(GTK_BUTTON(object->button), 
                                com->state ? "Stop" : "Start");
        }
}

static void
on_press (GtkWidget *button, CommonData *com)
{
        GSList *list = com->list;
        com->state = !com->state;

        g_slist_foreach(list, (GFunc)update_button, com);
}

static void
update_speed_label(ObjectData *object, int value)
{       
        if (object->adj)
        {
                gtk_adjustment_set_value(object->adj, (double)value);
        }
}

static void
set_speed(GtkAdjustment *adj, CommonData *com)
{
        int speed = adj->value > 0 ? (int) (10000/adj->value) : 0;

        if (speed != com->speed)
        {
                if (com->timer) g_source_remove(com->timer);

                if (speed > 0)
                        com->timer = g_timeout_add(speed,
                                (GSourceFunc)move_all_components, com);

                com->speed = speed;

                g_slist_foreach(com->list, (GFunc)update_speed_label, 
                                GINT_TO_POINTER((int)adj->value));
        }
}

BonoboObject *
circle_control_new (CommonData *com)
{
        BonoboPropertyBag  *pb;
        BonoboControl      *control;
        GParamSpec        **pspecs;
        guint               n_pspecs;
        GtkWidget *button, *frame, *spin, *hbox, *spin_label;
        GtkObject *adj;
        GSList **list = &com->list;
        GSList *li;
        ObjectData *object = NULL;

        frame = gtk_frame_new("Circle");
        hbox = gtk_hbox_new(FALSE, 2);
        gtk_container_add(GTK_CONTAINER(frame), hbox);
        button = gtk_button_new();
        gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
        spin_label = gtk_label_new ("Speed:");
        gtk_box_pack_start (GTK_BOX (hbox), spin_label, FALSE, FALSE, 0);
        adj = gtk_adjustment_new(100.0, 0.0, 1000.0, 1.0, 10.0, 10.0);
        g_signal_connect(adj, "value_changed", G_CALLBACK(set_speed), com);
        spin = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0.0, 0);
        gtk_box_pack_start (GTK_BOX (hbox), spin, FALSE, FALSE, 0);

        gtk_widget_show_all(frame);
        control = bonobo_control_new (frame);

        pb = bonobo_property_bag_new (NULL, NULL, NULL);
        bonobo_control_set_properties (control, BONOBO_OBJREF (pb), NULL);
        bonobo_object_unref (BONOBO_OBJECT (pb));

        g_signal_connect(button, "clicked", G_CALLBACK(on_press), com);

        pspecs = g_object_class_list_properties (
                G_OBJECT_GET_CLASS (button), &n_pspecs);

        bonobo_property_bag_map_params (
                pb, G_OBJECT (button), (const GParamSpec **)pspecs, n_pspecs);

        g_free (pspecs);

        bonobo_control_life_instrument (control);

        li = g_slist_last(*list);
        if (li)
        {
               object = li->data;
        }
        if (!object || object->button)
        {
                object = g_new0(ObjectData, 1);
                *list = g_slist_append(*list, object);
        }

        object->button = button;
        object->adj = GTK_ADJUSTMENT(adj);

        update_button(object, com);
        set_speed(GTK_ADJUSTMENT(adj), com);

        return BONOBO_OBJECT (control);
}

static BonoboObject *
control_factory (BonoboGenericFactory *this,
                 const char           *object_id,
                 gpointer             data)
{
        BonoboObject *object = NULL;

        g_return_val_if_fail (object_id != NULL, NULL);

        if (!strcmp (object_id, "OAFIID:Circle_Controller"))
        {
                object = circle_control_new ((CommonData *) data);
        }

        return object;
}

static void
drag_component(ObjectData *object, CommonData *com)
{
        if (object->item)
        {
                gnome_canvas_item_move (object->item, com->last_x, com->last_y);
        }
}

static gboolean
item_event (GnomeCanvasItem *item, GdkEvent *event, CommonData *com)
{
        double item_x, item_y;

        item_x = event->button.x;
        item_y = event->button.y;

        gnome_canvas_item_w2i (item->parent, &item_x, &item_y);

        switch (event->type) {
        case GDK_BUTTON_PRESS:
                switch (event->button.button) {
                case 1:
                        com->last_x = item_x;
                        com->last_y = item_y;

                        com->dragging = TRUE;
                        break;

                default:
                        break;
                }

                break;

        case GDK_MOTION_NOTIFY:
                if (com->dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
                        com->last_x = item_x - com->last_x;
                        com->last_y = item_y - com->last_y;
                        g_slist_foreach(com->list, (GFunc)drag_component, com);
                        com->last_x = item_x;
                        com->last_y = item_y;
                }
                break;

        case GDK_BUTTON_RELEASE:
                com->dragging = FALSE;
                break;

        }

        return FALSE;
}

static GnomeCanvasItem*
canvas_item_new(GnomeCanvas *canvas, gpointer data)
{
	GnomeCanvasItem *item;
	GnomeCanvasItem *group;
        GSList **list = &((CommonData *)data)->list;
        GSList *li;
        ObjectData *object = NULL;

        group = gnome_canvas_item_new (
		GNOME_CANVAS_GROUP (gnome_canvas_root (canvas)),
		gnome_canvas_group_get_type (),
		NULL);

        item = gnome_canvas_item_new (
		GNOME_CANVAS_GROUP (group),
		gnome_canvas_ellipse_get_type (),
		"x1", 0.0,
		"y1", 0.0,
		"x2", 20.0,
		"y2", 20.0,
		"outline_color", "red",
		"fill_color", "blue",
		NULL);

        li = g_slist_last(*list);
        if (li) {
                object = li->data;
        }

        if (!object || object->item) {
                object = g_new0(ObjectData, 1);
                *list = g_slist_append(*list, object);
        }

        object->item = item;

        g_signal_connect(G_OBJECT(item), "event",
                         G_CALLBACK (item_event),
                         data);

	return (group);
}

static BonoboCanvasComponent *
item_factory (GnomeCanvas *canvas, gpointer data)
{
	GnomeCanvasItem *group;
	BonoboCanvasComponent *component;

	group = canvas_item_new(canvas, data);

	component = bonobo_canvas_component_new (group);

	return component;
}

static BonoboObject *
bonobo_item_factory (BonoboGenericFactory *factory, const char *component,
                     gpointer data)
{
        BonoboObject *object = NULL;

        g_return_val_if_fail (component != NULL, NULL);

        if (!strcmp (component, "OAFIID:CircleItem"))
        {
                g_print("activation requested\n");
                object = BONOBO_OBJECT(
                   bonobo_canvas_component_factory_new (
                      item_factory, data));
        }
        else
        {
                g_print("attempted to activate w/ bad oid %s\n", component);
        }
        return object;
}

int
main (int argc, char *argv [])
{
        int retval;
        BonoboObject *factory;
        CommonData data;

        data.state = 1;
        data.list = NULL;
        data.pos = 0.0;
        data.inc = 2.0;
        data.timer = 0;
        data.speed = 0;

        if (!bonobo_ui_init (argv[0], VERSION, &argc, argv))
                g_error (_("Could not initialize Bonobo UI"));

        factory = BONOBO_OBJECT
		(bonobo_generic_factory_new
			("OAFIID:CircleItem_Factory",
			 bonobo_item_factory, &data));
        if (factory)
                bonobo_running_context_auto_exit_unref (factory);

        retval = bonobo_generic_factory_main
		("OAFIID:Circle_ControllerFactory",
		 control_factory, &data);

        return retval;
}
