
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         codec_info.h  -  description
 *                         ----------------------------
 *   begin                : Mon Sep 27 2003
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all simple wrappers to get
 *                          information, bitrate and quality of a codec.
 *                          It doesn't cope with codecs directly.
 *   Additional code      : /
 */


#ifndef _CODECS_H_
#define _CODECS_H_

#include "../config.h"

#include "common.h"

#define GM_AUDIO_CODECS_NUMBER 10


static char* CodecInfo [GM_AUDIO_CODECS_NUMBER] [5] = {

  {"iLBC-13k3", N_("iLBC-13k3 (internet Low Bitrate Codec) is a free speech codec suitable for robust voice communication over IP. The codec is designed for narrow band speech and results in a payload bit rate of 13.33 kbit/s with an encoding frame length of 30 ms. The iLBC codec enables graceful speech quality degradation in the case of lost frames, which occurs in connection with lost or delayed IP packets."), N_("Excellent"), N_("13.33 Kbps")},

  {"iLBC-15k2", N_("iLBC-15k2 (internet Low Bitrate Codec) is a free speech codec suitable for robust voice communication over IP. The codec is designed for narrow band speech and results in a payload bitrate of 15.20 kbps with an encoding length of 20 ms. The iLBC codec enables graceful speech quality degradation in the case of lost frames, which occurs in connection with lost or delayed IP packets."), N_("Excellent"), N_("15.2 Kbps")},

  {"SpeexNarrow-15k", N_("Speex is an Open Source/Free Software patent-free audio compression format designed for speech. The Speex Project aims to lower the barrier of entry for voice applications by providing a free alternative to expensive proprietary speech codecs. Moreover, Speex is well-adapted to Internet applications and provides useful features that are not present in most other codecs. Finally, Speex is part of the GNU Project and is available under the Xiph.org variant of the BSD license. SpeexNarrow-15k is based on CELP and is designed to compress voice at a payload bitrate of 15 kbps."), N_("Excellent"), N_("15 Kbps")},

  {"SpeexNarrow-8k", N_("Speex is an Open Source/Free Software patent-free audio compression format designed for speech. The Speex Project aims to lower the barrier of entry for voice applications by providing a free alternative to expensive proprietary speech codecs. Moreover, Speex is well-adapted to Internet applications and provides useful features that are not present in most other codecs. Finally, Speex is part of the GNU Project and is available under the Xiph.org variant of the BSD license. SpeexNarrow-8k is based on CELP and is designed to compress voice at a payload bitrate of 8 kbps."), N_("Good"), N_("8 Kbps")},

  {"MS-GSM", N_("MS-GSM is the Microsoft version of GSM 06.10. GSM 06.10 is a standardized lossy speech compression employed by most European wireless telephones. It uses RPE/LTP (residual pulse excitation/long term prediction) coding to compress frames of 160 13-bit samples with a frame rate of 50 Hz into 260 bits.  Microsoft's GSM 06.10 codec is not compatible with the standard frame format, they use 65-byte-frames (2 x 32 1/2) rather than rounding to 33, and they number the bits in their bytes from the other end."), N_("Good"), N_("13 Kbps")},
  
  {"G.711-ALaw-64k", N_("G.711 is the international standard for encoding telephone audio on 64 kbps channel. It is a pulse code modulation (PCM) scheme operating at 8 kHz sample rate, with 8 bits per sample, fully meeting ITU-T recommendations. This standard has two forms, A-Law and u-Law. A-Law G.711 PCM encoder converts 13 bit linear PCM samples into 8 bit compressed PCM (logarithmic form) samples, and the decoder does the conversion vice versa."), N_("Excellent"), N_("64 Kbps")},

  {"G.711-uLaw-64k", N_("G.711 is the international standard for encoding telephone audio on 64 kbps channel. It is a pulse code modulation (PCM) scheme operating at 8 kHz sample rate, with 8 bits per sample, fully meeting ITU-T recommendations. This standard has two forms, A-Law and u-Law. u-Law G.711 PCM encoder converts 14 bit linear PCM samples into 8 bit compressed PCM (logarithmic form) samples, and the decoder does the conversion vice versa."), N_("Excellent"), N_("64 Kbps")},

  {"GSM-06.10", N_("GSM 06.10 is a standardized lossy speech compression employed by first generation of digital mobile phones in Europe. It uses RPE/LTP (residual pulse excitation/long term prediction) coding to compress frames of 20 ms, achieving a bitrate of 13kbps."), N_("Good"), N_("16.5 Kbps")},

  {"G.726-32k", N_("G.726 conforms to ITU-T G.726 recommendation that specifies speech compression and decompression at rates of 16, 24, 32 and 40 Kbps based on Adaptive Differential Pulse Code Modulation (ADPCM)."), N_("Excellent"), N_("32 Kbps")},
  
  {"G.723.1", N_("G.723.1 conforms to ITU-T G.723.1 recommendation. It was designed for videoconferencing / telephony over standard phone lines, and is optimized for realtime encode and decode. That codec is only available in GnomeMeeting when using Quicknet cards due to patents restrictions."), N_("Excellent"), N_("6.3 or 5.7 Kbps")}
};



class GMH323CodecInfo
{
  
 public:


  /* DESCRIPTION  :  Constructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  GMH323CodecInfo ();


  /* DESCRIPTION  :  Constructor.
   * BEHAVIOR     :  /
   * PRE          :  A codec standard name.
   */
  GMH323CodecInfo (PString);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns information about the codec given as argument.
   * PRE          :  /
   */
  PString GetCodecInfo ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the quality of the codec given as argument.
   * PRE          :  /
   */
  PString GetCodecQuality ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the Bit Rate of the codec given as argument.
   * PRE          :  /
   */
  PString GetCodecBitRate ();


  /* DESCRIPTION  :  = Operator
   * BEHAVIOR     :  Assign a new object with values of the old one.
   * PRE          :  /
   */
  GMH323CodecInfo operator = (const GMH323CodecInfo &);

 private:

  PString GetCodecInfo (int);
  PString codec_name;
};

#endif
