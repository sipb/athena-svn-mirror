#ifndef __GWEATHER_DIALOG_H_
#define __GWEATHER_DIALOG_H_

/* $Id: gweather-dialog.h,v 1.1.1.1 2003-01-04 21:17:27 ghudson Exp $ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Main status dialog
 *
 */

G_BEGIN_DECLS

extern void gweather_dialog_create (GWeatherApplet *gw_applet);
extern void gweather_dialog_open (GWeatherApplet *gw_applet);
extern void gweather_dialog_close (GWeatherApplet *gw_applet);
extern void gweather_dialog_display_toggle (GWeatherApplet *gw_applet);
extern void gweather_dialog_update (GWeatherApplet *gw_applet);

G_END_DECLS

#endif /* __GWEATHER_DIALOG_H_ */

