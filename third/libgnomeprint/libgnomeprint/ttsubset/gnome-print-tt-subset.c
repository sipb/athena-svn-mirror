/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-print-tt-subset.c: Function for creating a subfont out of a
 *  ttf. The resultant font will be encoded in symbol format.
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

#include "sft.h"
#include "gnome-print-tt-subset.h"

void gnome_print_pdf_tt_create_subfont (const unsigned char *file_name, 
			unsigned char **subfont_file, 
			unsigned short *glyphArray, 
			unsigned char *encoding, unsigned short len)
{
    TrueTypeFont *fnt;
    int r;

    if ((r = OpenTTFont(file_name, 0, &fnt)) != SF_OK) {
        fprintf(stderr, "Error %d opening font file: `%s`.\n", r, file_name);
	return;
    }

    *subfont_file = tmpnam (NULL);

    CreateTTFromTTGlyphs(fnt, *subfont_file, glyphArray, encoding, len, 0, NULL, TTCF_AutoName | TTCF_IncludeOS2);

    CloseTTFont(fnt);

}

void
gnome_print_ps_tt_create_subfont (const unsigned char *file_name, 
			const unsigned char *encoded_font_name,
			unsigned char **subfont_file, 
			unsigned short *glyphArray, 
			unsigned char *encoding, unsigned short len)
{
    TrueTypeFont *fnt;
    FILE *subf;
    int r;

    if ((r = OpenTTFont(file_name, 0, &fnt)) != SF_OK) {
        fprintf(stderr, "Error %d opening font file: `%s`.\n", r, file_name);
	return;
    }

    *subfont_file = tmpnam (NULL);
    subf = fopen (*subfont_file, "wb");
    CreateT42FromTTGlyphs (fnt, subf, encoded_font_name, (uint16 *)glyphArray, (byte *)encoding, len);
    fclose (subf);

    CloseTTFont(fnt);

}
