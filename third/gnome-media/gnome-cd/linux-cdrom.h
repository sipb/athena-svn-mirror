/*
 * linux-cdrom.h
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes  <iain@ximian.com>
 */

#ifndef __LINUX_CDROM_H__
#define __LINUX_CDROM_H__

#include <glib/gerror.h>
#include <glib-object.h>

#include "cdrom.h"
G_BEGIN_DECLS

#define LINUX_CDROM_TYPE (linux_cdrom_get_type ())
#define LINUX_CDROM(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), LINUX_CDROM_TYPE, LinuxCDRom))
#define LINUX_CDROM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), LINUX_CDROM_TYPE, LinuxCDRomClass))
#define IS_LINUX_CDROM(obj) (GTK_TYPE_CHECK_INSTANCE_TYPE ((obj), LINUX_CDROM_TYPE))
#define IS_LINUX_CDROM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LINUX_CDROM_TYPE))
#define LINUX_CDROM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), LINUX_CDROM_TYPE, LinuxCDRomClass))

typedef struct _LinuxCDRom LinuxCDRom;
typedef struct _LinuxCDRomPrivate LinuxCDRomPrivate;
typedef struct _LinuxCDRomClass LinuxCDRomClass;

struct _LinuxCDRom {
	GnomeCDRom cdrom;

	LinuxCDRomPrivate *priv;
};

struct _LinuxCDRomClass {
	GnomeCDRomClass klass;
};

GType linux_cdrom_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif
