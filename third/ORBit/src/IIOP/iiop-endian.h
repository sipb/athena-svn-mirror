#ifndef IIOP_ENDIAN_H
#define IIOP_ENDIAN_H 1

#include <glib.h>

#if G_BYTE_ORDER == G_BIG_ENDIAN

# define FLAG_ENDIANNESS FLAG_BIG_ENDIAN
# define conversion_needed(to_endianness) ((to_endianness)!=FLAG_BIG_ENDIAN)

#elif G_BYTE_ORDER == G_LITTLE_ENDIAN

# define FLAG_ENDIANNESS FLAG_LITTLE_ENDIAN
# define conversion_needed(to_endianness) ((to_endianness)!=FLAG_LITTLE_ENDIAN)

#else

#error "Unsupported endianness on this system."

#endif

#define FLAG_BIG_ENDIAN 0
#define FLAG_LITTLE_ENDIAN 1

/* This is also defined in IIOP-types.c */
void iiop_byteswap(guchar *outdata,
		   const guchar *data,
		   gulong datalen);

#if defined(G_CAN_INLINE) && !defined(IIOP_DO_NOT_INLINE_IIOP_BYTESWAP)
G_INLINE_FUNC void iiop_byteswap(guchar *outdata,
				 const guchar *data,
				 gulong datalen)
{
  const guchar *source_ptr = data;
  guchar *dest_ptr = outdata + datalen - 1;
  while(dest_ptr >= outdata)
    *dest_ptr-- = *source_ptr++;
}
#endif

#endif
