/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef _HTMLHSPACE_H_
#define _HTMLHSPACE_H_

#include "htmlobject.h"

#define HTML_HSPACE(x) ((HTMLHSpace *)(x))
#define HTML_HSPACE_CLASS(x) ((HTMLHSpaceClass *)(x))

struct _HTMLHSpace {
	HTMLObject object;

	/* FIXME!!!  WHY!?!?!?  Shouldn't we use the one from HTMLObject???  */
	HTMLFont *font;
};

struct _HTMLHSpaceClass {
	HTMLObjectClass object_class;
};


extern HTMLHSpaceClass html_hspace_class;


void        html_hspace_type_init   (void);
void        html_hspace_class_init  (HTMLHSpaceClass *klass,
				     HTMLType         type,
				     guint            object_size);
void        html_hspace_init        (HTMLHSpace      *hspace,
				     HTMLHSpaceClass *klass,
				     HTMLFont        *font,
				     gboolean         hidden);
HTMLObject *html_hspace_new         (HTMLFont        *font,
				     gboolean         hidden);

#endif /* _HTMLHSPACE_H_ */
