/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Iain Holmes <iain@prettypeople.org>
 *
 *  Copyright 2002 Iain Holmes
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public
 *  License as published by the Free Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef __GSR_WINDOW_H__
#define __GSR_WINDOW_H__

#include <bonobo/bonobo-window.h>

#define GSR_WINDOW_TYPE (gsr_window_get_type ())
#define GSR_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSR_WINDOW_TYPE, GSRWindow))
#define GSR_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GSR_WINDOW_TYPE, GSRWindowClass))
#define IS_GSR_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSR_WINDOW_TYPE))
#define IS_GSR_WINDOW_CLASS(klasS) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSRy_WINDOW_TYPE))

typedef struct _GSRWindow GSRWindow;
typedef struct _GSRWindowClass GSRWindowClass;
typedef struct _GSRWindowPrivate GSRWindowPrivate;

struct _GSRWindow {
        BonoboWindow app_parent;

        GSRWindowPrivate *priv;
};

struct _GSRWindowClass {
        BonoboWindowClass parent_class;
};

GType gsr_window_get_type (void);

#endif
