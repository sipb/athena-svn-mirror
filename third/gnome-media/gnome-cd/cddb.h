/*
 * cddb.h
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes <iain@ximian.com>
 */

#ifndef __CDDB_H__
#define __CDDB_H__

#include "gnome-cd.h"

void cddb_get_query      (GnomeCD *gcd);
int  cddb_sum            (int n);
void cddb_close_client   (void);
void cddb_free_disc_info (GnomeCDDiscInfo *info);

#endif
