/* Internals of Encrypted-stream implementation.
 * Written by Ken Raeburn (Raeburn@Cygnus.COM).
 * Copyright 1991, 1992, 1993, 1994 by Cygnus Support.
 *
 * Permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation.
 * Cygnus Support makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * kstream-int.h
 */

#include <kstream.h>

/*
 * This is needed now that we are considering including these routines into the
 * interface of an MS-Windows DLL.  The calling sequences of the functions
 * get changed based on their declarations, and we must make sure that
 * the declaration and definition match, by having the declaration reach
 * the definition. 
 *
 * Also may be needed to include the target configuration files.
 */
#include "krb.h"


typedef struct fifo {
  char data[10*1024];
  size_t next_write, next_read;
} fifo;

typedef struct kstream_rec {
  const struct kstream_crypt_ctl_block *ctl;
  int fd;
  int buffering : 2;
  kstream_ptr data;
  /* These should be made pointers as soon as code has been
     written to reallocate them.  Also, it would be more efficient
     to use pointers into the buffers, rather than continually shifting
     them down so unprocessed data starts at index 0.  */
  /* incoming */
  fifo in_crypt, in_clear;
  /* outgoing */
  fifo out_clear;
} kstream_rec;
