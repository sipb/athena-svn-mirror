
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
 *                         codec_info.cpp  -  description
 *                         ------------------------------
 *   begin                : Mon Sep 27 2003
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all simple wrappers to get
 *                          information, bitrate and quality of a codec.
 *                          It doesn't cope with codecs directly.
 *   Additional code      : /
 */


#include "codec_info.h"


GMH323CodecInfo::GMH323CodecInfo ()
{
}


GMH323CodecInfo::GMH323CodecInfo (PString name)
{
  codec_name = name;
}


PString
GMH323CodecInfo::GetCodecInfo ()
{
  return GetCodecInfo (1);
}


PString
GMH323CodecInfo::GetCodecQuality ()
{
  return GetCodecInfo (2);
}


PString
GMH323CodecInfo::GetCodecBitRate ()
{
  return GetCodecInfo (3);
}


GMH323CodecInfo
GMH323CodecInfo::operator = (const GMH323CodecInfo & info)
{
  codec_name = info.codec_name;

  return *this;
}


PString
GMH323CodecInfo::GetCodecInfo (int pos)
{
  PString s;

  int i = 0;

  for (i = 0 ; i < GM_AUDIO_CODECS_NUMBER ; i++)
    if ( PString (CodecInfo [i] [0]) == codec_name)
      return PString (gettext (CodecInfo [i] [pos]) );

  return s;
}



