/*
 * GNOME Speech - Speech services for the GNOME desktop
 *
 * Copyright 2002 Sun Microsystems Inc. 
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
 * Speaker.c: Implementation of the Speaker  object.  This
 *                  object provides convenience functions and implementations
 *                  of some methods which are defined in the GNOME Speech
 *		    interface.  the purpose of this object is to simplify
 *		    GNOME Speech TTS driver development.
 *
 */

#include <glib.h>
#include <libbonobo.h>
#include <bonobo/Bonobo.h>
#include "speaker.h"




/*
 *
 * Internal private structure for containing information about parameters
 *
 */

typedef struct {
	gchar *name;
	gdouble min;
	gdouble current;
	gdouble max;
	parameter_set_func set_func;
	GSList *value_descriptions;
} ParameterPrivate;



/*
 *
 * Struct to contain value descriptions 
 *
 */

typedef struct {
	gdouble value;
	gchar *description;
} ValueDescription;



static void
value_description_destroy (ValueDescription *d)
{
	g_return_if_fail (d);
	if (d->description)
		g_free (d->description);
	g_free (d);
}



static void
parameter_private_destroy (ParameterPrivate *p)
{
	GSList *tmp;

	g_return_if_fail (p);
	if (p->name)
		g_free (p->name);
	for (tmp = p->value_descriptions; tmp; tmp = tmp->next) {
		value_description_destroy ((ValueDescription *) tmp->data);
	}
	if (p->value_descriptions)
		g_slist_free (p->value_descriptions);
	g_free (p);
}



static ParameterPrivate *
find_parameter (Speaker *s,
		const gchar *name)
{
	GSList *tmp;

	g_return_val_if_fail (s, NULL);
	g_return_val_if_fail (name, NULL);

	for (tmp = s->parameters; tmp; tmp = tmp->next) {
		ParameterPrivate *p = (ParameterPrivate *) tmp->data;
		if (!g_strcasecmp (name, p->name))
			break;
	}
	if (tmp)
		return (ParameterPrivate *) tmp->data;
	else
		return NULL;
}



static GObjectClass *parent_class;

/*
 *
 * Returns Speaker from servant
 *
 */



static Speaker *
speaker_from_servant (PortableServer_Servant servant)
{
	return SPEAKER(bonobo_object_from_servant (servant));
}



/*
 *
 * Implementations of CORBA interface functions
 *
 */

static GNOME_Speech_ParameterList *
impl_getSupportedParameters (PortableServer_Servant servant,
			     CORBA_Environment * ev)
{
	Speaker *s = speaker_from_servant (servant);
	GSList *tmp;
	GNOME_Speech_ParameterList *pl;
	gint i, length;
  
	pl = GNOME_Speech_ParameterList__alloc ();
	pl->_length = pl->_maximum = 0;
	pl->_release = 1;
	g_return_val_if_fail (s->parameters, pl);
	length = g_slist_length (s->parameters);
	pl->_length = pl->_maximum = length;
	pl->_buffer = GNOME_Speech_ParameterList_allocbuf (length);
	i = 0;
	for (tmp = s->parameters; tmp; tmp = tmp->next)
	{
		ParameterPrivate *priv = (ParameterPrivate *) tmp->data;

		pl->_buffer[i].name = CORBA_string_dup (priv->name);
		pl->_buffer[i].min = priv->min;
		pl->_buffer[i].current = priv->current;
		pl->_buffer[i].max = priv->max;
		if (priv->value_descriptions)
			pl->_buffer[i].enumerated = TRUE;
		else
			pl->_buffer[i].enumerated = FALSE;
		i++;
	}
	return pl;
}



static CORBA_string
impl_getParameterValueDescription (PortableServer_Servant servant,
				   const CORBA_char * name,
				   const CORBA_double value,
				   CORBA_Environment *ev)
{
	Speaker  *s = speaker_from_servant (servant);
	ParameterPrivate *priv;
	GSList *tmp;
	ValueDescription *d = NULL;
  
	g_return_val_if_fail (s, NULL);
	priv = find_parameter (s, name);
	g_return_val_if_fail (priv, NULL);
	g_return_val_if_fail (priv->value_descriptions, NULL);
	for (tmp = priv->value_descriptions; tmp; tmp = tmp->next)
	{
		d = (ValueDescription *) tmp->data;
		if (d->value == value)
			break;
	}
	return CORBA_string_dup (d && d->description ? d->description : "");
}



static CORBA_double
impl_getParameterValue (PortableServer_Servant servant,
			const CORBA_char *name,
			CORBA_Environment *ev)
{
	Speaker *s = speaker_from_servant (servant);
	ParameterPrivate *priv;

	priv = find_parameter (s, name);
	if (!priv)
		return -1.0;
	return priv->current;
}



static CORBA_boolean 
impl_setParameterValue (PortableServer_Servant servant,
			const CORBA_char *name,
			const CORBA_double value,
			CORBA_Environment *ev)
{
	Speaker *s = speaker_from_servant (servant);

	return speaker_set_parameter (s, (gchar *) name, (gdouble) value);
}


static void
speaker_init (Speaker *s)
{
	s->clb_list = NULL;
	s->parameters = NULL;
	s->parameter_refresh = FALSE;
}


static void
speaker_finalize(GObject *obj)
{
	Speaker *s = SPEAKER (obj);
	GSList *tmp;

	/* Unref the callbacks list */
	
	clb_list_free (s->clb_list);

	/* Destroy all the parameters */

	for (tmp = s->parameters; tmp; tmp = tmp->next)
		parameter_private_destroy ((ParameterPrivate *) tmp->data);
	if (s->parameters)
		g_slist_free (s->parameters);
	if (parent_class->finalize)
		parent_class->finalize (obj);
}


GSList*
speaker_get_clb_list (Speaker *s)
{
    return clb_list_duplicate (s->clb_list);
}


void
clb_list_free (GSList *list)
{
	GSList *tmp;

	for (tmp = list; tmp; tmp = tmp->next)
	    CORBA_Object_release ((GNOME_Speech_SpeechCallback) tmp->data, NULL);

	g_slist_free (list);
}


GSList*
clb_list_duplicate (GSList *dup_list)
{
	GSList *list = NULL;
	GSList *tmp = list;  

	for (tmp = dup_list; tmp; tmp = tmp->next)
		list = g_slist_append (list, CORBA_Object_duplicate (tmp->data, NULL));
	
	return list;
}


static void
speaker_class_init (SpeakerClass *klass)
{
	POA_GNOME_Speech_Speaker__epv *epv = &klass->epv;
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = speaker_finalize;

	/* Setup epv table */

	epv->getSupportedParameters = impl_getSupportedParameters;
	epv->getParameterValueDescription = impl_getParameterValueDescription;
	epv->getParameterValue = impl_getParameterValue;
	epv->setParameterValue = impl_setParameterValue;
	epv->say = NULL;
	epv->stop = NULL;
	epv->wait = NULL;
	epv->isSpeaking = NULL;
}



void
speaker_add_parameter (Speaker *s,
		       const gchar *name,
		       gdouble min,
		       gdouble current,
		       gdouble max,
		       parameter_set_func set_func)
{
	ParameterPrivate *priv;

	g_return_if_fail (s);

	priv = g_new (ParameterPrivate, 1);
	priv->name = g_strdup (name);
	priv->min = min;
	priv->current = current;
	priv->max = max;
	priv->set_func = set_func;
	priv->value_descriptions = NULL;
	s->parameters = g_slist_prepend (s->parameters, priv);
	s->parameter_refresh = TRUE;
}



void
speaker_add_parameter_value_description (Speaker *s,
					 const gchar *name,
					 gdouble value,
					 gchar *description)
{
	ParameterPrivate *priv;
	ValueDescription *d;

	g_return_if_fail (s);
	g_return_if_fail (name);

	priv = find_parameter (s, name);
	g_return_if_fail (priv);
	d = g_new (ValueDescription, 1);
	d->value = value;
	d->description = g_strdup (description);
	priv->value_descriptions = g_slist_append (priv->value_descriptions, d);
}


gboolean
speaker_set_parameter (Speaker *s,
		       gchar *name,
		       gdouble value)
{
	ParameterPrivate *priv;

	priv = find_parameter (s, name);
	g_return_val_if_fail (priv, FALSE);
	g_return_val_if_fail (priv->set_func, FALSE);
	if (value >= priv->min && value <= priv->max) {
		priv->current = value;
		s->parameter_refresh = TRUE;
		return TRUE;
	}
	return FALSE;
}


gboolean
speaker_needs_parameter_refresh (Speaker *s)
{
	g_return_val_if_fail (s, FALSE);
	return s->parameter_refresh;
}


gboolean
speaker_refresh_parameters (Speaker *s)
{
	GSList *tmp;

	g_return_val_if_fail (s, FALSE);
	for (tmp = s->parameters; tmp; tmp = tmp->next) {
		ParameterPrivate *p = (ParameterPrivate *) tmp->data;
		gboolean result;

		result = p->set_func (s, p->current);
		if (!result)
			return FALSE;
	}
	s->parameter_refresh = FALSE;
	return TRUE;
}


BONOBO_TYPE_FUNC_FULL (Speaker,
		       GNOME_Speech_Speaker,
		       bonobo_object_get_type (),
		       speaker)
