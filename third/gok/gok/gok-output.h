/* gok-output.h
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
* Copyright 2001,2002 University Of Toronto
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
*/

#ifndef __GOKOUTPUT_H__
#define __GOKOUTPUT_H__

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <gnome.h>
#include <cspi/spi.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
        OUTPUT_INVALID,
	OUTPUT_KEYSYM,
	OUTPUT_KEYCODE,
	OUTPUT_KEYSTRING,
	OUTPUT_EXEC,
	OUTPUT_INTERNAL
} GokOutputType;

/* an output */
typedef struct GokOutput{
	gint Type;
	AccessibleKeySynthType Flag;
	gchar* Name;
	struct GokOutput* pOutputNext;	
} GokOutput;

void gok_output_delete_all (GokOutput* pOutput);
GokOutput* gok_output_new (gint Type, gchar* Name, AccessibleKeySynthType Flag);
GokOutput* gok_output_new_from_xml (xmlNode* pNode);
void gok_output_send_to_system (GokOutput* pOutput, gboolean bNoWordCompletion);
int gok_output_get_keycode (GokOutput* output);
void gok_output_internal (gchar* action);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOKOUTPUT_H__ */
