#ifndef __GWEATHER_APPLET_H_
#define __GWEATHER_APPLET_H_

/* $Id: gweather-applet.h,v 1.1.1.1 2003-01-04 21:17:51 ghudson Exp $ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Main applet widget
 *
 */

#include "weather.h"
#include "gweather.h"

G_BEGIN_DECLS

extern void gweather_applet_create(GWeatherApplet *gw_applet);
extern gint timeout_cb (gpointer data);
extern void gweather_update (GWeatherApplet *applet);
extern void gweather_info_load (const gchar *path, GWeatherApplet *applet);
extern void gweather_info_save (const gchar *path, GWeatherApplet *applet);

G_END_DECLS

#endif /* __GWEATHER_APPLET_H_ */
