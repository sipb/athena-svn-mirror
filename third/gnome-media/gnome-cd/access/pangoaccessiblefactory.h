/*
 * Copyright 2002 Sun Microsystems Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#ifndef __PANGO_ACCESSIBLE_FACTORY_H__
#define __PANGO_ACCESSIBLE_FACTORY_H__

#include <atk/atkobjectfactory.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PANGO_TYPE_ACCESSIBLE_FACTORY                    (pango_accessible_factory_get_type ())
#define PANGO_ACCESSIBLE_FACTORY(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), PANGO_TYPE_ACCESSIBLE_FACTORY, PangoAccessibleFactory))
#define PANGO_ACCESSIBLE_FACTORY_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), PANGO_TYPE_ACCESSIBLE_FACTORY, PangoAccessibleFactoryClass))
#define IS_PANGO_ACCESSIBLE_FACTORY(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PANGO_TYPE_ACCESSIBLE_FACTORY))
#define IS_PANGO_ACCESSIBLE_FACTORY_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), PANGO_TYPE_ACCESSIBLE_FACTORY))
#define PANGO_ACCESSIBLE_FACTORY_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), PANGO_TYPE_ACCESSIBLE_FACTORY, PangoAccessibleFactoryClass))

typedef struct _PangoAccessibleFactory              PangoAccessibleFactory;
typedef struct _PangoAccessibleFactoryClass         PangoAccessibleFactoryClass;

struct _PangoAccessibleFactory
{
	AtkObjectFactory parent;
};

struct _PangoAccessibleFactoryClass
{
	AtkObjectFactoryClass parent_class;
};

GType pango_accessible_factory_get_type();

AtkObjectFactory *pango_accessible_factory_new();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PANGO_ACCESSIBLE_FACTORY_H__ */

