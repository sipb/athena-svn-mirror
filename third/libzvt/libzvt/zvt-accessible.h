/* zvt_access-- acccessibility support for libzvt
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

#ifndef __ZVT_ACCESSIBLE_H__
#define __ZVT_ACCESSIBLE_H__

#include <gtk/gtkaccessible.h>
#include <libzvt/vt.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ZVT_TYPE_ACCESSIBLE                     (zvt_accessible_get_type ())
#define ZVT_ACCESSIBLE(obj)                     (G_TYPE_CHECK_INSTANCE_CAST ((obj), ZVT_TYPE_ACCESSIBLE, ZvtAccessible))
#define ZVT_ACCESSIBLE_CLASS(klass)             (G_TYPE_CHECK_CLASS_CAST ((klass), ZVT_TYPE_ACCESSIBLE, ZvtAccessibleClass))
#define ZVT_IS_ACCESSIBLE(obj)                  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ZVT_TYPE_ACCESSIBLE))
#define ZVT_IS_ACCESSIBLE_CLASS(klass)          (G_TYPE_CHECK_CLASS_TYPE ((klass), ZvtAccessible))
#define ZVT_ACCESSIBLE_GET_CLASS(obj)           (G_TYPE_INSTANCE_GET_CLASS ((obj), ZVT_TYPE_ACCESSIBLE, ZvtAccessibleClass))

typedef struct _ZvtAccessible ZvtAccessible;
typedef struct _ZvtAccessibleClass ZvtAccessibleClass;


struct _ZvtAccessible
{
  GtkAccessible parent;
};


GType
zvt_accessible_get_type (void);

struct _ZvtAccessibleClass
{
  GtkAccessibleClass parent_class;
};

AtkObject *zvt_accessible_new(GtkWidget *widget);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __ZVT_ACCESSIBLE_H__ */
