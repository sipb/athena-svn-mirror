/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * ggv-window.c: everything describing a single ggv window
 *
 * Copyright (C) 2002 the Free Software Foundation
 *
 * Author: Jaka Mocnik  <jaka@gnu.org>
 */

#ifndef __GGV_WINDOW_H__
#define __GGV_WINDOW_H__

#include <config.h>

#include <gnome.h>
#include <bonobo-activation/bonobo-activation.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gconf/gconf-client.h>
#include <bonobo.h>
#include <bonobo/bonobo-ui-main.h>

#include <math.h>
#include <ctype.h>

#include <Ggv.h>

#include "gtkgs.h"

G_BEGIN_DECLS 

#define GGV_TYPE_WINDOW            (ggv_window_get_type ())
#define GGV_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGV_TYPE_WINDOW, GgvWindow))
#define GGV_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GGV_TYPE_WINDOW, GgvWindowClass))
#define GGV_IS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGV_TYPE_WINDOW))
#define GGV_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGV_TYPE_WINDOW))
#define GGV_WINDOW_GET_CLASS(o)    (G_TYPE_INSTANCE_GET_CLASS ((o), GGV_TYPE_WINDOW, GgvWindowClass))

typedef struct _GgvWindow      GgvWindow;
typedef struct _GgvWindowClass GgvWindowClass;

struct _GgvWindow 
{
        BonoboWindow win;

        Bonobo_Control control;
        Bonobo_PropertyBag pb;

        BonoboControlFrame *ctlframe;
        BonoboUIComponent *uic, *popup_uic;

        GtkWidget *hbox, *vbox;
        GtkWidget *statusbar;
        GtkWidget *popup_menu;

        gboolean show_menus, show_sidebar, show_toolbar, show_statusbar;

        gboolean fullscreen;
        gint orig_x, orig_y, orig_width, orig_height;
        gboolean orig_ss, orig_sss, orig_sm, orig_st;

        gboolean loaded;
        gboolean pan;
        gboolean pane_auto_jump;	/* ...to top of new page */
        gdouble prev_x, prev_y;
        gfloat xcoord, ycoord;

        gchar **uris_to_open;
        gchar *filename;

        gint current_page;

        /* these are values from ggv control: we keep a copy here that
           we update on any change (mostly via the control's property
           bag) in order to use CORBA to communicate with the control
           as little as possible... */
        CORBA_float doc_width, doc_height;
        GNOME_GGV_Orientation doc_orient;
        CORBA_long page, page_count;
};

struct _GgvWindowClass
{
        BonoboWindowClass klass;
};

GType     ggv_window_get_type           (void);
GtkWidget *ggv_window_new               (void);
void      ggv_window_close              (GgvWindow *win);
gboolean  ggv_window_load               (GgvWindow *win, const gchar *filename);
void      ggv_window_apply_prefs        (GgvWindow *win);
void      ggv_window_sync_toolbar_style (GgvWindow *win);
const GList *ggv_get_window_list(void);
void      ggv_window_destroy_all(void);

G_END_DECLS

#endif /* __GGV_WINDOW_H__ */
