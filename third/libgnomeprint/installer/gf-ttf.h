#ifndef __GF_TTF_H__
#define __GF_TTF_H__

#include <glib.h>
#include "parseAFM.h"

G_BEGIN_DECLS

typedef struct _GFTTF GFTTF;

struct _GFTTF {
	gchar *filename;
	guint num_faces;
	GlobalFontInfo gfi;
};

GFTTF *gf_ttf_open (const gchar *name, gint face, const guchar *familyname, const guchar *stylename);

void gf_ttf_close (GFTTF *pfb);

G_END_DECLS

#endif
