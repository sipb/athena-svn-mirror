#include "htmlglobalatoms.h"

gint
main (gint argc, gchar **argv)
{
	HtmlAtomList *al = html_atom_list_new ();
	
	html_global_atoms_initialize (al);

	g_print ("Display is: %d %d\n", HTML_ATOM_DISPLAY, html_atom_list_get_atom (al, "display"));

	g_print ("Embed is: %d %d\n", HTML_ATOM_EMBED, html_atom_list_get_atom (al, "embed"));
	
	g_print ("Foo is: %d\n", html_atom_list_get_atom (al, "foo"));
	g_print ("Bar is: %d\n", html_atom_list_get_atom (al, "bar"));
	g_print ("Foo is: %d\n", html_atom_list_get_atom (al, "foo"));
	return 0;
}
