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

#ifndef __CDDISPLAY_ACCESSIBLE_FACTORY_H__
#define __CDDISPLAY_ACCESSIBLE_FACTORY_H__

#include <atk/atkobjectfactory.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CDDISPLAY_TYPE_ACCESSIBLE_FACTORY                    (cddisplay_accessible_factory_get_type ())
#define CDDISPLAY_ACCESSIBLE_FACTORY(obj)                    (G_TYPE_CHECK_INSTANCE_CAST ((obj), CDDISPLAY_TYPE_ACCESSIBLE_FACTORY, CDDisplayAccessibleFactory))
#define CDDISPLAY_ACCESSIBLE_FACTORY_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), CDDISPLAY_TYPE_ACCESSIBLE_FACTORY, CDDisplayAccessibleClassFactory))
#define IS_CDDISPLAY_ACCESSIBLE_FACTORY(obj)                 (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CDDISPLAY_TYPE_ACCESSIBLE_FACTORY))
#define IS_CDDISPLAY_ACCESSIBLE_FACTORY_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), CDDISPLAY_TYPE_ACCESSIBLE_FACTORY))
#define CDDISPLAY_ACCESSIBLE_FACTORY_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), CDDISPLAY_TYPE_ACCESSIBLE_FACTORY, CDDisplayAccessibleFactoryClass))

typedef struct _CDDisplayAccessibleFactory        CDDisplayAccessibleFactory;
typedef struct _CDDisplayAccessibleFactoryClass CDDisplayAccessibleFactoryClass;

struct _CDDisplayAccessibleFactory
{
	AtkObjectFactory parent;
};

struct _CDDisplayAccessibleFactoryClass
{
	AtkObjectFactoryClass parent_class;
};

GType cddisplay_accessible_factory_get_type(void);

AtkObjectFactory *cddisplay_accessible_factory_new(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CDDISPLAY_ACCESSIBLE_FACTORY_H__ */

