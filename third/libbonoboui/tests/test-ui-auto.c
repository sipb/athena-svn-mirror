/*
 * test-ui-auto.c: An automatic Bonobo UI api regression test
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2002 Ximian, Inc.
 */
#include <config.h>
#include <string.h>

#include <libbonoboui.h>
#include <bonobo/bonobo-ui-private.h>
#include <bonobo/bonobo-ui-node-private.h>
#include <bonobo/bonobo-ui-engine-private.h>

static const char *typical_pixbuf =
	"000000130000000eA"
	"0b0c0c000d0e0e001c1b1b00141213020f0e0e04141b1b15111717581319195e171e1e34"
	"171c1c16080a0a0b060606180a0d0d660d1414910a0f0f71070b0b510406062700000005"
	"000000001d1f1f09050606190c0c0c1c1d1c1d362626265d3c444490607c7ae5587573fc"
	"3b4f4fe71b2424be121818990f1616a42a3e3eea507878ff395858fe1e2f30f90c1314c1"
	"03050532000101011a1c1c4d131717b02e3939b63f4746db636766ff909292ff929696ff"
	"848d8dff788888ff6f8887ff617c7bff4b6565ff547979ff4b7473ff467271ff477677ff"
	"264041f20b13146b030707070f12125f3c4a4af5a0cac9ff97bcbbff9cbab9ffaabdbcff"
	"b5c1c0ffb8c0c0ffb6bbbbffafb1b1ff9f9f9fff868989ff707878ff5e6b6bff516464ff"
	"3e5c5cff233f3fff0f1c1c90070f0f0c0f13131a303e3dc096c4c3ff8cbbbaff83b1b0ff"
	"7eacaaff79a5a4ff7ca3a2ff8ba8a8ff9baeaeffa7b5b5ffa8b2b2ffa2a6a6ffa8a7a7ff"
	"a19d9dff6e7575fe2b4344fa0e202073050d0d08030404082a36366e7ea3a2fc8cbbb9ff"
	"7ca8a7ff76a3a2ff709e9dff6b9998ff659392ff5b8989ff578383ff588080ff708888ff"
	"b8bdbdffd2d0d0ff919495ff304142e30818194900050503030404011e2626324a5f5fe8"
	"89b7b5ff76a2a1ff709d9cff6a9796ff659190ff5e898aff558181ff4e7b7bff467474ff"
	"486f6fff9fadadffd3d2d2ff797f80ff253132b5010a0b2500000000000000000101011b"
	"293635be729998ff6f9c9bff6a9695ff64908fff5e8a8aff578383ff4f7a7bff497474ff"
	"416d6dff396263ff798b8cffafaeaeff575d5ef81b22237e0000000e00000000000000ff"
	"0203030d1e2626864c6868ff689695ff628e8eff5c8888ff568281ff4f7b7bff477273ff"
	"416c6dff3a6566ff315b5cff566c6cff7c7e7dff3e4040da1014144c0000000300000000"
	"00000000050404040c1010572f4242f85d8786ff567f7fff517c7bff4c7877ff467272ff"
	"3f6a6bff396465ff335e5fff2a5657ff395353ff4d5050fe262626b60304043400000005"
	"000000010000000004040400040505301d2929c93a5656fd3e6060ff3e6362ff3d6363ff"
	"3a6162ff355e5fff315a5bff2c5657ff255051ff2b4546ff2e3131f7121111a90000004a"
	"0000001f0000000800000000000000000405050a11181852182525b51c2d2de41e3131f8"
	"203636fe233c3dff234243ff234445ff214647ff1e4547ff223b3bff181b1bee040303a9"
	"0000007300000047000000170000000000000000000101000a0f0f090b1212210a101047"
	"070c0c7c060b0ba6080f0fc90b1616eb0d1c1cfe0f2222fe102929fe142727ff0b0f0fe3"
	"000000a00000007f000000570000001d0000000000000000000000000000000000000000"
	"000000040000000b00000011010202290204035b02040480020505ac020808c8050b0bc3"
	"030505940000005f0000004a000000300000000f";

static void
test_ui_node_attrs (void)
{
	GQuark baa_id;
	BonoboUINode *node;

	fprintf (stderr, "  attrs ...\n");

	node = bonobo_ui_node_new ("foo");
	g_assert (node != NULL);
	g_assert (bonobo_ui_node_has_name (node, "foo"));

	baa_id = g_quark_from_static_string ("baa");
	g_assert ( bonobo_ui_node_try_set_attr (node, baa_id, "baz"));
	g_assert (!bonobo_ui_node_try_set_attr (node, baa_id, "baz"));

	g_assert (!strcmp (bonobo_ui_node_get_attr_by_id (node, baa_id), "baz"));

	bonobo_ui_node_set_attr (node, "A", "A");
	bonobo_ui_node_set_attr (node, "A", "B");
	g_assert (!strcmp (bonobo_ui_node_peek_attr (node, "A"), "B"));

	bonobo_ui_node_free (node);
}

static void
test_ui_node_inserts (void)
{
	BonoboUINode *parent, *a, *b;

	fprintf (stderr, "  inserts ...\n");

	parent = bonobo_ui_node_new ("parent");

	a = bonobo_ui_node_new_child (parent, "a");
	g_assert (a->prev == NULL);
	g_assert (a->next == NULL);
	g_assert (a->parent == parent);

	b = bonobo_ui_node_new ("b");
	g_assert (b->prev == NULL);
	g_assert (b->next == NULL);

	bonobo_ui_node_insert_before (a, b);

	g_assert (b->prev == NULL);
	g_assert (b->next == a);
	g_assert (b->parent == parent);
	g_assert (a->prev == b);
	g_assert (a->next == NULL);
	g_assert (a->parent == parent);

	bonobo_ui_node_free (parent);
}

static void
test_xml_roundtrip (const char *txt)
{
	char *result;
	BonoboUINode *node;

	node = bonobo_ui_node_from_string (txt);
	g_assert (node != NULL);

	result = bonobo_ui_node_to_string (node, TRUE);
	bonobo_ui_node_free (node);

	if (strcmp (result, txt))
		g_error ("'%s' => '%s'", txt, result);

	bonobo_ui_node_free_string (result);
}

static void
test_ui_node_parsing (void)
{
	fprintf (stderr, "  parsing ...\n");

	test_xml_roundtrip ("<item/>\n");
	test_xml_roundtrip ("<item name=\"Foo&quot;\"/>\n");
	test_xml_roundtrip ("<item name=\"Foo&amp;\"/>\n");
	test_xml_roundtrip ("<item name=\"Foo&apos;\"/>\n");
}

static void
test_ui_node (void)
{
	fprintf (stderr, "testing BonoboUINode ...\n");

	test_ui_node_attrs ();
	test_ui_node_inserts ();
	test_ui_node_parsing ();
}

static void
check_prop (BonoboUIEngine *engine,
	    const char     *path,
	    const char     *value,
	    const char     *intended)
{
	CORBA_char *str;

	str = bonobo_ui_engine_xml_get_prop (engine, path, value, NULL);

	if (intended) {
		g_assert (str != NULL);
		g_assert (!strcmp (str, intended));
		CORBA_free (str);
	} else
		g_assert (str == NULL);
}

static void
test_engine_misc (CORBA_Environment *ev)
{
	BonoboUINode *node;
	BonoboUIEngine *engine;

	fprintf (stderr, "   misc ...\n");

	engine = bonobo_ui_engine_new (NULL);

	node = bonobo_ui_node_from_string (
		"<testnode name=\"Foo\" prop=\"A\"/>");

	bonobo_ui_engine_xml_merge_tree (engine, "/", node, "A");

	bonobo_ui_engine_xml_set_prop (engine, "/Foo", "prop", "B", "B");

	check_prop (engine, "/Foo", "prop", "B");

	bonobo_ui_engine_xml_rm (engine, "/", "B");

	check_prop (engine, "/Foo", "prop", "A");

	g_assert (bonobo_ui_engine_node_is_dirty (
		engine, bonobo_ui_engine_get_path (engine, "/Foo")));

	g_object_unref (G_OBJECT (engine));
}

static void
test_engine_container (CORBA_Environment *ev)
{
	BonoboUIEngine *engine;
	BonoboUIContainer *container;
	BonoboUIContainer *new_container;

	fprintf (stderr, "  UI container association ...\n");

	engine = bonobo_ui_engine_new (NULL);
	container = bonobo_ui_container_new ();
	new_container = bonobo_ui_container_new ();

	bonobo_ui_engine_set_ui_container (engine, container);
	g_assert (engine->priv->container == container);
	g_assert (bonobo_ui_container_get_engine (container) == engine);

	bonobo_ui_engine_set_ui_container (engine, new_container);
	g_assert (engine->priv->container == new_container);
	g_assert (bonobo_ui_container_get_engine (container) == NULL);
	g_assert (bonobo_ui_container_get_engine (new_container) == engine);

	bonobo_object_unref (new_container);
	bonobo_object_unref (container);
	g_object_unref (engine);
}

static void
test_engine_default_placeholder (CORBA_Environment *ev)
{
	BonoboUIEngine *engine;
	CORBA_char *str;
	BonoboUINode *node;

	fprintf (stderr, "  default placeholders ...\n");

	engine = bonobo_ui_engine_new (NULL);

	node = bonobo_ui_node_from_string (
		"<Root>"
		"  <nodea name=\"fooa\" attr=\"baa\"/>"
		"  <placeholder/>"
		"  <nodec name=\"fooc\" attr=\"baa\"/>"
		"</Root>");

	bonobo_ui_engine_xml_merge_tree (engine, "/", node, "A");

	node = bonobo_ui_node_from_string ("<nodeb name=\"foob\" attr=\"baa\"/>");
	bonobo_ui_engine_xml_merge_tree (engine, "/", node, "A");

	str = bonobo_ui_engine_xml_get (engine, "/", FALSE);
/*	g_warning ("foo '%s'", str); */
	CORBA_free (str);

	node = bonobo_ui_engine_get_path (engine, "/fooa");
	g_assert (node != NULL);
	g_assert (node->name_id == g_quark_from_string ("nodea"));
	g_assert (node->next != NULL);
	node = node->next;
	g_assert (node->name_id == g_quark_from_string ("nodeb"));
	g_assert (node->next != NULL);
	node = node->next;
	g_assert (node->name_id == g_quark_from_string ("placeholder"));
	g_assert (node->next != NULL);
	node = node->next;
	g_assert (node->name_id == g_quark_from_string ("nodec"));
	g_assert (node->next == NULL);
	
	g_object_unref (engine);
}

static void
test_ui_engine (CORBA_Environment *ev)
{
	fprintf (stderr, "testing BonoboUIEngine ...\n");

	test_engine_misc (ev);
	test_engine_container (ev);
	test_engine_default_placeholder (ev);
}

static void
test_ui_performance (CORBA_Environment *ev)
{
	int i;
	GTimer *timer;
	BonoboUINode *node;
	BonoboUIEngine *engine;

	fprintf (stderr, "performance tests ...\n");

	timer = g_timer_new ();
	g_timer_start (timer);

	engine = bonobo_ui_engine_new (NULL);

        node = bonobo_ui_node_from_file (BONOBO_TOPSRCDIR "/doc/std-ui.xml");
	if (!node)
		g_error ("Can't find std-ui.xml");

	bonobo_ui_engine_xml_merge_tree (engine, "/", node, "A");

	g_timer_reset (timer);
	for (i = 0; i < 10000; i++)
		bonobo_ui_engine_xml_set_prop (
			engine, "/menu/File/FileOpen",
			"hidden", (i / 3) % 2 ? "1" : "0", "A");

	fprintf (stderr, "  set prop item: %g(ns)\n",
		 g_timer_elapsed (timer, NULL) * 100);


	g_timer_reset (timer);
	for (i = 0; i < 10000; i++)
		bonobo_ui_engine_xml_set_prop (
			engine, "/menu/File/FileOpen",
			"hidden", (i / 3) % 2 ? "1" : "0", (i/6) % 2 ? "A" : "B");
	fprintf (stderr, "  set prop cmd override: %g(ns)\n",
		 g_timer_elapsed (timer, NULL) * 100);


	g_timer_reset (timer);
	for (i = 0; i < 1000; i++) {
		char *str = g_strdup_printf (
			"<cmd name=\"MenuGoHistoryPlaceholder%d\"/>", i % 13);
		node = bonobo_ui_node_from_string (str);
		bonobo_ui_engine_xml_merge_tree (engine, "/commands", node, "A");
		g_free (str);
	}
	fprintf (stderr, "  merge command: %g(ns)\n",
		 g_timer_elapsed (timer, NULL) * 1000);


	g_timer_reset (timer);
	for (i = 0; i < 1000; i++) {
		char *str = g_strdup_printf (
			"<menuitem name=\"%d\" verb=\"MenuGoHistoryPlaceholder%d\" "
			"pixtype=\"pixbuf\" pixname=\"%s\"/>",
			i % 13, i % 13, typical_pixbuf);
		node = bonobo_ui_node_from_string (str);
		bonobo_ui_engine_xml_merge_tree (engine, "/menu", node, "A");
		g_free (str);
	}
	fprintf (stderr, "  merge pixbuf: %g(ns)\n",
		 g_timer_elapsed (timer, NULL) * 1000);

	g_timer_reset (timer);
	for (i = 0; i < 10000; i++) {
		BonoboUINode *node;
		char *path = g_strdup_printf ("/menu/%d", i % 14);
		node = bonobo_ui_engine_get_path (engine, path);
		g_free (path);
	}
	fprintf (stderr, "  path lookup: %g(ns)\n",
		 g_timer_elapsed (timer, NULL) * 100);
	
	g_object_unref (engine);
}

int
main (int argc, char **argv)
{
	CORBA_Environment *ev, real_ev;

	ev = &real_ev;
	CORBA_exception_init (ev);

	free (malloc (8)); /* -lefence */

	gnome_program_init ("test-ui-uao", VERSION,
			    LIBBONOBOUI_MODULE,
			    argc, argv, NULL);

	bonobo_activate ();

	test_ui_node ();
	test_ui_engine (ev);
	test_ui_performance (ev);

	CORBA_exception_free (ev);

	fprintf (stderr, "All tests passed successfully\n");

	return bonobo_ui_debug_shutdown ();
}
