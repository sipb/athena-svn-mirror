/*
 * Copyright (C) 2003 Sun Microsystems, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Authors:
 *    Mark McLoughlin <mark@skynet.ie>
 */

#ifndef __NETSTATUS_APPLET_H__
#define __NETSTATUS_APPLET_H__

#include <panel-applet.h>

G_BEGIN_DECLS

#define NETSTATUS_TYPE_APPLET         (netstatus_applet_get_type ())
#define NETSTATUS_APPLET(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), NETSTATUS_TYPE_APPLET, NetstatusApplet))
#define NETSTATUS_APPLET_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), NETSTATUS_TYPE_APPLET, NetstatusAppletClass))
#define NETSTATUS_IS_APPLET(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), NETSTATUS_TYPE_APPLET))
#define NETSTATUS_IS_APPLET_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), NETSTATUS_TYPE_APPLET))
#define NETSTATUS_APPLET_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), NETSTATUS_TYPE_APPLET, NetstatusAppletClass))

typedef struct _NetstatusApplet        NetstatusApplet;
typedef struct _NetstatusAppletClass   NetstatusAppletClass;
typedef struct _NetstatusAppletPrivate NetstatusAppletPrivate;

struct _NetstatusApplet
{
  PanelApplet              parent_instance; 
	
  NetstatusAppletPrivate  *priv;
};

struct _NetstatusAppletClass
{
  PanelAppletClass parent_class;
};

GType netstatus_applet_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __NETSTATUS_APPLET_H__ */
