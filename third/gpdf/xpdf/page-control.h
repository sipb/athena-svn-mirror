/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Page number entry
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

#ifndef PAGE_CONTROL_H
#define PAGE_CONTROL_H

#include <gtk/gtkhbox.h>

G_BEGIN_DECLS

#define GPDF_TYPE_PAGE_CONTROL            (gpdf_page_control_get_type ())
#define GPDF_PAGE_CONTROL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_PAGE_CONTROL, GPdfPageControl))
#define GPDF_PAGE_CONTROL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_PAGE_CONTROL, GPdfPageControlClass))
#define GPDF_IS_PAGE_CONTROL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_PAGE_CONTROL))
#define GPDF_IS_PAGE_CONTROL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_PAGE_CONTROL))

typedef struct _GPdfPageControl        GPdfPageControl;
typedef struct _GPdfPageControlClass   GPdfPageControlClass;
typedef struct _GPdfPageControlPrivate GPdfPageControlPrivate;

struct _GPdfPageControl {
	GtkHBox parent;
        
        GPdfPageControlPrivate *priv;
};

struct _GPdfPageControlClass {
        GtkHBoxClass parent_class;
        
        void (*set_page) (GPdfPageControl *control, int page);
};

GType gpdf_page_control_get_type (void);
void  gpdf_page_control_set_page (GPdfPageControl *control, int page);
void  gpdf_page_control_set_total_pages (GPdfPageControl *control, int page);

G_END_DECLS

#endif /* PAGE_CONTROL_H */
