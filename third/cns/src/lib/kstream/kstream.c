#define DEBUG
/* Encrypted-stream implementation for MIT Kerberos.
 * Written by Ken Raeburn (Raeburn@Cygnus.COM).
 * Copyright (C) 1991, 1992, 1993, 1994 by Cygnus Support.
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
 */

/* Current assumptions:
   * the encryption/decryption routine may change the size of the data
     significantly, in either direction, and need not be consistent
     about it.  (e.g., a pipe to "compress" could be used.)
   * encryption/decryption may not consume all characters passed, but
     will consume some, or will return a negative number indicating how
     many more characters should be read before it will be able to do
     anything with the data.  (A return value of -1 is always safe; so
     is calling the crypt routine again without enough data.)
   * the file descriptor used is not set to non-blocking.
   * no other routines will do i/o on that file descriptor while this
     library is using it.  (out-of-band messages are probably okay.)

   Bugs:
   * if the encryption package cannot handle arbitrarily small data,
     flushing the outgoing stream may not work.

   ToDo: Lots.
   * completion of the code...
   * begin testing
   * buffering (efficiency)
   * reduce data copying for large buffers that get sent immediately
   * error handling
   * consistency in error reporting conventions
 */

#include <assert.h>
#include <stdio.h>
#include <errno.h>

/* Only use alloca if we've got gcc 2 or better */
#ifdef __GNUC__
#if __GNUC__ >= 2
#define alloca __builtin_alloca
#define HAS_ALLOCA
#endif
#endif

#include "kstream_int.h"
#if defined (__STDC__) || defined (_WINDOWS)
int krb_net_write (int, char *, int);
int krb_net_read (int, char *, int);
void abort ();
void *memmove (void *, const void *, size_t);
void *memcpy (void *, const void *, size_t);
void *malloc (size_t);
void *realloc (void *, size_t);
void free (void *);
#else
int krb_net_write ();
int krb_net_read ();
void abort ();
char *memmove ();
void *memcpy ();
char *malloc ();
char *realloc ();
void free ();
#endif
#ifndef _WINDOWS
extern int errno;
#endif

#ifdef sun
#ifndef solaris20
/* SunOS has no memmove, but bcopy overlaps correctly */
#define memmove(dest,src,size)	bcopy(src,dest,size)
#endif
#endif

static void fifo__init (this)
     fifo *this;
{
  this->next_write = this->next_read = 0;
  memset (this->data, 0, sizeof (this->data));
}
static char *fifo__data_start (this)
     fifo *this;
{
  return this->data + this->next_read;
}
static size_t fifo__bytes_available (this)
     fifo *this;
{
  return this->next_write - this->next_read;
}
static size_t fifo__space_available (this)
     fifo *this;
{
  return sizeof (this->data) - fifo__bytes_available (this);
}
static int fifo__append (this, ptr, len)
     fifo *this;
     const char *ptr;
     size_t len;
{
  if (len > fifo__space_available (this))
    len = fifo__space_available (this);
  if (sizeof (this->data) - this->next_write < len)
    {
      memmove (this->data, this->data + this->next_read,
	       this->next_write - this->next_read);
      this->next_write -= this->next_read;
      this->next_read = 0;
    }
  memcpy (this->data + this->next_write, ptr, len);
  this->next_write += len;
  return len;
}
static int fifo__extract (this, ptr, len)
     fifo *this;
     char *ptr;
     size_t len;
{
  size_t n = fifo__bytes_available (this);
  if (len > n)
    len = n;
  if (ptr)
    memcpy (ptr, this->data + this->next_read, len);
  this->next_read += len;
  if (this->next_read == this->next_write)
    this->next_read = this->next_write = 0;
  return len;
}

static void kstream_rec__init (this)
     kstream_rec *this;
{
  fifo__init (&this->in_crypt);
  fifo__init (&this->in_clear);
  fifo__init (&this->out_clear);
}

kstream	INTERFACE
kstream_create_from_fd (fd, ctl, data)
     int fd;
     const struct kstream_crypt_ctl_block *ctl;
     void *data;
{
  kstream k;
  k = (kstream) malloc (sizeof (kstream_rec));
  if (!k)
    return 0;
  kstream_rec__init (k);
  k->ctl = ctl;
  k->data = 0;
  k->fd = fd;
  k->buffering = 1;		/* why not? */
  if (ctl && ctl->init && (ctl->init) (k, data) != 0)
    {
      free (k);
      return 0;
    }
  return k;
}

int	INTERFACE
kstream_destroy (k)
     kstream k;
{
  int x = kstream_flush (k);
  if (k->ctl && k->ctl->destroy)
    (k->ctl->destroy) (k);
  free (k);
  return x;
}

void INTERFACE
kstream_set_buffer_mode (k, mode)
     kstream k;
     int mode;
{
  k->buffering = mode;
}

int	INTERFACE
kstream_write (k, p_data, p_len)
     kstream k;
     kstream_ptr p_data;
     size_t p_len;
{
  size_t len = p_len;
  char *data = p_data;
  int x = 0;
  fifo *out = &k->out_clear;

  assert (k != 0);

  if (!k->ctl)
    return write (k->fd, p_data, p_len);

  while (len)
    {
      x = fifo__append (out, data, len);
      assert (x >= 0);
      data += x;
      len -= x;
      if (len == 0 && k->buffering != 0)
	return p_len;
      x = kstream_flush (k);
      if (x < 0)
	return x;
    }
  return p_len;
}

int	INTERFACE
kstream_flush (k)
     kstream k;
{
  int x, n;
  fifo *out = &k->out_clear;
  struct kstream_data_block kd_out, kd_in;

  assert (k != 0);

  if (k->ctl == 0)
    {
      int n = fifo__bytes_available (out);
      x = krb_net_write (k->fd, fifo__data_start (out), n);
      if (x < 0)
	return x;
      else if (x != n)
	abort ();
      fifo__extract (out, NULL, n);
      return 0;
    }

  n = fifo__bytes_available (out);
  kd_in.length = n;
  kd_in.ptr = fifo__data_start (out);
  kd_out.ptr = 0;
  kd_out.length = 0;
  while (fifo__bytes_available (out))
    {
      x = (k->ctl->encrypt) (&kd_out, &kd_in, k);
      if (x < 0)
	return x;
      else if (x == 0)
	return -1;
      /* x is number of input characters processed */
      fifo__extract (out, NULL, x);
      x = krb_net_write (k->fd, kd_out.ptr, kd_out.length);
      if (x < 0)
	return x;
      else if (x != kd_out.length)
	abort ();
    }
  return 0;
}

int	INTERFACE
kstream_read (k, p_data, p_len)
     kstream k;
     kstream_ptr p_data;
     size_t p_len;
{
  char *data = p_data;
  size_t len = p_len;
  int n;
  fifo *in = &k->in_clear, *crypt;
  struct kstream_data_block kd_out, kd_in;

  assert (k != 0);

 read_clear:
  if (k->ctl == 0)
    return read (k->fd, data, len);

  if (fifo__bytes_available (in) > 0)
    return fifo__extract (in, data, len);

  crypt = &k->in_crypt;
 try_2:
  kd_out.ptr = 0;
  kd_out.length = 0;
  kd_in.length = fifo__bytes_available (crypt);
  kd_in.ptr = fifo__data_start (crypt);
  if (kd_in.length == 0)
    {
      n = -1;
      goto read_source;
    }
  n = (k->ctl->decrypt) (&kd_out, &kd_in, k);
  if (n > 0)
    {
      /* Succeeded in decrypting some data.  */
      fifo__extract (crypt, NULL, n);
      {
	int n2;
	n = kd_out.length;
	n2 = fifo__append (in, kd_out.ptr, kd_out.length);
	assert (n == n2);
      }
      goto read_clear;
    }
  else if (n == 0)
    assert (n != 0);			/* not handling errors yet */

 read_source:
  {
    size_t sz;
    static char *buf;
    sz = -n;
    if (sz > fifo__space_available (crypt))
      {
#ifdef DEBUG
#ifndef _WINDOWS
	fprintf (stderr, "insufficient space avail (%lu, want %lu)\n",
		 (unsigned long) fifo__space_available (crypt),
		 (unsigned long) sz);
#endif
#endif
	errno = ENOMEM;
	return -1;
      }
#ifdef HAS_ALLOCA
    buf = alloca (sz);
#else
    if (buf)
      buf = realloc (buf, sz);
    else
      buf = malloc (sz);
    assert(buf);
#endif	
    while (sz > 0)
      {
	int n2;
	n2 = krb_net_read (k->fd, buf, sz);
	if (n2 < 0)
	  {
#ifdef DEBUG
	    perror ("kstream_read: krb_net_read");
#endif
	    return n2;
	  }
	else if (n2 == 0)
	  return 0;
	fifo__append (crypt, buf, n2);
	sz -= n2;
      }
    goto try_2;
  }
}

#if 0
extern "C" {

/* simple rotate */

typedef struct kstream_data_block DB;

static int
move (DB *out, DB *in, kstream k, int direction)
{
  int i, x;
  static char *ptr;
  char *inp = in->ptr;
  if (in->length == 0) return -1;
  /* Sun's realloc loses on null pointers.  */
  ptr = ptr ? realloc (ptr, in->length) : malloc (in->length);
  out->ptr = ptr;
  if (!ptr) return 0;
  out->length = in->length;
  x = direction ? +1 : -1;
  for (i = in->length; i-- > 0; )
    ptr[i] = inp[i] + x;
  return in->length;
}

static int emove (DB *out, DB *in, kstream k) { return move (out, in, k, 0); }
static int dmove (DB *out, DB *in, kstream k) { return move (out, in, k, 1); }
extern const struct kstream_crypt_ctl_block rot1_ccb = {
  emove, dmove, 0, 0
};
}
#endif
