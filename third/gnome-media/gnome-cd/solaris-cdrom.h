/*
 * solaris-cdrom.h
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes  <iain@ximian.com>
 */

#ifndef __SOLARIS_CDROM_H__
#define __SOLARIS_CDROM_H__

#include <glib/gerror.h>
#include <glib-object.h>

#include "cdrom.h"
G_BEGIN_DECLS

#define SOLARIS_CDROM_TYPE (solaris_cdrom_get_type ())
#define SOLARIS_CDROM(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOLARIS_CDROM_TYPE, SolarisCDRom))
#define SOLARIS_CDROM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SOLARIS_CDROM_TYPE, SolarisCDRomClass))
#define IS_SOLARIS_CDROM(obj) (GTK_TYPE_CHECK_INSTANCE_TYPE ((obj), SOLARIS_CDROM_TYPE))
#define IS_SOLARIS_CDROM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SOLARIS_CDROM_TYPE))
#define SOLARIS_CDROM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), SOLARIS_CDROM_TYPE, SolarisCDRomClass))

#define CD_FRAMES 75

typedef struct _SolarisCDRom SolarisCDRom;
typedef struct _SolarisCDRomPrivate SolarisCDRomPrivate;
typedef struct _SolarisCDRomClass SolarisCDRomClass;

struct _SolarisCDRom {
	GnomeCDRom cdrom;

	SolarisCDRomPrivate *priv;
};

struct _SolarisCDRomClass {
	GnomeCDRomClass klass;
};

GType solaris_cdrom_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif
