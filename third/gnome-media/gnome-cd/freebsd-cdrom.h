/*
 * freebsd-cdrom.h
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes  <iain@ximian.com>
 */

#ifndef __FREEBSD_CDROM_H__
#define __FREEBSD_CDROM_H__

#include <glib/gerror.h>
#include <glib-object.h>

#include "cdrom.h"
G_BEGIN_DECLS

#define FREEBSD_CDROM_TYPE (freebsd_cdrom_get_type ())
#define FREEBSD_CDROM(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), FREEBSD_CDROM_TYPE, FreeBSDCDRom))
#define FREEBSD_CDROM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), FREEBSD_CDROM_TYPE, FreeBSDCDRomClass))
#define IS_FREEBSD_CDROM(obj) (GTK_TYPE_CHECK_INSTANCE_TYPE ((obj), FREEBSD_CDROM_TYPE))
#define IS_FREEBSD_CDROM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FREEBSD_CDROM_TYPE))
#define FREEBSD_CDROM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), FREEBSD_CDROM_TYPE, FreeBSDCDRomClass))

typedef struct _FreeBSDCDRom FreeBSDCDRom;
typedef struct _FreeBSDCDRomPrivate FreeBSDCDRomPrivate;
typedef struct _FreeBSDCDRomClass FreeBSDCDRomClass;

struct _FreeBSDCDRom {
	GnomeCDRom cdrom;

	FreeBSDCDRomPrivate *priv;
};

struct _FreeBSDCDRomClass {
	GnomeCDRomClass klass;
};

GType freebsd_cdrom_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif
