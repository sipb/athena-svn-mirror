/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 *  Bonobo::ZoomableFrame - container side part of Bonobo::Zoomable.
 *
 *  Copyright (C) 2000 Eazel, Inc.
 *                2000 SuSE GmbH.
 *                2001 Ximian Inc.
 * 
 *  Authors: Maciej Stachowiak <mjs@eazel.com>
 *           Martin Baulig <baulig@suse.de>
 */
#include <config.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-zoomable-frame.h>

#undef ZOOMABLE_DEBUG

static GObjectClass *parent_class = NULL;

struct _BonoboZoomableFramePrivate {
	Bonobo_Zoomable zoomable;
};

enum {
	ZOOM_LEVEL_CHANGED,
	ZOOM_PARAMETERS_CHANGED,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL];

static inline BonoboZoomableFrame *
frame_from_servant (PortableServer_Servant servant)
{
	return BONOBO_ZOOMABLE_FRAME (bonobo_object_from_servant (servant));
}

static void 
impl_Bonobo_ZoomableFrame_onLevelChanged (PortableServer_Servant  servant,
					  const CORBA_float      zoom_level,
					  CORBA_Environment      *ev)
{
	BonoboZoomableFrame *frame = frame_from_servant (servant);

	g_signal_emit (G_OBJECT (frame), signals [ZOOM_LEVEL_CHANGED], 0,
		       zoom_level);
}

static void 
impl_Bonobo_ZoomableFrame_onParametersChanged (PortableServer_Servant  servant,
							  CORBA_Environment      *ev)
{
	BonoboZoomableFrame *frame = frame_from_servant (servant);

	g_signal_emit (G_OBJECT (frame), signals [ZOOM_PARAMETERS_CHANGED], 0);
}

static void
bonobo_zoomable_frame_dispose (GObject *object)
{
	BonoboZoomableFrame *frame = (BonoboZoomableFrame *) object;

	CORBA_Object_release (frame->priv->zoomable, NULL);

	parent_class->dispose (object);
}

static void
bonobo_zoomable_frame_finalize (GObject *object)
{
	BonoboZoomableFrame *frame = (BonoboZoomableFrame *) object;

	g_free (frame->priv);

	parent_class->finalize (object);
}

static void
bonobo_zoomable_frame_class_init (BonoboZoomableFrameClass *klass)
{
	GObjectClass *object_class;
	POA_Bonobo_ZoomableFrame__epv *epv = &klass->epv;
	
	object_class = (GObjectClass *) klass;
	
	parent_class = g_type_class_peek_parent (klass);

	signals [ZOOM_LEVEL_CHANGED] =
		g_signal_new ("zoom_level_changed",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboZoomableFrameClass, zoom_level_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__FLOAT,
			      G_TYPE_NONE, 1, G_TYPE_FLOAT);

	signals [ZOOM_PARAMETERS_CHANGED] =
		g_signal_new ("zoom_parameters_changed",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboZoomableFrameClass, zoom_parameters_changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	object_class->dispose  = bonobo_zoomable_frame_dispose;
	object_class->finalize = bonobo_zoomable_frame_finalize;

	epv->onLevelChanged      = impl_Bonobo_ZoomableFrame_onLevelChanged;
	epv->onParametersChanged = impl_Bonobo_ZoomableFrame_onParametersChanged;
}

static void
bonobo_zoomable_frame_init (BonoboZoomableFrame *zoomable)
{
	zoomable->priv = g_new0 (BonoboZoomableFramePrivate, 1);
}

BONOBO_TYPE_FUNC_FULL (BonoboZoomableFrame, Bonobo_ZoomableFrame,
		       BONOBO_TYPE_OBJECT, bonobo_zoomable_frame);


/**
 * bonobo_zoomable_frame_new:
 * 
 * Create a new bonobo-zoomable implementing BonoboObject
 * interface.
 * 
 * Return value: the new frame.
 **/
BonoboZoomableFrame *
bonobo_zoomable_frame_new (void)
{
	return g_object_new (bonobo_zoomable_frame_get_type (), NULL);
}

/**
 * bonobo_zoomable_frame_bind_to_zoomable:
 * @zoomable_frame: A BonoboZoomableFrame object.
 * @zoomable: The CORBA object for the BonoboZoomable embedded
 * in this BonoboZoomableFrame.
 *
 * Associates @zoomable with this @zoomable_frame.
 */
void
bonobo_zoomable_frame_bind_to_zoomable (BonoboZoomableFrame *zoomable_frame,
					Bonobo_Zoomable      zoomable,
					CORBA_Environment   *opt_ev)
{
	CORBA_Environment *ev, temp_ev;

	g_return_if_fail (zoomable != CORBA_OBJECT_NIL);
	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame));

	if (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL)
		CORBA_Object_release (zoomable_frame->priv->zoomable, NULL);

	zoomable_frame->priv->zoomable = CORBA_Object_duplicate (zoomable, NULL);

	if (!opt_ev) {
		CORBA_exception_init (&temp_ev);
		ev = &temp_ev;
	} else
		ev = opt_ev;

	Bonobo_Zoomable_setFrame (zoomable, BONOBO_OBJREF (zoomable_frame), ev);

	if (BONOBO_EX (ev))
		bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame), zoomable, ev);

	if (!opt_ev)
		CORBA_exception_free (&temp_ev);
}

/**
 * bonobo_zoomable_frame_get_zoomable:
 * @zoomable_frame: A BonoboZoomableFrame which is bound to a remote
 * BonoboZoomable.
 *
 * Returns: The Bonobo_Zoomable CORBA interface for the remote Zoomable
 * which is bound to @frame.  See also
 * bonobo_zoomable_frame_bind_to_zoomable().
 */
Bonobo_Zoomable
bonobo_zoomable_frame_get_zoomable (BonoboZoomableFrame *zoomable_frame)
{
	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame),
			      CORBA_OBJECT_NIL);

	return zoomable_frame->priv->zoomable;
}

void
bonobo_zoomable_frame_zoom_in (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;

	g_return_if_fail (zoomable_frame != NULL);
	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame));
	g_return_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);
	Bonobo_Zoomable_zoomIn  (zoomable_frame->priv->zoomable, &ev);
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);
}

void
bonobo_zoomable_frame_zoom_out (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame));
	g_return_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);
	Bonobo_Zoomable_zoomOut (zoomable_frame->priv->zoomable, &ev);
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);
}

void
bonobo_zoomable_frame_zoom_to_fit (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame));
	g_return_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);
	Bonobo_Zoomable_zoomFit (zoomable_frame->priv->zoomable, &ev);
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);
}

void
bonobo_zoomable_frame_zoom_to_default (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame));
	g_return_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);
	Bonobo_Zoomable_zoomDefault (zoomable_frame->priv->zoomable, &ev);
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);
}

float
bonobo_zoomable_frame_get_zoom_level (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	float retval;

	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), 0.0);
	g_return_val_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL, 0.0);

	CORBA_exception_init (&ev);
	retval = Bonobo_Zoomable__get_level (zoomable_frame->priv->zoomable, &ev);
	if (BONOBO_EX (&ev))
		retval = 0.0;
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

float
bonobo_zoomable_frame_get_min_zoom_level (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	float retval;

	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), 0.0);
	g_return_val_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL, 0.0);

	CORBA_exception_init (&ev);
	retval = Bonobo_Zoomable__get_minLevel (zoomable_frame->priv->zoomable, &ev);
	if (BONOBO_EX (&ev))
		retval = 0.0;
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

float
bonobo_zoomable_frame_get_max_zoom_level (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	float retval;

	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), 0.0);
	g_return_val_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL, 0.0);

	CORBA_exception_init (&ev);
	retval = Bonobo_Zoomable__get_maxLevel (zoomable_frame->priv->zoomable, &ev);
	if (BONOBO_EX (&ev))
		retval = 0.0;
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

gboolean
bonobo_zoomable_frame_has_min_zoom_level (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	gboolean retval;

	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), FALSE);
	g_return_val_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL, FALSE);

	CORBA_exception_init (&ev);
	retval = Bonobo_Zoomable__get_hasMinLevel (zoomable_frame->priv->zoomable, &ev);
	if (BONOBO_EX (&ev))
		retval = FALSE;
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

gboolean
bonobo_zoomable_frame_has_max_zoom_level (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	gboolean retval;

	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), FALSE);
	g_return_val_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL, FALSE);

	CORBA_exception_init (&ev);
	retval = Bonobo_Zoomable__get_hasMaxLevel (zoomable_frame->priv->zoomable, &ev);
	if (BONOBO_EX (&ev))
		retval = FALSE;
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

gboolean
bonobo_zoomable_frame_is_continuous (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	gboolean retval;

	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), FALSE);
	g_return_val_if_fail (zoomable_frame->priv->zoomable != CORBA_OBJECT_NIL, FALSE);

	CORBA_exception_init (&ev);
	retval = Bonobo_Zoomable__get_isContinuous (zoomable_frame->priv->zoomable, &ev);
	if (BONOBO_EX (&ev))
		retval = FALSE;
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

GList *
bonobo_zoomable_frame_get_preferred_zoom_levels (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	Bonobo_ZoomLevelList *zoom_levels;
	GList *list = NULL;
	int i;

	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), NULL);

	CORBA_exception_init (&ev);

	zoom_levels = Bonobo_Zoomable__get_preferredLevels (
		zoomable_frame->priv->zoomable, &ev);

	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
					 zoomable_frame->priv->zoomable, &ev);
		CORBA_exception_free (&ev);
		return NULL;
	}

	CORBA_exception_free (&ev);

	if (zoom_levels == CORBA_OBJECT_NIL)
		return NULL;

	for (i = 0; i < zoom_levels->_length; i++) {
		float *this;

		this = g_new0 (float, 1);
		*this = zoom_levels->_buffer [i];

		list = g_list_prepend (list, this);
	}

	CORBA_free (zoom_levels);

	return g_list_reverse (list);
}

GList *
bonobo_zoomable_frame_get_preferred_zoom_level_names (BonoboZoomableFrame *zoomable_frame)
{
	CORBA_Environment ev;
	Bonobo_ZoomLevelNameList *zoom_level_names;
	GList *list = NULL;
	int i;

	g_return_val_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame), NULL);

	CORBA_exception_init (&ev);

	zoom_level_names = Bonobo_Zoomable__get_preferredLevelNames (
		zoomable_frame->priv->zoomable, &ev);

	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
					 zoomable_frame->priv->zoomable, &ev);
		CORBA_exception_free (&ev);
		return NULL;
	}
	CORBA_exception_free (&ev);

	if (zoom_level_names == CORBA_OBJECT_NIL)
		return NULL;

	for (i = 0; i < zoom_level_names->_length; i++)
		list = g_list_prepend (list, g_strdup (zoom_level_names->_buffer [i]));

	CORBA_free (zoom_level_names);

	return g_list_reverse (list);
}

void
bonobo_zoomable_frame_set_zoom_level (BonoboZoomableFrame *zoomable_frame, float zoom_level)
{
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_ZOOMABLE_FRAME (zoomable_frame));

	CORBA_exception_init (&ev);
	Bonobo_Zoomable_setLevel (zoomable_frame->priv->zoomable, zoom_level, &ev);
	bonobo_object_check_env (BONOBO_OBJECT (zoomable_frame),
				 zoomable_frame->priv->zoomable, &ev);
	CORBA_exception_free (&ev);
}
