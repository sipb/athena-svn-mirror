/**
 * bonobo-property-editor.c:
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */

#include <config.h>
#include <gtk/gtksignal.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-property-bag-client.h>
#include <liboaf/liboaf.h>

#include "bonobo-property-editor.h"

#define CLASS(o) BONOBO_PEDITOR_CLASS (GTK_OBJECT(o)->klass)

struct _BonoboPEditorPrivate {
	Bonobo_PropertyBag bag;
	Bonobo_Property    property;
	BonoboPEditorSetFn set_cb;
	Bonobo_EventSource_ListenerId  id;
	GtkWidget          *widget;
};

#define  TYPE_PRINT(tckind)			                        \
	case tckind:							\
                printf (#tckind "\n");           			\
                break;

void
print_typecode (CORBA_TypeCode tc, const gchar *name, int level)
{
        int i, j;

	for (i=0;i<level;i++) printf(" ");          
	if (name) printf ("(%s)", name);
	printf ("(%d)(%p)", ORBIT_ROOT_OBJECT(tc)->refs, tc);

	switch (tc->kind) {

		TYPE_PRINT(CORBA_tk_null);
		TYPE_PRINT(CORBA_tk_void);
		TYPE_PRINT(CORBA_tk_short);
		TYPE_PRINT(CORBA_tk_long);
		TYPE_PRINT(CORBA_tk_ushort);
		TYPE_PRINT(CORBA_tk_ulong);
		TYPE_PRINT(CORBA_tk_float);
		TYPE_PRINT(CORBA_tk_double);
		TYPE_PRINT(CORBA_tk_boolean);
		TYPE_PRINT(CORBA_tk_char);
		TYPE_PRINT(CORBA_tk_octet);
		TYPE_PRINT(CORBA_tk_string);
		TYPE_PRINT(CORBA_tk_longlong);
		TYPE_PRINT(CORBA_tk_ulonglong);
		TYPE_PRINT(CORBA_tk_longdouble);
		TYPE_PRINT(CORBA_tk_wchar);
		TYPE_PRINT(CORBA_tk_wstring);
		TYPE_PRINT(CORBA_tk_objref);
		TYPE_PRINT(CORBA_tk_any);
		TYPE_PRINT(CORBA_tk_TypeCode);
		
	case CORBA_tk_struct:
		for (i=0;i<level;i++) printf(" "); 
		printf ("CORBA_tk_struct %s\n", tc->repo_id);
		for (i = 0; i < tc->sub_parts; i++) {
			print_typecode (tc->subtypes [i], tc->subnames [i],
					level+2);
		}
		break;

	case CORBA_tk_sequence:
		for (i=0;i<level;i++) printf(" "); 
		printf ("CORBA_tk_sequence\n");
		print_typecode (tc->subtypes [0], NULL, level+2);
		break;

	case CORBA_tk_alias:
		for (i=0;i<level;i++) printf(" "); 
		printf ("CORBA_tk_alias %p %p %s\n", tc, tc->repo_id,
			tc->repo_id);
		print_typecode (tc->subtypes [0], NULL, level+2);
		break;

	case CORBA_tk_enum:
		for (i=0;i<level;i++) printf(" "); 
		printf ("CORBA_tk_enum %p %p %s\n", tc, tc->repo_id,
			tc->repo_id);
		for (j = 0; j < tc->sub_parts; j++) {
			for (i=0;i<level+2;i++) printf(" "); 
			printf ("%s\n", tc->subnames [j]);
		}
		break;

	default:
		for (i=0;i<level;i++) printf(" "); 
		printf ("Unknown Type\n");
	
	}

}

static void
int_set_value (BonoboPEditor     *editor, 
	       CORBA_any         *any,
	       CORBA_Environment *ev)
{
	CORBA_any nv;

	/* fixme: support nested aliases */

	if (any->_type->kind == CORBA_tk_alias) {

		nv._type =  any->_type->subtypes [0];
		nv._value = any->_value;

		if (editor->priv->set_cb)
			editor->priv->set_cb (editor, &nv, ev);
		else if (CLASS (editor)->set_value)
			CLASS (editor)->set_value (editor, &nv, ev);

	} else {
		if (editor->priv->set_cb)
			editor->priv->set_cb (editor, any, ev);
		else if (CLASS (editor)->set_value)
			CLASS (editor)->set_value (editor, any, ev);
	}
}

static void 
value_changed_cb (BonoboListener    *listener,
		  char              *event_name, 
		  CORBA_any         *any,
		  CORBA_Environment *ev,
		  gpointer           user_data)
{
	BonoboPEditor *editor = BONOBO_PEDITOR (user_data);
	
	if (!bonobo_arg_type_is_equal (any->_type, editor->tc, ev)) {
		bonobo_exception_set 
			(ev, ex_Bonobo_Property_InvalidValue);
		g_warning ("property type change (changed cb) %d %d", 
			   any->_type->kind, editor->tc->kind);
		return;
	}

	int_set_value (editor, any, ev);
}

void                
bonobo_peditor_set_property (BonoboPEditor      *editor,
			     Bonobo_PropertyBag  bag,
			     const char         *name,
			     CORBA_TypeCode      tc, 
			     CORBA_any          *defval)
{
	CORBA_Environment  ev;
	Bonobo_Property    property;
	BonoboArg         *arg;

	g_return_if_fail (editor != NULL);
	g_return_if_fail (bag != CORBA_OBJECT_NIL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (tc != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);

	Bonobo_Unknown_ref (bag, &ev);
	editor->priv->bag = bag;

	property = Bonobo_PropertyBag_getPropertyByName (bag, name, &ev);
	
	if (BONOBO_EX (&ev) || property == CORBA_OBJECT_NIL) {
		CORBA_exception_free (&ev);
		return;
	}

	arg = Bonobo_Property_getValue (property, &ev);

	if (BONOBO_EX (&ev) || arg == NULL) {
		bonobo_object_release_unref (property, NULL);
		CORBA_exception_free (&ev);
		return;
	}

	if (bonobo_arg_type_is_equal (arg->_type, TC_null, NULL) ||
	    !bonobo_arg_type_is_equal (arg->_type, tc, NULL)) {
		CORBA_free (arg);
		
		if (defval) {
			Bonobo_Property_setValue (property, defval, &ev);
			arg = bonobo_arg_copy (defval);
		} else {
			arg = bonobo_arg_new (tc);
			Bonobo_Property_setValue (property, arg, &ev);
		}
	}
	
	/* fixme: use bonobo_object_release_unref instead */
	if (editor->priv->property) 
		CORBA_Object_release (property, NULL);
	   
	editor->priv->property = property;

	if (editor->priv->id && editor->priv->property != CORBA_OBJECT_NIL)
		bonobo_event_source_client_remove_listener 
			(editor->priv->property, editor->priv->id, NULL);

	editor->priv->id = bonobo_event_source_client_add_listener 
		(property, value_changed_cb, NULL, &ev, editor);

	if (bonobo_arg_type_is_equal (arg->_type, TC_null, NULL)) {
		bonobo_arg_release (arg);
		CORBA_exception_free (&ev);
		return;
	}

	if (editor->tc == CORBA_OBJECT_NIL)
		editor->tc = (CORBA_TypeCode)CORBA_Object_duplicate 
			((CORBA_Object) arg->_type, NULL);

	int_set_value (editor, arg, &ev);
		
	CORBA_exception_free (&ev);
	bonobo_arg_release (arg);
}


static void
bonobo_peditor_destroy (GtkObject *object)
{
	BonoboPEditor *editor = BONOBO_PEDITOR (object);
	CORBA_Environment ev;
	
	CORBA_exception_init (&ev);

	if (editor->priv->id && editor->priv->property != CORBA_OBJECT_NIL)
		bonobo_event_source_client_remove_listener (
	                editor->priv->property, editor->priv->id, NULL);

	if (editor->priv->bag != CORBA_OBJECT_NIL)
		Bonobo_Unknown_unref (editor->priv->bag, &ev);
		
	/* fixme: use bonobo_object_release_unref instead */
	if (editor->priv->property)
		CORBA_Object_release ((CORBA_Object) editor->priv->property, 
				      &ev);

	if (editor->tc)
		CORBA_Object_release ((CORBA_Object) editor->tc, &ev);


	CORBA_exception_free (&ev);	
}

static void 
real_peditor_set_value (BonoboPEditor     *editor,
			BonoboArg         *value,
			CORBA_Environment *ev)
{
	g_warning ("calling default set_value implementation");
}

static void
bonobo_peditor_class_init (BonoboPEditorClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *)klass;

	klass->set_value = real_peditor_set_value;
  
	object_class->destroy = bonobo_peditor_destroy;
}

static void
bonobo_peditor_init (BonoboPEditor *editor)
{
	GTK_WIDGET_SET_FLAGS (editor, GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS (editor, GTK_NO_WINDOW);

	editor->priv = g_new0 (BonoboPEditorPrivate, 1);
}

GtkType
bonobo_peditor_get_type (void)
{
	static GtkType petype = 0;

	if (!petype) {
		static const GtkTypeInfo peinfo = {
			"BonoboPEditor",
			sizeof (BonoboPEditor),
			sizeof (BonoboPEditorClass),
			(GtkClassInitFunc) bonobo_peditor_class_init,
			(GtkObjectInitFunc) bonobo_peditor_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};

		petype = gtk_type_unique (GTK_TYPE_OBJECT, &peinfo);
	}

	return petype;
}

BonoboPEditor *
bonobo_peditor_construct (GtkWidget          *widget, 
			  BonoboPEditorSetFn  set_cb,
			  CORBA_TypeCode      tc)
{
	BonoboPEditor *editor;
	
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
	g_return_val_if_fail (set_cb != NULL, NULL);

	editor = gtk_type_new (bonobo_peditor_get_type ());

	if (tc != CORBA_OBJECT_NIL)
		editor->tc = (CORBA_TypeCode)CORBA_Object_duplicate 
			((CORBA_Object) tc, NULL);

	editor->priv->widget = widget;
	editor->priv->set_cb = set_cb;

	gtk_signal_connect_object (GTK_OBJECT (widget), "destroy",
				   bonobo_peditor_destroy,
				   GTK_OBJECT (editor));

	return editor;
}

void                
bonobo_peditor_set_value (BonoboPEditor     *editor,
			  const BonoboArg   *value,
			  CORBA_Environment *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	CORBA_any nv;

	bonobo_return_if_fail (editor != NULL, opt_ev);
	bonobo_return_if_fail (BONOBO_IS_PEDITOR(editor), opt_ev);
	bonobo_return_if_fail (value != NULL, opt_ev);

	if (!editor->priv->property)
		return;

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	if (editor->tc->kind == CORBA_tk_alias && 
	    bonobo_arg_type_is_equal (value->_type, editor->tc->subtypes[0], 
				      my_ev)) {
		
		nv._type =  editor->tc;
		nv._value = value->_value;

		Bonobo_Property_setValue (editor->priv->property, &nv, my_ev);
		
	} else {
		if (!bonobo_arg_type_is_equal (value->_type, editor->tc, 
					       my_ev)) {
			bonobo_exception_set 
				(opt_ev, ex_Bonobo_Property_InvalidValue);
			g_warning ("property type change %d %d", 
				   value->_type->kind, editor->tc->kind);
			return;
		}
		Bonobo_Property_setValue (editor->priv->property, value, 
					  my_ev);
	}

	if (!opt_ev)
		CORBA_exception_free (&ev);
}

GtkWidget *
bonobo_peditor_get_widget (BonoboPEditor *editor) 
{
	g_return_val_if_fail (editor != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PEDITOR (editor), NULL);

	return editor->priv->widget;
}

typedef struct {
	CORBA_TypeCode tc;
	GtkObject *(*new) (CORBA_TypeCode tc);
} PEPlugin;

GtkObject *
bonobo_peditor_resolve (CORBA_TypeCode tc)
{
	static GHashTable *pehash = NULL;
	PEPlugin *plugin;

	if (!pehash) {

		/* fixme: we need a real plugin system */

		pehash = g_hash_table_new (NULL, NULL);

		plugin = g_new0 (PEPlugin, 1);
		plugin->tc = TC_Bonobo_Config_FileName;
		plugin->new = bonobo_peditor_filename_new;
		g_hash_table_insert (pehash, (gpointer)plugin->tc->repo_id, 
				     plugin);

		plugin = g_new0 (PEPlugin, 1);
		plugin->tc = TC_Bonobo_Config_Color;
		plugin->new = bonobo_peditor_color_new;
		g_hash_table_insert (pehash, (gpointer)plugin->tc->repo_id, 
				     plugin);
	}

	if ((plugin = g_hash_table_lookup (pehash, tc->repo_id)))
		return plugin->new (tc);

	switch (tc->kind) {

	case CORBA_tk_short:    
		return bonobo_peditor_short_new ();
	case CORBA_tk_long:   
		return bonobo_peditor_long_new ();
	case CORBA_tk_ushort: 
		return bonobo_peditor_ushort_new ();
	case CORBA_tk_ulong: 
		return bonobo_peditor_ulong_new ();
	case CORBA_tk_float: 
		return bonobo_peditor_float_new ();
	case CORBA_tk_double: 
		return bonobo_peditor_double_new ();
	case CORBA_tk_string: 
		return bonobo_peditor_string_new ();
	case CORBA_tk_boolean: 
		return bonobo_peditor_boolean_new (NULL);
	case CORBA_tk_enum:
		return bonobo_peditor_enum_new ();
	case CORBA_tk_char: 
		/* return bonobo_peditor_char_new (); */
	case CORBA_tk_struct:
	        /* return bonobo_peditor_struct_new (tc); */
	case CORBA_tk_sequence:
	        /* return bonobo_peditor_list_new (tc); */
	case CORBA_tk_array:  
	        /* return bonobo_peditor_array_new (tc); */
	default:              
		return bonobo_peditor_default_new ();
	}
}

GtkObject *
bonobo_peditor_new (Bonobo_PropertyBag  pb,
		    const char         *name,
		    CORBA_TypeCode      tc, 
		    CORBA_any          *defval)
{
	GtkObject *o;

	g_return_val_if_fail (pb != CORBA_OBJECT_NIL, NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (tc != CORBA_OBJECT_NIL, NULL);

	if (!(o = bonobo_peditor_resolve (tc))) 
		return NULL;

	bonobo_peditor_set_property (BONOBO_PEDITOR (o),
					     pb, name, tc, defval);

	return o;
}

static void
guard_cb (BonoboListener    *listener,
	  char              *event_name, 
	  CORBA_any         *any,
	  CORBA_Environment *ev,
	  gpointer           user_data)
{
	GtkWidget *widget = GTK_WIDGET (user_data);
	gboolean v;

	if (bonobo_arg_type_is_equal (any->_type, TC_boolean, NULL)) {

		v = BONOBO_ARG_GET_BOOLEAN (any);

		gtk_widget_set_sensitive (widget, v);
	}
}

static void
remove_listener_cb (GtkWidget *widget, Bonobo_PropertyBag bag)
{
	guint32 id;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	id = (guint32) gtk_object_get_data (GTK_OBJECT (widget), 
					    "BONOBO_LISTENER_ID");

	bonobo_event_source_client_remove_listener (bag, id, NULL);
	Bonobo_Unknown_unref (bag, &ev);

	CORBA_exception_free (&ev);
}

void
bonobo_peditor_set_guard (GtkWidget          *widget,
			  Bonobo_PropertyBag  bag,
			  const char         *prop_name)
{
	CORBA_Environment ev;
	char *mask;
	gboolean v;
	guint32 id;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (bag != CORBA_OBJECT_NIL);
	g_return_if_fail (prop_name != NULL);

	CORBA_exception_init (&ev);

	Bonobo_Unknown_ref (bag, &ev);

	mask = g_strconcat ("=Bonobo/Property:change:", prop_name, NULL);

	id = bonobo_event_source_client_add_listener (bag, guard_cb, mask, 
						      NULL, widget);

	gtk_object_set_data (GTK_OBJECT (widget), "BONOBO_LISTENER_ID", 
			     (gpointer) id);

	gtk_signal_connect (GTK_OBJECT (widget), "destroy",
			    GTK_SIGNAL_FUNC (remove_listener_cb), bag);

	v = bonobo_property_bag_client_get_value_gboolean (bag, prop_name,&ev);
	
	if (!BONOBO_EX (&ev))
		gtk_widget_set_sensitive (widget, v);

	CORBA_exception_free (&ev);
}
