/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
   Copyright (C) 2000 Helix Code, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Radek Doulik <rodo@helixcode.com>

*/

#include "htmlenumutils.h"

const gchar *
html_valign_name (HTMLVAlignType va)
{
	switch (va) {
	case HTML_VALIGN_TOP:
		return "top";
	case HTML_VALIGN_MIDDLE:
		return "middle";
	case HTML_VALIGN_BOTTOM:
		return "bottom";
	case HTML_VALIGN_NONE:
		return "none";
	default:
		return "unknown";
	}
}


const gchar *
html_halign_name (HTMLHAlignType ha)
{
	switch (ha) {
	case HTML_HALIGN_LEFT:
		return "left";
	case HTML_HALIGN_CENTER:
		return "center";
	case HTML_HALIGN_RIGHT:
		return "right";
	case HTML_HALIGN_NONE:
		return "none";
	default:
		return "unknown";
	}
}
