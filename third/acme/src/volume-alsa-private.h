/* Generated by GOB (v2.0.3)   (do not edit directly) */

#ifndef __VOLUME_ALSA_PRIVATE_H__
#define __VOLUME_ALSA_PRIVATE_H__

#include "volume-alsa.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _VolumeAlsaPrivate {
#line 148 "volume-alsa.gob"
	gboolean mixerpb;
#line 149 "volume-alsa.gob"
	gint saved_volume;
#line 151 "volume-alsa.gob"
	snd_mixer_t * handle;
#line 152 "volume-alsa.gob"
	snd_mixer_elem_t * elem;
#line 153 "volume-alsa.gob"
	long pmin;
#line 154 "volume-alsa.gob"
	long pmax;
#line 26 "volume-alsa-private.h"
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
