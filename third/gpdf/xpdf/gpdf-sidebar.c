/*
 *  Copyright (C) 2000, 2001, 2002 Marco Pesenti Gritti
 *                2003 Remi Cohen-Scali
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: gpdf-sidebar.c,v 1.1.1.1 2004-10-06 18:43:56 ghudson Exp $
 */

#include <string.h>

#include "gpdf-sidebar.h"
#include "gpdf-util.h"
#include "gpdf-marshal.h"
#include "gpdf-stock-icons.h"

#include "eel-gconf-extensions.h"
#include "prefs-strings.h"

#include <gtk/gtk.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-uidefs.h>

static void gpdf_sidebar_class_init    (GPdfSidebarClass *klass);
static void gpdf_sidebar_instance_init (GPdfSidebar *window);
static void gpdf_sidebar_finalize      (GObject *object);

typedef struct
{
	char *id;
	char *title;
	gboolean can_remove;
	GtkWidget *item;
	GtkWidget *tools_menu;
} GPdfSidebarPage;

struct GPdfSidebarPrivate
{
	GtkWidget *content_frame;
	GtkWidget *title_frame;
	GtkWidget *title_menu;
	GtkWidget *title_button;
	GtkWidget *title_label;
	GtkWidget *title_hbox;
	GtkWidget *tools_button;
	GtkWidget *remove_button;
	GObject *content;
	GList *pages;
	GPdfSidebarPage *current;
	gboolean has_default_size;
};

static void gpdf_sidebar_finalize      (GObject *);
static void gpdf_sidebar_size_allocate (GtkWidget *, GtkAllocation *);
static void gpdf_sidebar_show 	       (GtkWidget *);
static void paned_destroy_cb 	       (GtkWidget *, GPdfSidebar *);

enum
{
	PAGE_CHANGED,
	CLOSE_REQUESTED,
	REMOVE_REQUESTED,
	LAST_SIGNAL
};

static guint sidebar_signals[LAST_SIGNAL] = { 0 };

GPDF_CLASS_BOILERPLATE(GPdfSidebar, gpdf_sidebar, GtkVBox, GTK_TYPE_VBOX);

static void
gpdf_sidebar_class_init (GPdfSidebarClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class;
	
	parent_class = g_type_class_peek_parent (klass);
	widget_class = (GtkWidgetClass*) klass;
	
        object_class->finalize = gpdf_sidebar_finalize;
	
	widget_class->size_allocate = gpdf_sidebar_size_allocate;
	widget_class->show = gpdf_sidebar_show;
	
	sidebar_signals[PAGE_CHANGED] =
                g_signal_new ("page_changed",
                              G_OBJECT_CLASS_TYPE (object_class),
                              G_SIGNAL_RUN_FIRST,
                              G_STRUCT_OFFSET (GPdfSidebarClass, page_changed),
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
			      G_TYPE_STRING);
	sidebar_signals[CLOSE_REQUESTED] =
                g_signal_new ("close_requested",
                              G_OBJECT_CLASS_TYPE (object_class),
                              G_SIGNAL_RUN_FIRST,
                              G_STRUCT_OFFSET (GPdfSidebarClass, close_requested),
                              NULL, NULL,
                              g_cclosure_marshal_VOID__VOID,
                              G_TYPE_NONE,
                              0);
	sidebar_signals[REMOVE_REQUESTED] =
                g_signal_new ("remove_requested",
                              G_OBJECT_CLASS_TYPE (object_class),
                              G_SIGNAL_RUN_FIRST,
                              G_STRUCT_OFFSET (GPdfSidebarClass, remove_requested),
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
			      G_TYPE_STRING);
}

static void
gpdf_sidebar_finalize (GObject *object)
{
        GPdfSidebar *sidebar;

        g_return_if_fail (object != NULL);
        g_return_if_fail (GPDF_IS_SIDEBAR (object));

        sidebar = GPDF_SIDEBAR (object);

        g_return_if_fail (sidebar->priv != NULL);

	/* FIXME free pages list */

	if (sidebar->priv->current)
	{
		eel_gconf_set_string (CONF_WINDOWS_SIDEBAR_PAGE,
				      sidebar->priv->current->id);
	}

        g_free (sidebar->priv);

        G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gpdf_sidebar_size_allocate (GtkWidget *widget,
                            GtkAllocation *allocation)
{
	int width;
	GtkAllocation child_allocation;
	GPdfSidebar *sidebar = GPDF_SIDEBAR(widget);
	GtkWidget *frame = sidebar->priv->title_frame;
	GtkWidget *hbox = sidebar->priv->title_hbox;
	GtkRequisition child_requisition;
	
	GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);
	
	gtk_widget_get_child_requisition (hbox, &child_requisition);
	width = child_requisition.width;
	
	child_allocation = frame->allocation;
	child_allocation.width = MAX (width, frame->allocation.width);
	
	gtk_widget_size_allocate (frame, &child_allocation);
}

static void
gpdf_sidebar_show (GtkWidget *widget)
{
	GPdfSidebar *sidebar = GPDF_SIDEBAR(widget);

	if (!sidebar->priv->current)
	{
		const char *id;

		id = eel_gconf_get_string (CONF_WINDOWS_SIDEBAR_PAGE);
		g_return_if_fail (id != NULL);
		
		gpdf_sidebar_select_page (sidebar, id);
	}

	if (!sidebar->priv->has_default_size)
	{
		GtkWidget *paned = GTK_WIDGET(sidebar)->parent;
		int pos;
		
		pos = eel_gconf_get_integer (CONF_WINDOWS_SIDEBAR_SIZE);
		gtk_paned_set_position (GTK_PANED(paned), pos);

		/* connect a signal to save it */
		g_signal_connect (G_OBJECT(paned), "destroy",
				  G_CALLBACK(paned_destroy_cb),
				  sidebar);

		sidebar->priv->has_default_size = TRUE;
	}
	
	GTK_WIDGET_CLASS (parent_class)->show (widget);	
}


static void
paned_destroy_cb (GtkWidget *widget, GPdfSidebar *sidebar)
{
	eel_gconf_set_integer (CONF_WINDOWS_SIDEBAR_SIZE,
			       gtk_paned_get_position (GTK_PANED(widget)));
}
	
static void
popup_position_func (GtkMenu *menu,
                    int *x, 
                    int *y,
                    gboolean *push_in,
                    gpointer user_data)
{
       GtkWidget *button;

       button = GTK_WIDGET (user_data);
       
       gdk_window_get_origin (button->parent->window, x, y);
       *x += button->allocation.x;
       *y += button->allocation.y + button->allocation.height;

       *push_in = FALSE;
}


static int
title_button_press_cb (GtkWidget *item, GdkEventButton *event, gpointer data)
{
       GPdfSidebar *sidebar;
       
       sidebar = GPDF_SIDEBAR (data);

       gtk_menu_popup (GTK_MENU (sidebar->priv->title_menu),
                       NULL, NULL,
                       popup_position_func, sidebar->priv->title_button, 
                       event->button, event->time);

       return TRUE;
}

static int
tools_button_press_cb (GtkWidget *item, GdkEventButton *event, gpointer data)
{
       GPdfSidebar *sidebar;
       
       sidebar = GPDF_SIDEBAR (data);

       if (sidebar->priv->current && 
           sidebar->priv->current->tools_menu &&
           GTK_IS_MENU (sidebar->priv->current->tools_menu))
               gtk_menu_popup (GTK_MENU (sidebar->priv->current->tools_menu),
                               NULL, NULL,
                               popup_position_func, sidebar->priv->tools_button, 
                               event->button, event->time);

       return TRUE;
}

static void
close_clicked_cb (GtkWidget *button, GPdfSidebar *sidebar)
{
	g_signal_emit (G_OBJECT (sidebar), sidebar_signals[CLOSE_REQUESTED], 0);
}

#if ANYTHING_THERE_THATS_REMOVABLE
static void
remove_clicked_cb (GtkWidget *button, GPdfSidebar *sidebar)
{
	g_return_if_fail (sidebar->priv->current != NULL);
	
	g_signal_emit (G_OBJECT (sidebar), sidebar_signals[REMOVE_REQUESTED], 0,
		       sidebar->priv->current->id);
}
#endif

static void
gpdf_sidebar_instance_init (GPdfSidebar *sidebar)
{
	GtkWidget *frame;
	GtkWidget *frame_hbox;
	GtkWidget *button_hbox;
	GtkWidget *close_button;
	GtkWidget *close_image;
#if ANYTHING_THERE_THATS_REMOVABLE
	GtkWidget *remove_image;
	GtkWidget *remove_button;
#endif
	GtkWidget *tools_image;
	GtkWidget *tools_button;
	GtkWidget *arrow;
        GtkTooltips *tooltips;

        tooltips = gtk_tooltips_new ();
	
        sidebar->priv = g_new0 (GPdfSidebarPrivate, 1);
	sidebar->priv->content = NULL;
	sidebar->priv->pages = NULL;
	sidebar->priv->current = NULL;
	sidebar->priv->has_default_size = FALSE;
		
	frame_hbox = gtk_hbox_new (FALSE, 0);
	
        sidebar->priv->title_button = gtk_button_new ();
        gtk_button_set_relief (GTK_BUTTON (sidebar->priv->title_button),
                               GTK_RELIEF_NONE);
        gtk_tooltips_set_tip (tooltips, sidebar->priv->title_button,
                              _("Select sidebar page by title for displaying it"), 
                              NULL);
        
	g_signal_connect (sidebar->priv->title_button, "button_press_event",
                          G_CALLBACK (title_button_press_cb),
                          sidebar);
       
        button_hbox = gtk_hbox_new (FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button_hbox), 0);
       
        sidebar->priv->title_label = gtk_label_new ("");
       
        /* FIXME: Temp */
        gtk_widget_show (sidebar->priv->title_label);

        gtk_box_pack_start (GTK_BOX (button_hbox), 
                            sidebar->priv->title_label,
                            FALSE, FALSE, 0);
       
        arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE);
        gtk_widget_show (arrow);
        gtk_box_pack_end (GTK_BOX (button_hbox), arrow, FALSE, FALSE, 0);
        gtk_tooltips_set_tip (tooltips, arrow,
                              _("Select sidebar page by title for displaying it"), 
                              NULL);
       
        gtk_widget_show (button_hbox);
       
        gtk_container_add (GTK_CONTAINER (sidebar->priv->title_button),
                           button_hbox);

        sidebar->priv->title_menu = gtk_menu_new ();
        gtk_widget_show (sidebar->priv->title_button);
        gtk_widget_show (sidebar->priv->title_menu);

        gtk_box_pack_start (GTK_BOX (frame_hbox), 
                            sidebar->priv->title_button,
                            FALSE, FALSE, 0);

        /*
         * Page tools menu sidebar button
         */
        /* Create tools menu button image */    /* or PREFERENCES or PROPERTIES */
        tools_image = gtk_image_new_from_stock (GTK_STOCK_EXECUTE,
                                                GTK_ICON_SIZE_SMALL_TOOLBAR);
        gtk_widget_show (tools_image);

        /* Create button itself */
        tools_button = gtk_button_new ();
	gtk_button_set_relief (GTK_BUTTON (tools_button), GTK_RELIEF_NONE);
        /* Connect button press event to tools CB */
	g_signal_connect (tools_button, "button_press_event",
                          G_CALLBACK (tools_button_press_cb), 
                          sidebar);
        gtk_tooltips_set_tip (tooltips, tools_button,
                              _("Click to open page tools menu"), 
                              NULL);

        /* Create replacement container for button */
        button_hbox = gtk_hbox_new (FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button_hbox), 0);

        /* Pack image */
        gtk_box_pack_start (GTK_BOX (button_hbox), 
                            tools_image,
                            FALSE, FALSE, 0);

        /* And an arrow */
        arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE);
        gtk_widget_show (arrow);
        gtk_box_pack_end (GTK_BOX (button_hbox), arrow, FALSE, FALSE, 0);
        gtk_tooltips_set_tip (tooltips, arrow,
                              _("Click to open page tools menu"), 
                              NULL);

        gtk_widget_show (button_hbox);

        gtk_container_add (GTK_CONTAINER (tools_button),
                           button_hbox);
        
        sidebar->priv->tools_button = tools_button;
        gtk_widget_show (tools_button);
        
#if ANYTHING_THERE_THATS_REMOVABLE
	/* Remove sidebar button */
	remove_image = gtk_image_new_from_stock (GTK_STOCK_DELETE, 
                                                 GTK_ICON_SIZE_SMALL_TOOLBAR);
        gtk_widget_show (remove_image);

	remove_button = gtk_button_new ();
        gtk_tooltips_set_tip (tooltips, remove_button,
                              _("Remove page if it is removeable"), 
                              NULL);

	gtk_button_set_relief (GTK_BUTTON (remove_button), GTK_RELIEF_NONE);
	gtk_container_add (GTK_CONTAINER (remove_button), remove_image);

	g_signal_connect (remove_button, "clicked",
                          G_CALLBACK (remove_clicked_cb), 
                          sidebar);

        gtk_widget_show (remove_button);

	sidebar->priv->remove_button = remove_button;
#endif
	
	/* Close button */
        close_image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, 
                                                GTK_ICON_SIZE_SMALL_TOOLBAR);
        gtk_widget_show (close_image);

	close_button = gtk_button_new ();
	gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
	gtk_container_add (GTK_CONTAINER (close_button), close_image);
	g_signal_connect (close_button, "clicked",
                          G_CALLBACK (close_clicked_cb), 
                          sidebar);
        gtk_tooltips_set_tip (tooltips, close_button,
                              _("Close sidebar"), 
                              NULL);

        gtk_widget_show (close_button); 
       
        gtk_box_pack_end (GTK_BOX (frame_hbox), close_button,
                          FALSE, FALSE, 0);
#if ANYTHING_THERE_THATS_REMOVABLE
	gtk_box_pack_end (GTK_BOX (frame_hbox), remove_button,
                          FALSE, FALSE, 0);
#endif
        gtk_box_pack_end (GTK_BOX (frame_hbox), tools_button,
                          FALSE, FALSE, 0);
	sidebar->priv->title_hbox = frame_hbox;

        frame = gtk_frame_new (NULL);
        gtk_frame_set_shadow_type (GTK_FRAME (frame), 
				   GTK_SHADOW_NONE);
        gtk_widget_show (frame);
	sidebar->priv->title_frame = frame;
 
        gtk_container_add (GTK_CONTAINER (frame), 
			   frame_hbox);

        gtk_widget_show (frame_hbox);

	gtk_box_pack_start (GTK_BOX (sidebar), frame,
                            FALSE, FALSE, 2);

        sidebar->priv->content_frame = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (sidebar->priv->content_frame);
		
        gtk_box_pack_start (GTK_BOX (sidebar), sidebar->priv->content_frame,
                            TRUE, TRUE, 0);
}

GtkWidget *
gpdf_sidebar_new (void)
{
	return GTK_WIDGET (g_object_new (GPDF_SIDEBAR_TYPE, NULL));
}

static void
set_tools_button_sensitivity (GPdfSidebar *sidebar, GPdfSidebarPage *page)
{
    if (!sidebar->priv->current) return;

    if (GTK_IS_WIDGET(sidebar->priv->tools_button) &&
        !strncmp(page->id,
                 sidebar->priv->current->id,
                 strlen(sidebar->priv->current->id)))
      gtk_widget_set_sensitive (GTK_WIDGET(sidebar->priv->tools_button),
                                page->tools_menu && GTK_IS_MENU (page->tools_menu));
}

static void
select_page (GPdfSidebar *sidebar, GPdfSidebarPage *page)
{
	sidebar->priv->current = page;

	gtk_label_set_text (GTK_LABEL (sidebar->priv->title_label),
                            page->title);
#if ANYTHING_THERE_THATS_REMOVABLE
	gtk_widget_set_sensitive (GTK_WIDGET(sidebar->priv->remove_button),
				  page->can_remove);
#endif
        set_tools_button_sensitivity (sidebar, page);
          
	g_signal_emit (G_OBJECT (sidebar), sidebar_signals[PAGE_CHANGED], 0, 
		       page->id);
}

static void
title_menu_item_activated_cb (GtkWidget *item, GPdfSidebar *sidebar)
{
	GPdfSidebarPage *page;

	page = (GPdfSidebarPage *) g_object_get_data (G_OBJECT(item), "page");

	select_page (sidebar, page);
}

void
gpdf_sidebar_add_page	(GPdfSidebar *sidebar,
			 const char *title,
			 const char *page_id,
                         GtkWidget *tools_menu, 
			 gboolean can_remove)
{
	GtkWidget *item;
	GPdfSidebarPage *page;

	page = g_new0 (GPdfSidebarPage, 1);	
	page->id = g_strdup (page_id);
	page->title = g_strdup (title);
        page->tools_menu = tools_menu; 
	page->can_remove = can_remove;

        item = gtk_menu_item_new_with_label (title); 
	g_object_set_data (G_OBJECT(item), "page", (gpointer)page);
	g_signal_connect (G_OBJECT(item), "activate",
			  G_CALLBACK (title_menu_item_activated_cb),
			  sidebar);	
	gtk_menu_shell_append (GTK_MENU_SHELL(sidebar->priv->title_menu),
                               item);
        
        gtk_widget_show (item);

	page->item = item;
	
	sidebar->priv->pages = g_list_append (sidebar->priv->pages,
		       			      (gpointer)page);
}

static gint 
page_compare_func (gconstpointer  a,
                   gconstpointer  b)
{
	GPdfSidebarPage *page = (GPdfSidebarPage *)a;

	return strcmp (page->id, (char*) b);
}

static GPdfSidebarPage *
gpdf_sidebar_find_page_by_id (GPdfSidebar *sidebar,
                              const char *page_id)
{
	GList *l;
	
	l = g_list_find_custom (sidebar->priv->pages, page_id,
				page_compare_func);
	if (!l) return NULL;
	
	return (GPdfSidebarPage *)l->data;
}

gboolean 
gpdf_sidebar_remove_page (GPdfSidebar *sidebar,
                          const char *page_id)
{
	GPdfSidebarPage *page, *new_page;
	GList *l;

	page = gpdf_sidebar_find_page_by_id (sidebar, page_id);
	g_return_val_if_fail (page != NULL, FALSE);

	sidebar->priv->pages = g_list_remove (sidebar->priv->pages,
		       			      (gpointer)page);
	
	l = sidebar->priv->pages;
	new_page = (GPdfSidebarPage *)l->data ?
	           (GPdfSidebarPage *)l->data : NULL;

	if (page)
	{
		gpdf_sidebar_select_page (sidebar, new_page->id);
	}

	gtk_widget_destroy (page->item);
	
	return TRUE;
}

gboolean 
gpdf_sidebar_select_page (GPdfSidebar *sidebar,
                          const char *page_id)
{
	GPdfSidebarPage *page;

	page = gpdf_sidebar_find_page_by_id (sidebar, page_id);
	g_return_val_if_fail (page != NULL, FALSE);
	
	select_page (sidebar, page);
	
	return FALSE;
}

void 
gpdf_sidebar_set_content (GPdfSidebar *sidebar,
                          GObject *content)
{
	if (sidebar->priv->content == content) return; 

	if (GTK_IS_WIDGET(sidebar->priv->content))
	{
		gtk_container_remove(GTK_CONTAINER(sidebar->priv->content_frame),
				     GTK_WIDGET(sidebar->priv->content));
	}

	sidebar->priv->content = content;

	if (GTK_IS_WIDGET(content))
	{
		gtk_container_add (GTK_CONTAINER(sidebar->priv->content_frame),
				   GTK_WIDGET(content));
	}
}

const gchar *
gpdf_sidebar_get_current_page_id (GPdfSidebar *sidebar)
{
        return (sidebar->priv->current && sidebar->priv->current->id ? 
                (const gchar *)sidebar->priv->current->id : NULL); 
}

GtkWidget *
gpdf_sidebar_set_page_tools_menu (GPdfSidebar *sidebar,
                                  const char *page_id,
                                  GtkWidget *tools_menu)
{
	GPdfSidebarPage *page;
        GtkWidget *old_menu; 

	page = gpdf_sidebar_find_page_by_id (sidebar, page_id);
	g_return_val_if_fail (page != NULL, NULL);

        old_menu = page->tools_menu;
        page->tools_menu = tools_menu;

        set_tools_button_sensitivity (sidebar, page);
        
        return old_menu; 
}
