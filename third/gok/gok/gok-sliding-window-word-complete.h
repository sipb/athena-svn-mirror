/* gok-utf8-word-complete.c
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*
**/

#ifndef __GOK_SLIDING_WINDOW_WORDCOMPLETE_H__
#define __GOK_SLIDING_WINDOW_WORDCOMPLETE_H__

#include "gok-utf8-word-complete.h"

#define GOK_TYPE_SW_WORDCOMPLETE           (gok_sw_wordcomplete_get_type ())
#define GOK_SW_WORDCOMPLETE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOK_TYPE_SW_WORDCOMPLETE, GokSWWordComplete))
#define GOK_SW_WORDCOMPLETE_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST (klass, GOK_TYPE_SW_WORDCOMPLETE, GokSWWordCompleteClass)
#define GOK_IS_SW_WORDCOMPLETE(obj)       G_TYPE_CHECK_INSTANCE_TYPE (obj, GOK_TYPE_SW_WORDCOMPLETE)
#define GOK_IS_SW_WORDCOMPLETE_CLASS(klass)       G_TYPE_CHECK_CLASS_TYPE (klass, GOK_TYPE_SW_WORDCOMPLETE)

typedef struct _GokSWWordComplete       GokSWWordComplete;
typedef struct _GokSWWordCompleteClass  GokSWWordCompleteClass;

struct _GokSWWordComplete
{
	GokUTF8WordComplete parent;
};

struct _GokSWWordCompleteClass
{
	GokUTF8WordCompleteClass parent_class;
};

GType gok_sw_wordcomplete_get_type (void);
#endif /* #ifndef _GOK_SLIDING_WINDOW_WORDCOMPLETE_H__ */
