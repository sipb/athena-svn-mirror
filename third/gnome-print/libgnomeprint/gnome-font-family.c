#define _GNOME_FONT_FAMILY_C_

#include <libgnomeprint/gnome-font-family.h>
#include <libgnomeprint/gnome-font-private.h>

struct _GnomeFontFamily {
	GtkObject object;

	gchar * name;
	GList * stylelist;
	GHashTable * faces;
};

struct _GnomeFontFamilyClass {
	GtkObjectClass parent_class;
};

static void gnome_font_family_class_init (GnomeFontFamilyClass * klass);
static void gnome_font_family_init (GnomeFontFamily * family);

static void gnome_font_family_destroy (GtkObject * object);

static void gnome_font_family_generate_list (void);
static void gnome_font_family_add_font (const gchar * name);

static GHashTable * families = NULL;

static GtkObjectClass * parent_class = NULL;

GtkType
gnome_font_family_get_type (void) {
	static GtkType family_type = 0;
	if (!family_type) {
		GtkTypeInfo family_info = {
			"GnomeFontFamily",
			sizeof (GnomeFontFamily),
			sizeof (GnomeFontFamilyClass),
			(GtkClassInitFunc) gnome_font_family_class_init,
			(GtkObjectInitFunc) gnome_font_family_init,
			NULL, NULL,
			(GtkClassInitFunc) NULL,
		};
		family_type = gtk_type_unique (gtk_object_get_type (), &family_info);
	}
	return family_type;
}

static void
gnome_font_family_class_init (GnomeFontFamilyClass * klass)
{
	GtkObjectClass * object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	object_class->destroy = gnome_font_family_destroy;
}

static void
gnome_font_family_init (GnomeFontFamily * family)
{
}

static void
gnome_font_family_destroy (GtkObject * object)
{
	GnomeFontFamily * family;

	family = (GnomeFontFamily *) object;

	g_hash_table_remove (families, family->name);

	if (family->name) {
		g_free (family->name);
		family->name = NULL;
	}

	while (family->stylelist) {
		g_free (family->stylelist->data);
		family->stylelist = g_list_remove (family->stylelist, family->stylelist->data);
	}

	if (((GtkObjectClass *) parent_class)->destroy)
		(* ((GtkObjectClass *) parent_class)->destroy) (object);
}

GnomeFontFamily *
gnome_font_family_new (const gchar * name)
{
	GnomeFontFamily * family;

	g_return_val_if_fail (name != NULL, NULL);

	if (!families) gnome_font_family_generate_list ();
	g_assert (families);

	family = g_hash_table_lookup (families, name);

	if (family) gnome_font_family_ref (family);

	return family;
}

GList *
gnome_font_family_style_list (GnomeFontFamily * family)
{
	g_return_val_if_fail (family != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FAMILY (family), NULL);

	return family->stylelist;
}

void
gnome_font_family_style_list_free (GList * list)
{
}

GnomeFontFace *
gnome_font_family_get_face_by_stylename (GnomeFontFamily * family, const gchar * style)
{
	GnomeFontFace * face;

	g_return_val_if_fail (family != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FAMILY (family), NULL);
	g_return_val_if_fail (style != NULL, NULL);

	face = g_hash_table_lookup (family->faces, style);

	if (face) gnome_font_face_ref (face);

	return face;
}

/* This is crap */

static void
gnome_font_family_generate_list (void)
{
	GList * fontlist, * l;

	g_assert (!families);

	families = g_hash_table_new (g_str_hash, g_str_equal);

	fontlist = gnome_font_list ();

	for (l = fontlist; l != NULL; l = l->next) {
		gnome_font_family_add_font ((const gchar *) l->data);
	}

	gnome_font_list_free (fontlist);
}

static void
gnome_font_family_add_font (const gchar * name)
{
	GnomeFontFace * face;
	GnomeFontFamily * family;
	const gchar * stylename;

	face = gnome_font_face_new (name);
	g_return_if_fail (face != NULL);

	family = g_hash_table_lookup (families, face->private->familyname);

	if (!family) {
		family = gtk_type_new (GNOME_TYPE_FONT_FAMILY);
		family->name = g_strdup (face->private->familyname);
		family->faces = g_hash_table_new (g_str_hash, g_str_equal);
		g_hash_table_insert (families, family->name, family);
	}

	stylename = gnome_font_face_get_species_name (face);

	if (!g_hash_table_lookup (family->faces, stylename)) {
		g_hash_table_insert (family->faces, g_strdup (stylename), face);
		family->stylelist = g_list_prepend (family->stylelist, g_strdup (stylename));
	} else {
		gnome_font_face_unref (face);
	}
}





