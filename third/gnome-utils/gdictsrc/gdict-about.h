#ifndef __GDICT_ABOUT_H_
#define __GDICT_ABOUT_H_

/* $Id: gdict-about.h,v 1.1.1.3 2004-10-04 05:06:06 ghudson Exp $ */

#include "gtk/gtkwindow.h"
/*
 *  Papadimitriou Spiros <spapadim@cs.cmu.edu>
 *  Mike Hughes <mfh@psilord.com>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict About box
 *
 */

GtkWidget *gdict_about_new (void);
extern void gdict_about (GtkWindow *parent);

#endif /* __GDICT_ABOUT_H_ */

