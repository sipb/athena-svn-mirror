/* Generated by GOB (v2.0.3) on Thu Dec 19 22:14:54 2002
   (do not edit directly) */

/* End world hunger, donate to the World Food Programme, http://www.wfp.org */

#define GOB_VERSION_MAJOR 2
#define GOB_VERSION_MINOR 0
#define GOB_VERSION_PATCHLEVEL 3

#define selfp (self->_priv)


#line 1 "volume-oss.gob"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#ifdef __NetBSD__
#include <soundcard.h>
#else
#include <sys/soundcard.h>
#endif /* __NetBSD__ */

#include "volume-oss.h"
#include "volume-oss-private.h"

#line 29 "volume-oss.c"
/* self casting macros */
#define SELF(x) VOLUME_OSS(x)
#define SELF_CONST(x) VOLUME_OSS_CONST(x)
#define IS_SELF(x) VOLUME_IS_OSS(x)
#define TYPE_SELF VOLUME_TYPE_OSS
#define SELF_CLASS(x) VOLUME_OSS_CLASS(x)

#define SELF_GET_CLASS(x) VOLUME_OSS_GET_CLASS(x)

/* self typedefs */
typedef VolumeOSS Self;
typedef VolumeOSSClass SelfClass;

/* here are local prototypes */
static void ___object_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void ___object_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void volume_oss_init (VolumeOSS * o) G_GNUC_UNUSED;
static void volume_oss_class_init (VolumeOSSClass * c) G_GNUC_UNUSED;
static gint volume_oss_vol_check (gint volume) G_GNUC_UNUSED;
static gboolean volume_oss_mixer_check (VolumeOSS * self, gint fd) G_GNUC_UNUSED;

/*
 * Signal connection wrapper macro shortcuts
 */
#define self_connect__fd_problem(object,func,data)	volume_oss_connect__fd_problem((object),(func),(data))
#define self_connect_after__fd_problem(object,func,data)	volume_oss_connect_after__fd_problem((object),(func),(data))
#define self_connect_data__fd_problem(object,func,data,destroy_data,flags)	volume_oss_connect_data__fd_problem((object),(func),(data),(destroy_data),(flags))

enum {
	FD_PROBLEM_SIGNAL,
	LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_VOLUME,
	PROP_MUTE,
	PROP_USE_PCM
};

static guint object_signals[LAST_SIGNAL] = {0};

/* pointer to the class of our parent */
static GObjectClass *parent_class = NULL;

/* Short form macros */
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#define self_get_volume(args...) volume_oss_get_volume(args)
#define self_set_volume(args...) volume_oss_set_volume(args)
#define self_get_mute(args...) volume_oss_get_mute(args)
#define self_set_mute(args...) volume_oss_set_mute(args)
#define self_get_use_pcm(args...) volume_oss_get_use_pcm(args)
#define self_set_use_pcm(args...) volume_oss_set_use_pcm(args)
#define self_new() volume_oss_new()
#define self_fd_problem(args...) volume_oss_fd_problem(args)
#define self_vol_check(args...) volume_oss_vol_check(args)
#define self_mixer_check(args...) volume_oss_mixer_check(args)
#endif /* __GNUC__ && !__STRICT_ANSI__ */

/* Short form pointers */
static gint (* const self_get_volume) (VolumeOSS * self) = volume_oss_get_volume;
static void (* const self_set_volume) (VolumeOSS * self, gint val) = volume_oss_set_volume;
static gboolean (* const self_get_mute) (VolumeOSS * self) = volume_oss_get_mute;
static void (* const self_set_mute) (VolumeOSS * self, gboolean val) = volume_oss_set_mute;
static gboolean (* const self_get_use_pcm) (VolumeOSS * self) = volume_oss_get_use_pcm;
static void (* const self_set_use_pcm) (VolumeOSS * self, gboolean val) = volume_oss_set_use_pcm;
static VolumeOSS * (* const self_new) (void) = volume_oss_new;
static void (* const self_fd_problem) (VolumeOSS * self) = volume_oss_fd_problem;
static gint (* const self_vol_check) (gint volume) = volume_oss_vol_check;
static gboolean (* const self_mixer_check) (VolumeOSS * self, gint fd) = volume_oss_mixer_check;

GType
volume_oss_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		static const GTypeInfo info = {
			sizeof (VolumeOSSClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) volume_oss_class_init,
			(GClassFinalizeFunc) NULL,
			NULL /* class_data */,
			sizeof (VolumeOSS),
			0 /* n_preallocs */,
			(GInstanceInitFunc) volume_oss_init,
		};

		type = g_type_register_static (G_TYPE_OBJECT, "VolumeOSS", &info, (GTypeFlags)0);
	}

	return type;
}

/* a macro for creating a new object of our type */
#define GET_NEW ((VolumeOSS *)g_object_new(volume_oss_get_type(), NULL))

/* a function for creating a new object of our type */
#include <stdarg.h>
static VolumeOSS * GET_NEW_VARG (const char *first, ...) G_GNUC_UNUSED;
static VolumeOSS *
GET_NEW_VARG (const char *first, ...)
{
	VolumeOSS *ret;
	va_list ap;
	va_start (ap, first);
	ret = (VolumeOSS *)g_object_new_valist (volume_oss_get_type (), first, ap);
	va_end (ap);
	return ret;
}


static void
___finalize(GObject *obj_self)
{
#define __GOB_FUNCTION__ "Volume:OSS::finalize"
	VolumeOSS *self = VOLUME_OSS (obj_self);
	gpointer priv = self->_priv;
	if(G_OBJECT_CLASS(parent_class)->finalize) \
		(* G_OBJECT_CLASS(parent_class)->finalize)(obj_self);
	g_free (priv);
	return;
	self = NULL;
}
#undef __GOB_FUNCTION__

static void 
volume_oss_init (VolumeOSS * o)
{
#define __GOB_FUNCTION__ "Volume:OSS::init"
	o->_priv = g_new0 (VolumeOSSPrivate, 1);
#line 1 "volume-oss.gob"
	o->volume = 0;
#line 164 "volume-oss.c"
#line 44 "volume-oss.gob"
	o->mute = FALSE;
#line 167 "volume-oss.c"
#line 78 "volume-oss.gob"
	o->use_pcm = FALSE;
#line 170 "volume-oss.c"
#line 105 "volume-oss.gob"
	o->_priv->mixerpb = FALSE;
#line 173 "volume-oss.c"
#line 105 "volume-oss.gob"
	o->_priv->saved_volume = 0;
#line 176 "volume-oss.c"
#line 105 "volume-oss.gob"
	o->_priv->pcm_avail = TRUE;
#line 179 "volume-oss.c"
	return;
	o = NULL;
}
#undef __GOB_FUNCTION__
static void 
volume_oss_class_init (VolumeOSSClass * c)
{
#define __GOB_FUNCTION__ "Volume:OSS::class_init"
	GObjectClass *g_object_class = (GObjectClass*) c;

	parent_class = g_type_class_ref (G_TYPE_OBJECT);

	object_signals[FD_PROBLEM_SIGNAL] =
		g_signal_new ("fd_problem",
			G_TYPE_FROM_CLASS (g_object_class),
			(GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
			G_STRUCT_OFFSET (VolumeOSSClass, fd_problem),
			NULL, NULL,
			g_cclosure_marshal_VOID__VOID,
			G_TYPE_NONE, 0);

	c->fd_problem = NULL;
	g_object_class->finalize = ___finalize;
	g_object_class->get_property = ___object_get_property;
	g_object_class->set_property = ___object_set_property;
    {
	GParamSpec   *param_spec;

	param_spec = g_param_spec_int ("volume", NULL, NULL,
		G_MININT, G_MAXINT,
		0,
		(GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE));
	g_object_class_install_property (g_object_class,
		PROP_VOLUME, param_spec);
	param_spec = g_param_spec_int ("mute", NULL, NULL,
		G_MININT, G_MAXINT,
		0,
		(GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE));
	g_object_class_install_property (g_object_class,
		PROP_MUTE, param_spec);
	param_spec = g_param_spec_int ("use_pcm", NULL, NULL,
		G_MININT, G_MAXINT,
		0,
		(GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE));
	g_object_class_install_property (g_object_class,
		PROP_USE_PCM, param_spec);
    }
	return;
	c = NULL;
	g_object_class = NULL;
}
#undef __GOB_FUNCTION__

static void
___object_set_property (GObject *object,
	guint property_id,
	const GValue *VAL,
	GParamSpec *pspec)
#define __GOB_FUNCTION__ "Volume:OSS::set_property"
{
	VolumeOSS *self;

	self = VOLUME_OSS (object);

	switch (property_id) {
	case PROP_VOLUME:
	{	gint  ARG = (gint ) g_value_get_int (VAL);
		{
#line 44 "volume-oss.gob"

		gint fd, tvol, volume;

		volume = volume_oss_vol_check(ARG);

		fd = open ("/dev/mixer", O_RDONLY);
		if (volume_oss_mixer_check(self, fd) == FALSE)
		{
			return;
		} else {
			tvol = (volume << 8) + volume;
			if (self->use_pcm && self->_priv->pcm_avail)
				ioctl (fd, MIXER_WRITE (SOUND_MIXER_PCM),
						&tvol);
			else
				ioctl (fd, MIXER_WRITE (SOUND_MIXER_VOLUME),
						&tvol);
			close(fd);
			self->volume = volume;
		}
	
#line 270 "volume-oss.c"
		}
		if (&ARG) ;
		break;
	}
	case PROP_MUTE:
	{	gboolean  ARG = (gboolean ) g_value_get_int (VAL);
		{
#line 78 "volume-oss.gob"

		if (self->mute == FALSE)
		{
			self->_priv->saved_volume =
				volume_oss_get_volume(self);
			volume_oss_set_volume (self, 0);
			self->mute = TRUE;
		} else {
			volume_oss_set_volume (self, self->_priv->saved_volume);
			self->mute = FALSE;
		}
	
#line 291 "volume-oss.c"
		}
		if (&ARG) ;
		break;
	}
	case PROP_USE_PCM:
	{	gboolean  ARG = (gboolean ) g_value_get_int (VAL);
		{
#line 93 "volume-oss.gob"
self->use_pcm = ARG;
#line 301 "volume-oss.c"
		}
		if (&ARG) ;
		break;
	}
	default:
/* Apparently in g++ this is needed, glib is b0rk */
#ifndef __PRETTY_FUNCTION__
#  undef G_STRLOC
#  define G_STRLOC	__FILE__ ":" G_STRINGIFY (__LINE__)
#endif
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
	return;
	self = NULL;
	VAL = NULL;
	pspec = NULL;
}
#undef __GOB_FUNCTION__

static void
___object_get_property (GObject *object,
	guint property_id,
	GValue *VAL,
	GParamSpec *pspec)
#define __GOB_FUNCTION__ "Volume:OSS::get_property"
{
	VolumeOSS *self;

	self = VOLUME_OSS (object);

	switch (property_id) {
	case PROP_VOLUME:
	{	gint  ARG;
	memset (&ARG, 0, sizeof (gint ));
		{
#line 20 "volume-oss.gob"

		gint vol, r, l, fd;

		fd  = open ("/dev/mixer", O_RDONLY);
		if (volume_oss_mixer_check(self, fd) == FALSE)
		{
			ARG = 0;
		} else {
			if (self->use_pcm && self->_priv->pcm_avail)
				ioctl (fd, MIXER_READ (SOUND_MIXER_PCM), &vol);
			else
				ioctl (fd, MIXER_READ (SOUND_MIXER_VOLUME),
						&vol);
			close (fd);

			r = (vol & 0xff);
			l = (vol & 0xff00) >> 8;
			vol = (r + l) / 2;
			vol = volume_oss_vol_check (vol);

			ARG = vol;
		}
	
#line 362 "volume-oss.c"
		}
		g_value_set_int (VAL, ARG);
		break;
	}
	case PROP_MUTE:
	{	gboolean  ARG;
	memset (&ARG, 0, sizeof (gboolean ));
		{
#line 69 "volume-oss.gob"

		/* somebody else might have changed the volume */
		if ((self->mute == TRUE) && (self->volume != 0))
		{
			self->mute = FALSE;
		}
		ARG = self->mute;
	
#line 380 "volume-oss.c"
		}
		g_value_set_int (VAL, ARG);
		break;
	}
	case PROP_USE_PCM:
	{	gboolean  ARG;
	memset (&ARG, 0, sizeof (gboolean ));
		{
#line 93 "volume-oss.gob"
ARG = self->use_pcm;
#line 391 "volume-oss.c"
		}
		g_value_set_int (VAL, ARG);
		break;
	}
	default:
/* Apparently in g++ this is needed, glib is b0rk */
#ifndef __PRETTY_FUNCTION__
#  undef G_STRLOC
#  define G_STRLOC	__FILE__ ":" G_STRINGIFY (__LINE__)
#endif
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
	return;
	self = NULL;
	VAL = NULL;
	pspec = NULL;
}
#undef __GOB_FUNCTION__



#line 20 "volume-oss.gob"
gint 
volume_oss_get_volume (VolumeOSS * self)
#line 417 "volume-oss.c"
{
#define __GOB_FUNCTION__ "Volume:OSS::get_volume"
{
#line 19 "volume-oss.gob"
		gint val; g_object_get (G_OBJECT (self), "volume", &val, NULL); return val;
}}
#line 424 "volume-oss.c"
#undef __GOB_FUNCTION__

#line 44 "volume-oss.gob"
void 
volume_oss_set_volume (VolumeOSS * self, gint val)
#line 430 "volume-oss.c"
{
#define __GOB_FUNCTION__ "Volume:OSS::set_volume"
{
#line 19 "volume-oss.gob"
		g_object_set (G_OBJECT (self), "volume", val, NULL);
}}
#line 437 "volume-oss.c"
#undef __GOB_FUNCTION__

#line 69 "volume-oss.gob"
gboolean 
volume_oss_get_mute (VolumeOSS * self)
#line 443 "volume-oss.c"
{
#define __GOB_FUNCTION__ "Volume:OSS::get_mute"
{
#line 68 "volume-oss.gob"
		gboolean val; g_object_get (G_OBJECT (self), "mute", &val, NULL); return val;
}}
#line 450 "volume-oss.c"
#undef __GOB_FUNCTION__

#line 78 "volume-oss.gob"
void 
volume_oss_set_mute (VolumeOSS * self, gboolean val)
#line 456 "volume-oss.c"
{
#define __GOB_FUNCTION__ "Volume:OSS::set_mute"
{
#line 68 "volume-oss.gob"
		g_object_set (G_OBJECT (self), "mute", val, NULL);
}}
#line 463 "volume-oss.c"
#undef __GOB_FUNCTION__

#line 93 "volume-oss.gob"
gboolean 
volume_oss_get_use_pcm (VolumeOSS * self)
#line 469 "volume-oss.c"
{
#define __GOB_FUNCTION__ "Volume:OSS::get_use_pcm"
{
#line 93 "volume-oss.gob"
		gboolean val; g_object_get (G_OBJECT (self), "use_pcm", &val, NULL); return val;
}}
#line 476 "volume-oss.c"
#undef __GOB_FUNCTION__

#line 93 "volume-oss.gob"
void 
volume_oss_set_use_pcm (VolumeOSS * self, gboolean val)
#line 482 "volume-oss.c"
{
#define __GOB_FUNCTION__ "Volume:OSS::set_use_pcm"
{
#line 93 "volume-oss.gob"
		g_object_set (G_OBJECT (self), "use_pcm", val, NULL);
}}
#line 489 "volume-oss.c"
#undef __GOB_FUNCTION__

/**
 * volume_oss_new:
 *
 * Creates a new #VolumeOSS object
 *
 * Returns: a new object
 **/
#line 102 "volume-oss.gob"
VolumeOSS * 
volume_oss_new (void)
#line 502 "volume-oss.c"
{
#define __GOB_FUNCTION__ "Volume:OSS::new"
{
#line 105 "volume-oss.gob"
	
		VolumeOSS *self;
		int fd;

		self = (VolumeOSS *)GET_NEW;

		fd  = open ("/dev/mixer", O_RDONLY);
		if (volume_oss_mixer_check(self, fd) == FALSE)
		{
			self->_priv->pcm_avail = FALSE;
		} else {
			int mask = 0;

			ioctl (fd, SOUND_MIXER_READ_DEVMASK, &mask);
			if (mask & ( 1 << SOUND_MIXER_PCM))
				self->_priv->pcm_avail = TRUE;
			else
				self->_priv->pcm_avail = FALSE;
			if (!(mask & ( 1 << SOUND_MIXER_VOLUME)))
				self->use_pcm = TRUE;

			close (fd);
		}

		return self;
	}}
#line 533 "volume-oss.c"
#undef __GOB_FUNCTION__

#line 140 "volume-oss.gob"
void 
volume_oss_fd_problem (VolumeOSS * self)
#line 539 "volume-oss.c"
{
	GValue ___param_values[1];
	GValue ___return_val = {0};

#line 140 "volume-oss.gob"
	g_return_if_fail (self != NULL);
#line 140 "volume-oss.gob"
	g_return_if_fail (VOLUME_IS_OSS (self));
#line 548 "volume-oss.c"

	___param_values[0].g_type = 0;
	g_value_init (&___param_values[0], G_TYPE_FROM_INSTANCE (self));
	g_value_set_instance (&___param_values[0], (gpointer) self);

	g_signal_emitv (___param_values,
		object_signals[FD_PROBLEM_SIGNAL],
		0 /* detail */,
		&___return_val);
}

#line 144 "volume-oss.gob"
static gint 
volume_oss_vol_check (gint volume)
#line 563 "volume-oss.c"
{
#define __GOB_FUNCTION__ "Volume:OSS::vol_check"
{
#line 147 "volume-oss.gob"
	
		return CLAMP (volume, 0, 100);
	}}
#line 571 "volume-oss.c"
#undef __GOB_FUNCTION__

#line 152 "volume-oss.gob"
static gboolean 
volume_oss_mixer_check (VolumeOSS * self, gint fd)
#line 577 "volume-oss.c"
{
#define __GOB_FUNCTION__ "Volume:OSS::mixer_check"
#line 152 "volume-oss.gob"
	g_return_val_if_fail (self != NULL, (gboolean )0);
#line 152 "volume-oss.gob"
	g_return_val_if_fail (VOLUME_IS_OSS (self), (gboolean )0);
#line 584 "volume-oss.c"
{
#line 155 "volume-oss.gob"
	
		gboolean retval;

		if (fd <0) {
			if (self->_priv->mixerpb == FALSE) {
				self->_priv->mixerpb = TRUE;
				volume_oss_fd_problem(self);
			}
		}
		retval = (!self->_priv->mixerpb);
		return retval;
	}}
#line 599 "volume-oss.c"
#undef __GOB_FUNCTION__


#if (!defined __GNUC__) || (defined __GNUC__ && defined __STRICT_ANSI__)
/*REALLY BAD HACK
  This is to avoid unused warnings if you don't call
  some method.  I need to find a better way to do
  this, not needed in GCC since we use some gcc
  extentions to make saner, faster code */
static void
___volume_oss_really_bad_hack_to_avoid_warnings(void)
{
	((void (*)(void))GET_NEW_VARG)();
	((void (*)(void))self_get_volume)();
	((void (*)(void))self_set_volume)();
	((void (*)(void))self_get_mute)();
	((void (*)(void))self_set_mute)();
	((void (*)(void))self_get_use_pcm)();
	((void (*)(void))self_set_use_pcm)();
	((void (*)(void))self_new)();
	((void (*)(void))self_fd_problem)();
	((void (*)(void))self_vol_check)();
	((void (*)(void))self_mixer_check)();
	___volume_oss_really_bad_hack_to_avoid_warnings();
}
#endif /* !__GNUC__ || (__GNUC__ && __STRICT_ANSI__) */
