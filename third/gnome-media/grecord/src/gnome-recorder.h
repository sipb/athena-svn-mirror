/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Iain Holmes <iain@prettypeople.org>
 *
 *  Copyright 2002 Iain Holmes
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public
 *  License as published by the Free Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef __GNOME_RECORDER_H__
#define __GNOME_RECORDER_H__

#include "gsr-window.h"

#define GSR_STOCK_PLAY "media-play"
#define GSR_STOCK_STOP "media-stop"
#define GSR_STOCK_RECORD "media-record"

typedef void (*GSRForeachFunction) (GSRWindow *window,
				    gpointer closure);

#endif
