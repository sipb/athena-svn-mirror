//========================================================================
//
// BonoboStream.h
//
// Author Michael Meeks.
//
// Copyright 1999 Derek B. Noonburg & Michael Meeks.
//
//========================================================================

#ifndef BONOBO_STREAM_H
#define BONOBO_STREAM_H

#include "gpdf-g-switch.h"
#  include <bonobo.h>
#include "gpdf-g-switch.h"
#include "Object.h"
#include "Stream.h"

#define bonoboStreamBufSize fileStreamBufSize

class bonoboStream: public BaseStream {
 public:
  bonoboStream (Bonobo_Stream fA, Guint startA, GBool limitedA,
		Guint lengthA, Object *dictA);
  virtual ~bonoboStream();
  virtual Stream *makeSubStream(Guint startA, GBool limitedA,
				Guint lengthA, Object *dictA);
  virtual StreamKind getKind() { return strFile; }
  virtual void reset();
  virtual void close();
  virtual int getChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr++ & 0xff); }
  virtual int lookChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr & 0xff); }
  virtual int getPos() { return bufPos + (bufPtr - buf); }
  virtual void setPos(Guint pos, int dir = 0);
  virtual GBool isBinary(GBool last = gTrue) { return last; }
  virtual Guint getStart() { return start; }
  virtual void moveStart(int delta);

private:

  GBool fillBuf();

  Bonobo_Stream f;
  Guint start;
  GBool limited;
  Guint length;
  char buf[bonoboStreamBufSize];
  char *bufPtr;
  char *bufEnd;
  Guint bufPos;
  int savePos;
  GBool saved;
};

#endif /* BONOBO_STREAM_H */
