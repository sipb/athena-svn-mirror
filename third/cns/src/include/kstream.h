/* Header file for encrypted-stream library.
 * Written by Ken Raeburn (Raeburn@Cygnus.COM).
 * Copyright (C) 1991, 1992, 1994 by Cygnus Support.
 *
 * Permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation.
 * Cygnus Support makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */

#ifndef KSTREAM_H
#define KSTREAM_H

#include "conf.h"
#include <sys/types.h>		/* for size_t */

/* Each stream is set up for two-way communication; if buffering is
   requested, output will be flushed before input is read, or when
   kstream_flush is called.  */

#if defined (__STDC__) || defined (__cplusplus) || defined (_WINDOWS)
typedef void FAR *kstream_ptr;
#else
typedef char FAR *kstream_ptr;
#endif
typedef struct kstream_rec FAR *kstream;
struct kstream_data_block {
  kstream_ptr ptr;
  size_t length;
};
struct kstream_crypt_ctl_block {
  /* Don't rely on anything in this structure.
     It is almost guaranteed to change.
     Right now, it's just a hack so we can bang out the interface
     in some form that lets us run both rcp and rlogin.  This is also
     the only reason the contents of this structure are public.  */
#if defined (__STDC__) || defined (__cplusplus) || defined (_WINDOWS)

  int (INTERFACE *encrypt) (struct kstream_data_block FAR *, /* output -- written */
		  struct kstream_data_block FAR *, /* input */
		  kstream str
		  );		/* ret val = # input bytes used */

  int (INTERFACE *decrypt) (struct kstream_data_block FAR *, /* output -- written */
		  struct kstream_data_block FAR *, /* input */
		  kstream str
		  );		/* ret val = # input bytes used */
  int (INTERFACE *init) (kstream str, kstream_ptr data);
  void (INTERFACE *destroy) (kstream str);
#else
  int (*encrypt) (), (*decrypt) (), (*init) ();
  void (*destroy) ();
#endif
};

/* ctl==0 means no encryption.  data is specific to crypt functions */
#if defined (__STDC__) || defined (__cplusplus) || defined (_WINDOWS)
kstream INTERFACE kstream_create_from_fd (int fd,
				const struct kstream_crypt_ctl_block FAR *ctl,
				kstream_ptr data);
/* There should be a "standard" DES mode used here somewhere.
   These differ, and I haven't chosen one over the other (yet).  */
kstream INTERFACE kstream_create_rlogin_from_fd (int fd, void FAR * sched,
				       unsigned char (FAR *ivec)[8]);
kstream INTERFACE kstream_create_rcp_from_fd (int fd, void FAR * sched,
				    unsigned char (FAR *ivec)[8]);
int INTERFACE kstream_write (kstream, void FAR *, size_t);
int INTERFACE kstream_read (kstream, void FAR *, size_t);	   
int INTERFACE kstream_flush (kstream);
int INTERFACE kstream_destroy (kstream);
void INTERFACE kstream_set_buffer_mode (kstream, int);
#else
kstream kstream_create_from_fd (),
	kstream_create_rlogin_from_fd (),
	kstream_create_rcp_from_fd ();
void kstream_set_buffer_mode ();
#endif

#if 0 /* Perhaps someday... */
kstream INTERFACE kstream_create (principal, host, port, ...);
#endif

#endif
