/* gok-word-complete.h
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

#ifndef __GOKWORDCOMPLETE_H__
#define __GOKWORDCOMPLETE_H__

#include <glib-object.h>
#include "gok-output.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
	GOK_WORDCOMPLETE_CHAR_TYPE_NORMAL,
	GOK_WORDCOMPLETE_CHAR_TYPE_BACKSPACE,
	GOK_WORDCOMPLETE_CHAR_TYPE_COMPOSE, /* prefix composition */
	GOK_WORDCOMPLETE_CHAR_TYPE_COMBINE, /* postfix composition */
	GOK_WORDCOMPLETE_CHAR_TYPE_DELIMITER
} GokWordCompletionCharacterCategory;

#define GOK_TYPE_WORDCOMPLETE           (gok_wordcomplete_get_type ())
#define GOK_WORDCOMPLETE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), GOK_TYPE_WORDCOMPLETE, GokWordComplete))
#define GOK_WORDCOMPLETE_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST (klass, GOK_TYPE_WORDCOMPLETE, GokWordCompleteClass)
#define GOK_IS_WORDCOMPLETE(obj)       G_TYPE_CHECK_INSTANCE_TYPE (obj, GOK_TYPE_WORDCOMPLETE)
#define GOK_IS_WORDCOMPLETE_CLASS(klass)       G_TYPE_CHECK_CLASS_TYPE (klass, GOK_TYPE_WORDCOMPLETE)
#define GOK_WORDCOMPLETE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GOK_TYPE_WORDCOMPLETE, GokWordCompleteClass))

typedef struct _GokWordComplete       GokWordComplete;
typedef struct _GokWordCompleteClass  GokWordCompleteClass;
typedef struct _GokWordCompletePrivate GokWordCompletePrivate;

struct _GokWordComplete
{
	GObject parent;
	GokWordCompletePrivate *priv;
};

struct _GokWordCompleteClass
{
	GObjectClass parent_class;

	/* public */
	gboolean  (* open)         (GokWordComplete *complete,
				    const gchar *directory);
	void      (* close)        (GokWordComplete *complete);
	gchar*    (* process_unichar) (GokWordComplete *complete, gunichar letter);
	void      (* reset)        (GokWordComplete *complete);
	gchar**   (* predict)      (GokWordComplete *complete, gint num_predictions);
	gchar**   (* predict_string) (GokWordComplete *complete, const gchar *utf8_string,
				      gint num_predictions);
	gboolean  (* add_new_word) (GokWordComplete *complete,
				    const gchar *utf8_word);
	gboolean  (* increment_word_frequency) (GokWordComplete *complete,
						const gchar *word);
	gboolean  (* validate_word) (GokWordComplete *complete,
				     const gchar *utf8_word);

	/* private use */
	gchar*    (* unichar_pop) (GokWordComplete *complete);
	gchar*    (* unichar_push) (GokWordComplete *complete, gunichar letter);
	GokWordCompletionCharacterCategory (* unichar_get_category) (GokWordComplete *complete, gunichar letter);
	gchar*    (* get_word_part) (GokWordComplete *complete);
        const gchar*    (* get_aux_dictionary) (GokWordComplete *complete);
	gboolean  (* set_aux_dictionary) (GokWordComplete *complete, gchar *dict_path);
	const gchar*    (* get_delimiter) (GokWordComplete *complete);
};

GType    gok_wordcomplete_get_type       (void);
gboolean gok_wordcomplete_open           (GokWordComplete *complete, gchar *directory);
void     gok_wordcomplete_close          (GokWordComplete *complete);
void     gok_wordcomplete_reset          (GokWordComplete *complete);
gchar**  gok_wordcomplete_predict        (GokWordComplete *complete, gint num_predictions);
gchar*   gok_wordcomplete_process_unichar(GokWordComplete *complete, const gunichar letter);
gchar**  gok_wordcomplete_predict_string (GokWordComplete *complete, const gchar *string, 
					  gint num_predictions);

gboolean gok_wordcomplete_add_new_word   (GokWordComplete *complete, const char* word);
gboolean gok_wordcomplete_validate_word  (GokWordComplete *complete, const char* word);

GokWordComplete *gok_wordcomplete_get_default (void);
gchar**  gok_wordcomplete_process_and_predict (GokWordComplete *complete, const gunichar letter, 
					       gint num_predictions);

gchar*   gok_wordcomplete_get_word_part (GokWordComplete *complete);
const gchar*   gok_wordcomplete_get_aux_dictionaries (GokWordComplete *complete);
const gchar*   gok_wordcomplete_get_delimiter  (GokWordComplete *complete);
gboolean gok_wordcomplete_set_aux_dictionaries (GokWordComplete *complete, gchar *dictionary_path);
gboolean gok_wordcomplete_increment_word_frequency (GokWordComplete *complete, const gchar* pWord);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOKWORDCOMPLETE_H__ */
