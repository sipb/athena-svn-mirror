//========================================================================
//
// BonoboFile.cc
//
// Copyright 1999 Derek B. Noonburg assigned by Michael Meeks.
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <aconf.h>
#include "config.h"

#include "BonoboStream.h"
#include "gpdf-g-switch.h"
#  include <bonobo.h>
#include "gpdf-g-switch.h"

#ifndef NO_DECRYPTION
#include "Decrypt.h"
#endif

typedef Bonobo_Stream BaseFile;

#if 0
#  define DBG_MORE(x) x
#else
#  define DBG_MORE(x)
#endif

static inline size_t
bfread (void *ptr, size_t size, size_t nmemb, BaseFile file)
{
  CORBA_long len;
  CORBA_Environment ev;
  Bonobo_Stream_iobuf *buffer = NULL;

  g_return_val_if_fail (ptr != NULL, 0);

  DBG_MORE (g_message ("read %p %d %d to %p\n", file, size, nmemb, ptr));

  CORBA_exception_init (&ev);
  Bonobo_Stream_read (file, size*nmemb, &buffer, &ev);
  if (ev._major != CORBA_NO_EXCEPTION) {
    g_warning ("Failed bfread");
    CORBA_exception_free (&ev);
    return 0;
  }
  CORBA_exception_free (&ev);
  memcpy (ptr, buffer->_buffer, buffer->_length);
  len = buffer->_length;

  DBG_MORE (g_message ("Read %d bytes %p %d\n",
                       len, buffer->_buffer, buffer->_length));

  CORBA_free (buffer);

  return len;
}

static inline int
bfseek (BaseFile file, long offset, int whence)
{
  CORBA_Environment ev;
  Bonobo_Stream_SeekType t;
  CORBA_long ans;

  DBG_MORE (g_message ("Seek %p %ld %d\n", file, offset, whence));

  if (whence == SEEK_SET)
    t = Bonobo_Stream_SeekSet;
  else if (whence == SEEK_CUR)
    t = Bonobo_Stream_SeekCur;
  else if (whence == SEEK_END)
    t = Bonobo_Stream_SeekEnd;
  else {
    g_warning ("Serious error in seek type");
    t = Bonobo_Stream_SeekSet;
  }
  
  CORBA_exception_init (&ev);
  ans = Bonobo_Stream_seek (file, offset, t, &ev);
  CORBA_exception_free (&ev);
  return ans;
}

static inline void
brewind (BaseFile file)
{
  bfseek (file, 0, SEEK_SET);
}

static inline long
bftell (BaseFile file)
{
  CORBA_Environment ev;
  CORBA_long pos;

  DBG_MORE (g_message ("tell %p\n", file));

  CORBA_exception_init (&ev);
  pos = Bonobo_Stream_seek (file, 0, Bonobo_Stream_SeekCur, &ev);
  CORBA_exception_free (&ev);

  DBG_MORE (g_message ("tell returns %d\n", pos));

  return pos;
}

bonoboStream::bonoboStream(Bonobo_Stream fA, Guint startA, GBool limitedA,
			   Guint lengthA, Object *dictA):
  BaseStream(dictA) {
  f = fA;
  start = startA;
  limited = limitedA;
  length = lengthA;
  bufPtr = bufEnd = buf;
  bufPos = start;
  savePos = 0;
  saved = gFalse;
}

bonoboStream::~bonoboStream() {
  close();
}

Stream *bonoboStream::makeSubStream(Guint startA, GBool limitedA,
				    Guint lengthA, Object *dictA) {
  return new bonoboStream(f, startA, limitedA, lengthA, dictA);
}

void bonoboStream::reset() {
  savePos = (Guint)bftell(f);
  saved = gTrue;
  bfseek(f, start, SEEK_SET);
  bufPtr = bufEnd = buf;
  bufPos = start;
#ifndef NO_DECRYPTION
  if (decrypt)
    decrypt->reset();
#endif
}

void bonoboStream::close() {
  if (saved) {
    bfseek(f, savePos, SEEK_SET);
    saved = gFalse;
  }
}

GBool bonoboStream::fillBuf() {
  int n;
#ifndef NO_DECRYPTION
  char *p;
#endif

  bufPos += bufEnd - buf;
  bufPtr = bufEnd = buf;
  if (limited && bufPos >= start + length) {
    return gFalse;
  }
  if (limited && bufPos + bonoboStreamBufSize > start + length) {
    n = start + length - bufPos;
  } else {
    n = bonoboStreamBufSize;
  }
  n = bfread(buf, 1, n, f);
  bufEnd = buf + n;
  if (bufPtr >= bufEnd) {
    return gFalse;
  }
#ifndef NO_DECRYPTION
  if (decrypt) {
    for (p = buf; p < bufEnd; ++p) {
      *p = (char)decrypt->decryptByte((Guchar)*p);
    }
  }
#endif
  return gTrue;
}

void bonoboStream::setPos(Guint pos, int dir) {
  Guint size;

  if (dir >= 0) {
    bfseek(f, pos, SEEK_SET);
    bufPos = pos;
  } else {
    bfseek(f, 0, SEEK_END);
    size = (Guint)bftell(f);
    if (pos > size)
      pos = (Guint)size;
    bfseek(f, -(int)pos, SEEK_END);
    bufPos = (Guint)bftell(f);
  }
  bufPtr = bufEnd = buf;
}

void bonoboStream::moveStart(int delta) {
  start += delta;
  bufPtr = bufEnd = buf;
  bufPos = start;
}
