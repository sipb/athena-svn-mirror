#ifndef __GDICT_APPLET_H_
#define __GDICT_APPLET_H_

/* $Id: gdict-applet.h,v 1.1.1.3 2003-01-04 21:13:19 ghudson Exp $ */

/*
 *  Papadimitriou Spiros <spapadim@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict panel applet
 *
 */
 
#include <panel-applet.h>

/* This structure contains the internal state of the gdict applet.
 * Everything the functions and signal handlers need to be able to
 * manipulate the applet should be contained within. */
typedef struct _GDictApplet {
	GtkWidget *applet_widget;
	GtkWidget *button_widget;
	GtkWidget *image;
	GtkWidget *vbox_widget;
	GtkWidget *entry_widget;
	GtkWidget *handlebox_widget;
	PanelAppletOrient orient;
	gboolean handle;
	gint panel_size;
} GDictApplet;



#endif /* __GDICT_APPLET_H_ */
