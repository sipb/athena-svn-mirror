/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-tt-subset.h: header file for gnome-print-tt-subset.c
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Suresh Chandrasekharan <suresh.chandrasekharan@sun.com>
 *
 *  Copyright (C) 2004 Sun Microsystems Inc.
 */

#ifndef __GNOME_PRINT_TT_SUBSET_H__
#define __GNOME_PRINT_TT_SUBSET_H__

void gnome_print_pdf_tt_create_subfont (const unsigned char *file_name, 
			unsigned char **subfont_file, 
			unsigned short *glyphArray, 
			unsigned char *encoding, unsigned short len);

void gnome_print_ps_tt_create_subfont (const unsigned char *file_name, 
			const unsigned char *encoded_font_name,
			unsigned char **subfont_file, 
			unsigned short *glyphArray, 
			unsigned char *encoding, unsigned short len);

#endif /* __GNOME_PRINT_TT_SUBSET_H__ */
