/*  This file is part of the GtkHTML library.
 *
 *  Copyright 2002 Ximian, Inc.
 *
 *  Author: Radek Doulik
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef __HTML_A11Y_TEXT_H__
#define __HTML_A11Y_TEXT_H__

#include <libgail-util/gail-util.h>
#include "html.h"

#define G_TYPE_HTML_A11Y_TEXT            (html_a11y_text_get_type ())
#define HTML_A11Y_TEXT(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
									   G_TYPE_HTML_A11Y_TEXT, \
									   HTMLA11YText))
#define HTML_A11Y_TEXT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), \
									G_TYPE_HTML_A11Y_TEXT, \
									HTMLA11YTextClass))
#define G_IS_HTML_A11Y_TEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_HTML_A11Y_TEXT))
#define G_IS_HTML_A11Y_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), G_TYPE_HTML_A11Y_TEXT))
#define HTML_A11Y_TEXT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_HTML_A11Y_TEXT, \
									  HTMLA11YTextClass))

typedef struct _HTMLA11YText      HTMLA11YText;
typedef struct _HTMLA11YTextClass HTMLA11YTextClass;

struct _HTMLA11YText {
	HTMLA11Y html_a11y_object;

	GailTextUtil *util;
};

GType html_a11y_text_get_type (void);

struct _HTMLA11YTextClass {
	HTMLA11YClass parent_class;
};

AtkObject* html_a11y_text_new (HTMLObject *o);

#endif
