/* gnome_access - accessibility support for gnome-libs
 * Copyright 2001 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __ZVT_ACCESSIBLE_FACTORY_H__
#define __ZVT_ACCESSIBLE_FACTORY_H__

#include <atk/atkobjectfactory.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ZVT_TYPE_ACCESSIBLE_FACTORY                    (zvt_accessible_factory_get_type ())
#define ZVT_ACCESSIBLE_FACTORY(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), ZVT_TYPE_ACCESSIBLE_FACTORY, ZvtAccessibleFactory))
#define ZVT_ACCESSIBLE_FACTORY_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), ZVT_TYPE_ACCESSIBLE_FACTORY, ZvtZccessibleFactoryClass))
#define ZVT_IS_ACCESSIBLE_FACTORY(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ZVT_TYPE_ACCESSIBLE_FACTORY))
#define ZVT_IS_ACCESSIBLE_FACTORY_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), ZVT_TYPE_ACCESSIBLE_FACTORY))
#define ZVT_ACCESSIBLE_FACTORY_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), ZVT_TYPE_ACCESSIBLE_FACTORY, ZvtAccessibleFactoryClass))

typedef struct _ZvtAccessibleFactory ZvtAccessibleFactory;
typedef struct _ZvtAccessibleFactoryClass ZvtAccessibleFactoryClass;

struct _ZvtAccessibleFactory
{
  AtkObjectFactory parent;
};

struct _ZvtAccessibleFactoryClass
{
  AtkObjectFactoryClass parent_class;
};

GType
zvt_accessible_factory_get_type(void);

AtkObjectFactory *
zvt_accessible_factory_new(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __ZVT_ACCESSIBLE_FACTORY_H__ */

