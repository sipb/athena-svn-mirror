/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Nautilus PDF Property Page
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

#ifndef NAUTILUS_PDF_PROPERTY_PAGE_H
#define NAUTILUS_PDF_PROPERTY_PAGE_H

#include "gpdf-g-switch.h"
#  include <glib.h>
#  include <bonobo/bonobo-control.h>
#include "gpdf-g-switch.h"

G_BEGIN_DECLS

#define GPDF_TYPE_NAUTILUS_PROPERTY_PAGE            (gpdf_nautilus_property_page_get_type ())
#define GPDF_NAUTILUS_PROPERTY_PAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_NAUTILUS_PROPERTY_PAGE, GPdfNautilusPropertyPage))
#define GPDF_NAUTILUS_PROPERTY_PAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_NAUTILUS_PROPERTY_PAGE, GPdfNautilusPropertyPageClass))
#define GPDF_IS_NAUTILUS_PROPERTY_PAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_NAUTILUS_PROPERTY_PAGE))
#define GPDF_IS_NAUTILUS_PROPERTY_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_NAUTILUS_PROPERTY_PAGE))

typedef struct _GPdfNautilusPropertyPage        GPdfNautilusPropertyPage;
typedef struct _GPdfNautilusPropertyPageClass   GPdfNautilusPropertyPageClass;
typedef struct _GPdfNautilusPropertyPagePrivate GPdfNautilusPropertyPagePrivate;

struct _GPdfNautilusPropertyPage {
        BonoboControl parent;
        
        GPdfNautilusPropertyPagePrivate *priv;
};

struct _GPdfNautilusPropertyPageClass {
        BonoboControlClass parent_class;
};

GType gpdf_nautilus_property_page_get_type ();

G_END_DECLS

#endif /* NAUTILUS_PDF_PROPERTY_PAGE_H */
